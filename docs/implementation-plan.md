# Implementation Plan

All new code lives in `src/virtual/`, under `namespace npb`.  
Root-level umbrella header: `src/VirtualNeoPixelBus.h`.  
Coding style: Allman braces, 4-space indent, `#pragma once`, `constexpr` where possible, no exceptions.

---

## Phase 1 — Minimum Vertical Slice

Goal: `Color` → `NeoPixelTransform` → `PrintEmitter` → `PixelBus`, compiled and tested end-to-end.

### 1.1 Color

**File:** `src/virtual/internal/colors/Color.h`

```cpp
namespace npb
{

class Color
{
public:
    uint16_t R{0}, G{0}, B{0}, WW{0}, CW{0};

    static constexpr size_t ChannelCount = 5;

    constexpr Color() = default;
    constexpr Color(uint16_t r, uint16_t g, uint16_t b,
                    uint16_t ww = 0, uint16_t cw = 0);

    static constexpr Color fromRgb8(uint8_t r, uint8_t g, uint8_t b);
    static constexpr Color fromRgbw8(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    static constexpr Color fromRgbww8(uint8_t r, uint8_t g, uint8_t b,
                                       uint8_t ww, uint8_t cw);

    constexpr uint16_t  operator[](size_t idx) const;
    uint16_t&           operator[](size_t idx);

    constexpr bool operator==(const Color&) const = default;
};

} // namespace npb
```

Notes:
- 8→16 scaling: `static_cast<uint16_t>((v << 8) | v)` (maps 0xFF→0xFFFF, 0x00→0x0000).
- Header-only.
- `operator[]` maps 0=R, 1=G, 2=B, 3=WW, 4=CW (assert/UB on out-of-range).

### 1.2 ITransformColorToBytes

**File:** `src/virtual/internal/features/ITransformColorToBytes.h`

```cpp
namespace npb
{

class ITransformColorToBytes
{
public:
    virtual ~ITransformColorToBytes() = default;

    virtual void apply(std::span<uint8_t> pixels,
                       std::span<const Color> colors) = 0;

    virtual size_t bytesNeeded(size_t pixelCount) const = 0;
};

} // namespace npb
```

### 1.3 NeoPixelTransform

**File:** `src/virtual/internal/features/NeoPixelTransform.h`

```cpp
namespace npb
{

struct NeoPixelTransformConfig
{
    uint8_t channelCount;                           // 3, 4, or 5
    std::array<uint8_t, Color::ChannelCount> channelOrder;  // index mapping
    uint8_t bitsPerChannel;                         // 8 or 16
    // Phase 6 adds: std::optional<std::variant<...>> inBandSettings;
};

class NeoPixelTransform : public ITransformColorToBytes
{
public:
    explicit NeoPixelTransform(const NeoPixelTransformConfig& config);

    void apply(std::span<uint8_t> pixels,
               std::span<const Color> colors) override;

    size_t bytesNeeded(size_t pixelCount) const override;

private:
    NeoPixelTransformConfig _config;
    size_t _bytesPerPixel;  // channelCount * (bitsPerChannel / 8)
};

} // namespace npb
```

`apply()` logic (Phase 1 — no in-band settings):
```
for each color:
    for ch in 0..channelCount:
        srcChannel = channelOrder[ch]
        value = color[srcChannel]
        if bitsPerChannel == 8:
            pixels[offset++] = value >> 8        // 16-bit → 8-bit truncation
        else:
            pixels[offset++] = value & 0xFF      // low byte first (little-endian)
            pixels[offset++] = value >> 8
```

### 1.4 IEmitPixels

**File:** `src/virtual/internal/methods/IEmitPixels.h`

```cpp
namespace npb
{

class IEmitPixels
{
public:
    virtual ~IEmitPixels() = default;

    virtual void initialize() = 0;
    virtual void update(std::span<const uint8_t> data) = 0;
    virtual bool isReadyToUpdate() const = 0;
    virtual bool alwaysUpdate() const = 0;
};

} // namespace npb
```

