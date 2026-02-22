# Shader Pipeline Performance Analysis

## Overview

This document analyzes the per-pixel render cost of the virtual architecture's shader and transform pipeline, and presents a compile-time `ShaderPipeline` alternative that eliminates virtual dispatch overhead in the hot loop.

---

## Architecture: Render Hot Path

When `PixelBus::show()` is called, the emitter's `update(span<const Color>)` runs the following per-pixel:

```
PixelBus::show()                         // 1× per frame
  └─ _emitter->update(colors)            // virtual — 1× per frame (acceptable)
       ├─ _shader->begin(colors)         // virtual — 1× per frame
       ├─ for each pixel:
       │    _shader->apply(i, color)     // virtual — 1× per PIXEL
       │    _transform.apply(buf, &c)    // virtual — 1× per PIXEL
       ├─ _shader->end()                // virtual — 1× per frame
       └─ _bus.transmitBytes(buffer)    // DMA handoff
```

The per-frame virtual calls (`update`, `begin`, `end`) are negligible. The per-pixel calls (`apply` on shader and transform) dominate.

---

## Baseline Cost: Virtual Dispatch

Each virtual call on an embedded ARM target incurs:

| Step | Cycles (M33) |
|------|-------------|
| Load `this` pointer | 1 |
| Load vtable pointer from `*this` | 2 (memory load) |
| Load function pointer from vtable | 2 (memory load) |
| Indirect branch + pipeline flush | 5–8 |
| Argument passing (Color by value, 5 bytes) | 3–5 |
| Return value copy | 2–3 |
| **Total per virtual call** | **~15–20** |

With two virtual calls per pixel (shader + transform), the overhead is **~30–40 cycles/pixel** — roughly **40%** of the total per-pixel budget.

---

## Per-Pixel Cycle Estimates

### Workload: 12,288 pixels (12 × 1,024), GRB 3-byte output

#### Cortex-M33 @ 133 MHz (RP2350 / Pico 2W)

| Scenario | Cycles/pixel | Total Cycles | Time | % of 33.3 ms (30 fps) |
|----------|-------------|-------------|------|----------------------|
| No shader | ~50 | 615K | 4.6 ms | 14% |
| Gamma (table LUT) | ~90 | 1.11M | 8.3 ms | 25% |
| Gamma + current limiter | ~130 | 1.60M | 12.0 ms | 36% |

#### Cortex-M0+ @ 133 MHz (hypothetical)

| Scenario | Cycles/pixel | Total Cycles | Time | % of 33.3 ms |
|----------|-------------|-------------|------|--------------|
| No shader | ~65 | 799K | 6.0 ms | 18% |
| Gamma (table LUT) | ~115 | 1.41M | 10.6 ms | 32% |
| Gamma + current limiter | ~165 | 2.03M | 15.2 ms | 46% |

#### Xtensa LX6 @ 240 MHz (ESP32)

| Scenario | Cycles/pixel | Total Cycles | Time | % of 33.3 ms |
|----------|-------------|-------------|------|--------------|
| No shader | ~55 | 451K | 1.9 ms | 6% |
| Gamma (table LUT) | ~95 | 779K | 3.2 ms | 10% |
| Gamma + current limiter | ~135 | 1.11M | 4.6 ms | 14% |

### Per-pixel cycle breakdown (with gamma shader, M33)

| Operation | Cycles | % of Total |
|-----------|--------|-----------|
| vtable: `_shader->apply()` | ~18 | 20% |
| vtable: `_transform.apply()` | ~18 | 20% |
| GammaShader body (5× table lookup) | ~25 | 28% |
| ColorOrderTransform body (3× indexed store) | ~17 | 19% |
| Loop overhead + span construction | ~12 | 13% |
| **Total** | **~90** | |

---

## Compile-Time Solution: `ShaderPipeline`

### Concept

Template the emitter on its shader and transform types. The per-pixel shader and transform calls become static dispatch — fully inlined by the compiler. The only virtual boundary remains at `IEmitPixels::update()`, called once per frame.

### Implementation

```cpp
template<typename... TShaders>
class ShaderPipeline
{
public:
    constexpr explicit ShaderPipeline(TShaders... shaders)
        : _shaders{std::move(shaders)...}
    {
    }

    void begin(std::span<const Color> colors)
    {
        std::apply([&](auto&... s) { (s.begin(colors), ...); }, _shaders);
    }

    const Color apply(uint16_t index, const Color color)
    {
        Color result = color;
        std::apply([&](auto&... s)
        {
            ((result = s.apply(index, result)), ...);
        }, _shaders);
        return result;
    }

    void end()
    {
        std::apply([&](auto&... s) { (s.end(), ...); }, _shaders);
    }

private:
    std::tuple<TShaders...> _shaders;
};
```

The fold expression `((result = s.apply(index, result)), ...)` expands at compile time. For a two-shader pipeline (`GammaShader<GammaTableMethod>`, `CurrentLimiterShader`), the compiler generates:

```cpp
result = gamma.apply(index, result);    // inlined: 5× table lookups
result = limiter.apply(index, result);  // inlined: 5× scale + shift
```

No loops, no vtable, no indirect branches.

### Empty Pipeline

`ShaderPipeline<>` (zero template arguments) compiles to complete no-ops:
- `apply()` returns the input color unchanged
- `begin()` and `end()` are empty

The compiler eliminates the entire shader block — equivalent to the no-shader fast path.

### Templated Emitter

```cpp
template<typename TShader, typename TTransform>
class ClockDataEmitter : public IEmitPixels
{
public:
    ClockDataEmitter(IClockDataBus& bus,
                     TShader shader,
                     TTransform transform,
                     size_t pixelCount);

    void update(std::span<const Color> colors) override
    {
        _shader.begin(colors);           // static dispatch
        size_t offset = 0;
        const size_t bpp = _transform.bytesNeeded(1);
        for (uint16_t i = 0; i < colors.size(); ++i)
        {
            Color shaded = _shader.apply(i, colors[i]);  // static — inlined
            _transform.apply(                              // static — inlined
                std::span<uint8_t>(_byteBuffer).subspan(offset, bpp),
                std::span<const Color>(&shaded, 1));
            offset += bpp;
        }
        _shader.end();
        _bus.transmitBytes(_byteBuffer);
    }

private:
    IClockDataBus& _bus;
    TShader _shader;
    TTransform _transform;
    std::vector<uint8_t> _byteBuffer;
    size_t _pixelCount;
};
```

`PixelBus` remains unchanged — it holds `unique_ptr<IEmitPixels>` and doesn't know or care about the template parameters.

### Usage Examples

```cpp
// Maximum performance: compile-time gamma, no vtable
auto emitter = std::make_unique<ClockDataEmitter<
    ShaderPipeline<GammaShader<GammaTableMethod>>,
    ColorOrderTransform>>(bus, pipeline, transform, pixelCount);

// Two-shader chain: gamma + current limiter, still zero vtable in hot loop
auto pipeline = ShaderPipeline{
    GammaShader<GammaTableMethod>{},
    CurrentLimiterShader{5000, {20, 20, 20, 0, 0}}
};

// Runtime flexibility: wrap IShader* for dynamic shader swapping
auto emitter = std::make_unique<ClockDataEmitter<
    VirtualShaderAdapter,
    ColorOrderTransform>>(bus, adapter, transform, pixelCount);
```

### VirtualShaderAdapter (runtime fallback)

For users who need to swap shaders at runtime, a thin adapter wraps `IShader*` back into the duck-typed template interface:

```cpp
class VirtualShaderAdapter
{
public:
    explicit VirtualShaderAdapter(std::unique_ptr<IShader> shader)
        : _shader{std::move(shader)}
    {
    }

    void begin(std::span<const Color> colors) { _shader->begin(colors); }
    const Color apply(uint16_t i, const Color c) { return _shader->apply(i, c); }
    void end() { _shader->end(); }

private:
    std::unique_ptr<IShader> _shader;
};
```

This gives the same per-pixel cost as the current virtual approach, but lets users opt in only when needed.

---

## Performance Comparison: Virtual vs Templated

### Cortex-M33 @ 133 MHz, 12,288 pixels

| Scenario | Virtual (cycles/px) | Templated (cycles/px) | Speedup |
|----------|--------------------|-----------------------|---------|
| No shader | 50 | 30 | 1.7× |
| Gamma (table) | 90 | 50 | 1.8× |
| Gamma + limiter | 130 | 75 | 1.7× |

| Scenario | Virtual (ms) | Templated (ms) | Savings |
|----------|-------------|----------------|---------|
| No shader | 4.6 | 2.8 | -39% |
| Gamma (table) | 8.3 | 4.6 | -44% |
| Gamma + limiter | 12.0 | 6.9 | -42% |

### Tradeoffs

| Dimension | Virtual (`ShaderChain`) | Templated (`ShaderPipeline`) |
|-----------|------------------------|------------------------------|
| Per-pixel dispatch | vtable (2 indirect calls) | inline (0 indirect calls) |
| Shader count | runtime — `span<IShader*>` | compile-time — variadic template |
| Add/remove shaders | anytime | requires new emitter type |
| Code size | single instantiation | one per `<Shader, Transform>` combo |
| Per-pixel overhead | ~40% of budget | ~0% |
| `PixelBus` impact | none | none — hidden behind `IEmitPixels` |

---

## Recommendation

Use `ShaderPipeline` as the default for production. The shader configuration is typically known at compile time (gamma method and current limiter parameters are set once). Reserve `VirtualShaderAdapter` for development/debug scenarios where runtime shader swapping is genuinely needed.

Factory functions (Phase 7) should default to the templated path:

```cpp
auto bus = npb::makeNeoPixelBus<GammaTableMethod>(pixelCount, pin);
// Internally creates: ClockDataEmitter<ShaderPipeline<GammaShader<GammaTableMethod>>, ...>
```