### 1.5 PrintEmitter

**File:** `src/virtual/internal/methods/PrintEmitter.h`

```cpp
namespace npb
{

class PrintEmitter : public IEmitPixels
{
public:
    explicit PrintEmitter(Print& output);

    void initialize() override;                        // no-op
    void update(std::span<const uint8_t> data) override;  // hex dump to Print
    bool isReadyToUpdate() const override;             // true
    bool alwaysUpdate() const override;                // false

private:
    Print& _output;
};

} // namespace npb
```

`update()` writes each byte as two hex digits separated by spaces, with a newline at the end. This is a debug/test emitter — no hardware required.

### 1.6 IPixelBus

**File:** `src/virtual/internal/IPixelBus.h`

```cpp
namespace npb
{

class IPixelBus
{
public:
    virtual ~IPixelBus() = default;

    virtual void begin() = 0;
    virtual void show() = 0;
    virtual bool canShow() const = 0;

    virtual size_t pixelCount() const = 0;
    virtual std::span<Color> colors() = 0;
    virtual std::span<const Color> colors() const = 0;

    virtual void setPixelColor(size_t offset,
                               std::span<const Color> pixelData) = 0;
    virtual void getPixelColor(size_t offset,
                               std::span<Color> pixelData) const = 0;
};

} // namespace npb
```

### 1.7 PixelBus

**File:** `src/virtual/internal/PixelBus.h`

```cpp
namespace npb
{

class PixelBus : public IPixelBus
{
public:
    PixelBus(size_t pixelCount,
             ITransformColorToBytes& transform,
             IEmitPixels& emitter);

    void begin() override;
    void show() override;
    bool canShow() const override;

    size_t pixelCount() const override;
    std::span<Color> colors() override;
    std::span<const Color> colors() const override;

    void setPixelColor(size_t offset,
                       std::span<const Color> pixelData) override;
    void getPixelColor(size_t offset,
                       std::span<Color> pixelData) const override;

    // Single-pixel convenience (not on IPixelBus)
    void setPixelColor(size_t index, const Color& color);
    Color getPixelColor(size_t index) const;

private:
    std::vector<Color>    _colors;
    std::vector<uint8_t>  _byteBuffer;
    ITransformColorToBytes& _transform;
    IEmitPixels&            _emitter;
    bool                    _dirty{false};
};

} // namespace npb
```

`show()` implementation:
```cpp
void PixelBus::show()
{
    if (!_dirty && !_emitter.alwaysUpdate())
    {
        return;
    }
    _transform.apply(_byteBuffer, _colors);
    _emitter.update(_byteBuffer);
    _dirty = false;
}
```

`begin()` calls `_emitter.initialize()` and sizes `_byteBuffer` via `_transform.bytesNeeded(_colors.size())`.

### 1.8 Root Header

**File:** `src/VirtualNeoPixelBus.h`

```cpp
#pragma once

#include "virtual/internal/colors/Color.h"
#include "virtual/internal/features/ITransformColorToBytes.h"
#include "virtual/internal/features/NeoPixelTransform.h"
#include "virtual/internal/methods/IEmitPixels.h"
#include "virtual/internal/methods/PrintEmitter.h"
#include "virtual/internal/IPixelBus.h"
#include "virtual/internal/PixelBus.h"
```

### 1.9 Smoke Test

**File:** `examples-virtual/smoke-phase1/smoke-phase1.ino` (Arduino IDE)  
**File:** `examples-virtual/smoke-phase1/main.cpp` (PlatformIO — identical content)

```cpp
#include <Arduino.h>
#include <VirtualNeoPixelBus.h>

static constexpr uint16_t PixelCount = 8;

static npb::NeoPixelTransform transform(
    npb::NeoPixelTransformConfig{
        .channelCount = 3,
        .channelOrder = {1, 0, 2, 0, 0},  // GRB
        .bitsPerChannel = 8
    });

static npb::PrintEmitter emitter(Serial);
static npb::PixelBus bus(PixelCount, transform, emitter);

void setup()
{
    Serial.begin(115200);
    bus.begin();
}

void loop()
{
    static uint8_t value = 0;

    bus.setPixelColor(0, npb::Color::fromRgb8(value, 0, 0));
    bus.show();

    value += 8;
    delay(500);
}
```

### 1.10 PlatformIO Config

Add to `platformio.ini`:

```ini
[env:pico2w-virtual]
platform = https://github.com/maxgerhardt/platform-raspberrypi.git
board = rpipico2w
framework = arduino
board_build.core = earlephilhower
build_src_filter =
    +<examples-virtual/smoke-phase1/main.cpp>
build_unflags = ${common.build_unflags}
build_flags = ${common.build_flags}
lib_deps = ${common.lib_deps}
```

### Phase 1 File Manifest

| # | File | Type |
|---|------|------|
| 1 | `src/virtual/internal/colors/Color.h` | header-only |
| 2 | `src/virtual/internal/features/ITransformColorToBytes.h` | header-only interface |
| 3 | `src/virtual/internal/features/NeoPixelTransform.h` | header + inline impl |
| 4 | `src/virtual/internal/methods/IEmitPixels.h` | header-only interface |
| 5 | `src/virtual/internal/methods/PrintEmitter.h` | header + inline impl |
| 6 | `src/virtual/internal/IPixelBus.h` | header-only interface |
| 7 | `src/virtual/internal/PixelBus.h` | header + inline impl |
| 8 | `src/VirtualNeoPixelBus.h` | umbrella header |
| 9 | `examples-virtual/smoke-phase1/smoke-phase1.ino` | smoke test |
| 10 | `platformio.ini` | add `[env:pico2w-virtual]` |

---

## Phase 2 — Shader Pipeline + GammaShader + CurrentLimiterShader

### 2.1 IShader

**File:** `src/virtual/internal/transforms/IShader.h`

```cpp
namespace npb
{

class IShader
{
public:
    virtual ~IShader() = default;

    virtual void apply(std::span<Color> colors) = 0;
};

} // namespace npb
```

### 2.2 Gamma Methods (duck-typed, compile-time)

Gamma methods are lightweight structs with a static `correct()` method — no virtual dispatch. The compiler inlines them into the per-channel loop. All operate at 16-bit precision (the internal `Color` width).

**Required interface (duck-typed):**
```cpp
struct SomeGammaMethod
{
    static constexpr uint16_t correct(uint16_t value);
    // — or non-constexpr for table/dynamic methods
};
```

| # | Class | File | Approach | Memory | Configurable |
|---|-------|------|----------|--------|--------------|
| 1 | `GammaEquationMethod` | `GammaEquationMethod.h` | `pow(x, 1/0.45)` ≈ γ 2.222 | 0 | No |
| 2 | `GammaCieLabMethod` | `GammaCieLabMethod.h` | CIE L* piecewise curve | 0 | No |
| 3 | `GammaTableMethod` | `GammaTableMethod.h` | Static 256-entry LUT (flash) + 16-bit hint table | ~394 B flash | No |
| 4 | `GammaDynamicTableMethod` | `GammaDynamicTableMethod.h` | Runtime-filled 256 LUT + optional hint table | 256 B RAM + heap | Yes — custom curve function passed to `initialize()` |
| 5 | `GammaNullMethod` | `GammaNullMethod.h` | Identity pass-through | 0 | No |
| 6 | `GammaInvertMethod<T>` | `GammaInvertMethod.h` | Decorator template — `~T::correct(v)` | per `T` | Yes — wraps any method |

All files live under `src/virtual/internal/transforms/`.

**Key design points:**
- All methods work at 16-bit natively (no separate 8-bit path; `Color` is always 16-bit internally).
- Static method dispatch via templates — zero vtable overhead in the hot per-pixel loop.
- `GammaInvertMethod<T>` is a template decorator wrapping any gamma method at compile time.
- `GammaDynamicTableMethod` is the only stateful method (non-static `correct()` after `initialize()`).

**GammaEquationMethod** implementation:
```cpp
struct GammaEquationMethod
{
    static uint16_t correct(uint16_t value)
    {
        if (value == 0) return 0;
        if (value == 0xFFFF) return 0xFFFF;
        float unit = static_cast<float>(value) / 65535.0f;
        float corrected = powf(unit, 1.0f / 0.45f);
        return static_cast<uint16_t>(corrected * 65535.0f + 0.5f);
    }
};
```

**GammaCieLabMethod** implementation:
```cpp
struct GammaCieLabMethod
{
    static uint16_t correct(uint16_t value)
    {
        if (value == 0) return 0;
        if (value == 0xFFFF) return 0xFFFF;
        float unit = static_cast<float>(value) / 65535.0f;
        float corrected;
        if (unit <= 0.08f)
            corrected = unit / 9.033f;
        else
            corrected = powf((unit + 0.16f) / 1.16f, 3.0f);
        return static_cast<uint16_t>(corrected * 65535.0f + 0.5f);
    }
};
```

**GammaNullMethod** implementation:
```cpp
struct GammaNullMethod
{
    static constexpr uint16_t correct(uint16_t value) { return value; }
};
```

**GammaInvertMethod** implementation:
```cpp
template<typename T_METHOD>
struct GammaInvertMethod
{
    static constexpr uint16_t correct(uint16_t value)
    {
        return static_cast<uint16_t>(~T_METHOD::correct(value));
    }
};
```

### 2.3 GammaShader

**File:** `src/virtual/internal/transforms/GammaShader.h`

Template class that applies a compile-time gamma method to all channels, then type-erases into `IShader` for the shader pipeline.

```cpp
namespace npb
{

template<typename T_GAMMA>
class GammaShader : public IShader
{
public:
    void apply(std::span<Color> colors) override
    {
        for (auto& color : colors)
        {
            for (size_t ch = 0; ch < Color::ChannelCount; ++ch)
            {
                color[ch] = T_GAMMA::correct(color[ch]);
            }
        }
    }
};

} // namespace npb
```

The virtual call happens once per batch at the `IShader` level. The inner `T_GAMMA::correct()` calls are fully inlined by the compiler — no vtable dispatch in the hot loop.

### 2.4 CurrentLimiterShader

**File:** `src/virtual/internal/transforms/CurrentLimiterShader.h`

Per-pixel mA estimation, proportional scale-back to power budget.

```cpp
namespace npb
{

class CurrentLimiterShader : public IShader
{
public:
    CurrentLimiterShader(uint32_t maxMilliamps,
                         uint16_t milliampsPerChannel);

    void apply(std::span<Color> colors) override;

private:
    uint32_t _maxMilliamps;
    uint16_t _milliampsPerChannel;
};

} // namespace npb
```

### 2.6 ShadedTransform

**File:** `src/virtual/internal/transforms/ShadedTransform.h`

Batch decorator wrapping `ITransformColorToBytes` + shader chain.

```cpp
namespace npb
{

class ShadedTransform : public ITransformColorToBytes
{
public:
    ShadedTransform(ITransformColorToBytes& inner,
                    std::span<IShader* const> shaders);

    void apply(std::span<uint8_t> pixels,
               std::span<const Color> colors) override;

    size_t bytesNeeded(size_t pixelCount) const override;

private:
    ITransformColorToBytes& _inner;
    std::span<IShader* const> _shaders;
    static constexpr size_t ScratchSize = 32;
};

} // namespace npb
```

`apply()` processes colors in batches of `ScratchSize` through a stack-allocated `std::array<Color, 32>` scratch buffer (320 bytes). Each batch: copy source colors → apply shader chain in order → forward to inner transform. Zero-copy passthrough when shader span is empty.

### 2.7 Smoke Test

**File:** `examples-virtual/gamma-and-limiter/gamma-and-limiter.ino` (Arduino IDE)  
**File:** `examples-virtual/gamma-and-limiter/main.cpp` (PlatformIO)

Verify gamma curves with multiple methods, current budget clamping, and that original `Color` values in `PixelBus` remain unmodified after `show()`.

### Phase 2 File Manifest

| # | File | Type |
|---|------|------|
| 1 | `src/virtual/internal/transforms/IShader.h` | header-only interface |
| 2 | `src/virtual/internal/transforms/GammaEquationMethod.h` | header-only |
| 3 | `src/virtual/internal/transforms/GammaCieLabMethod.h` | header-only |
| 4 | `src/virtual/internal/transforms/GammaTableMethod.h` | header + static data |
| 5 | `src/virtual/internal/transforms/GammaDynamicTableMethod.h` | header + impl |
| 6 | `src/virtual/internal/transforms/GammaNullMethod.h` | header-only |
| 7 | `src/virtual/internal/transforms/GammaInvertMethod.h` | header-only template |
| 8 | `src/virtual/internal/transforms/GammaShader.h` | header-only template |
| 9 | `src/virtual/internal/transforms/CurrentLimiterShader.h` | header + inline impl |
| 10 | `src/virtual/internal/transforms/ShadedTransform.h` | header + inline impl |
| 11 | `examples-virtual/gamma-and-limiter/main.cpp` | smoke test |

## Phase 3 — Two-Wire Infrastructure

- `IClockDataBus` interface (`src/virtual/internal/methods/IClockDataBus.h`)
- `ClockDataProtocol` descriptor struct
- `ClockDataEmitter` — protocol-descriptor-driven framing + `IClockDataBus` delegation
- `DebugClockDataBus` — `Print`-based debug implementation
- `DotStarTransform` — APA102/HD108 serialization (0xFF/0xE0 prefix, luminance byte)
- Example: `examples-virtual/dotstar-debug/dotstar-debug.ino` — verify DotStar framing, start/end frames, pixel byte layout

## Phase 4 — Remaining Two-Wire Transforms

- `Lpd8806Transform` (7-bit, MSB set)
- `Lpd6803Transform` (5-5-5 packed)
- `P9813Transform` (checksum prefix)
- `Ws2801Transform` (raw 3-byte passthrough)
- `Tlc59711Transform` (reversed 16-bit + inline headers)
- `Tlc5947Transform` (12-bit packing, reverse order)
- Example: `examples-virtual/two-wire-transforms/two-wire-transforms.ino` — exercise each transform with PrintEmitter

## Phase 5 — In-Band Settings

- `Tm1814CurrentSettings`, `Tm1914ModeSettings`, `Sm168xGainSettings`, `Tlc59711Settings`
- `NeoPixelTransformConfig::inBandSettings` — `std::optional<std::variant<...>>`
- `NeoPixelTransform::apply()` serializes settings header/footer
- Example: `examples-virtual/in-band-settings/in-band-settings.ino` — verify settings bytes at correct stream positions

## Phase 6 — Platform Emitters (RP2040 First)

- `OneWireTiming` descriptor struct
- `Rp2040PioEmitter` — wraps existing PIO/DMA machinery behind `IEmitPixels`
- Example: `examples-virtual/rp2040-neopixel/rp2040-neopixel.ino` — integration test on hardware (pico2w target)

## Phase 7 — Convenience Aliases + Migration

- Type aliases: `NeoPixelBusGrb`, `NeoPixelBusGrbw`, etc. — pre-configured `PixelBus` factories
- Optional compatibility adapter: maps old `NeoPixelBus<F,M>` API surface onto new `PixelBus`
