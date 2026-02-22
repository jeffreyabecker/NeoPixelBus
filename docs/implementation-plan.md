# Implementation Plan

All new code lives in `src/virtual/`, under `namespace npb`.  
Root-level umbrella header: `src/VirtualNeoPixelBus.h`.  
Coding style: Allman braces, 4-space indent, `#pragma once`, `constexpr` where possible, no exceptions.

---

## Phase 1 — Minimum Vertical Slice

Goal: `Color` → `ColorOrderTransform` → `PrintEmitter` → `PixelBus`, compiled and tested end-to-end.

### 1.1 Color

**File:** `src/virtual/colors/Color.h`

```cpp
namespace npb
{

class Color
{
public:
    uint8_t R{0}, G{0}, B{0}, WW{0}, CW{0};

    static constexpr size_t ChannelCount = 5;

    constexpr Color() = default;
    constexpr Color(uint8_t r, uint8_t g, uint8_t b,
                    uint8_t ww = 0, uint8_t cw = 0);

    constexpr uint8_t  operator[](size_t idx) const;
    uint8_t&           operator[](size_t idx);

    constexpr bool operator==(const Color&) const = default;
};

} // namespace npb
```

Notes:
- 8-bit channels — matches actual LED chip capability.
- Header-only.
- `operator[]` maps 0=R, 1=G, 2=B, 3=WW, 4=CW (assert/UB on out-of-range).

### 1.2 ITransformColorToBytes

**File:** `src/virtual/emitters/ITransformColorToBytes.h`

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

### 1.3 ColorOrderTransform

**File:** `src/virtual/emitters/ColorOrderTransform.h`

```cpp
namespace npb
{

struct ColorOrderTransformConfig
{
    uint8_t channelCount;                           // 3, 4, or 5
    std::array<uint8_t, Color::ChannelCount> channelOrder;  // index mapping
    // Phase 5 adds: std::optional<std::variant<...>> inBandSettings;
};

class ColorOrderTransform : public ITransformColorToBytes
{
public:
    explicit ColorOrderTransform(const ColorOrderTransformConfig& config);

    void apply(std::span<uint8_t> pixels,
               std::span<const Color> colors) override;

    size_t bytesNeeded(size_t pixelCount) const override;

private:
    ColorOrderTransformConfig _config;
    size_t _bytesPerPixel;  // channelCount
};

} // namespace npb
```

`apply()` logic (Phase 1 — no in-band settings):
```
for each color:
    for ch in 0..channelCount:
        srcChannel = channelOrder[ch]
        pixels[offset++] = color[srcChannel]
```

### 1.4 IEmitPixels

**File:** `src/virtual/emitters/IEmitPixels.h`

```cpp
namespace npb
{

class IEmitPixels
{
public:
    virtual ~IEmitPixels() = default;

    virtual void initialize() = 0;
    virtual void update(std::span<const Color> colors) = 0;
    virtual bool isReadyToUpdate() const = 0;
    virtual bool alwaysUpdate() const = 0;
};

} // namespace npb
```

**Invariant:** `update()` is always called with `colors.size() == pixelCount` (the count provided at construction). Emitters may assume this without bounds-checking.

Emitters accept `span<const Color>` — they own the byte buffer and transform internally.

### 1.5 PrintEmitter

**File:** `src/virtual/emitters/PrintEmitter.h`

```cpp
namespace npb
{

struct PrintEmitterSettings
{
    Print& output;
    ColorOrderTransformConfig colorConfig;
};

class PrintEmitter : public IEmitPixels
{
public:
    PrintEmitter(uint16_t pixelCount,
                 std::unique_ptr<IShader> shader,
                 PrintEmitterSettings settings);

    void initialize() override;                          // no-op
    void update(std::span<const Color> colors) override; // shade + serialize + hex dump
    bool isReadyToUpdate() const override;               // true
    bool alwaysUpdate() const override;                  // false

private:
    Print& _output;
    std::unique_ptr<IShader> _shader;
    ColorOrderTransform _transform;
    std::vector<Color> _scratchColors;
    std::vector<uint8_t> _byteBuffer;
};

} // namespace npb
```

`update()` iterates pixels, optionally applies shader per-pixel, serializes through its internal `ColorOrderTransform`, and hex-dumps to `Print`. This is a debug/test emitter — no hardware required.

### 1.6 IPixelBus

**File:** `src/virtual/IPixelBus.h`

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

**File:** `src/virtual/PixelBus.h`

PixelBus is a thin orchestrator. It owns only the `Color` buffer and a `unique_ptr<IEmitPixels>`. No byte buffer, no transform reference — those are internal to the emitter.

```cpp
namespace npb
{

class PixelBus : public IPixelBus
{
public:
    PixelBus(size_t pixelCount,
             std::unique_ptr<IEmitPixels> emitter);

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
    std::vector<Color>           _colors;
    std::unique_ptr<IEmitPixels> _emitter;
    bool                         _dirty{false};
};

} // namespace npb
```

`show()` implementation:
```cpp
void PixelBus::show()
{
    if (!_dirty && !_emitter->alwaysUpdate())
    {
        return;
    }
    _emitter->update(_colors);
    _dirty = false;
}
```

`begin()` calls `_emitter->initialize()`.

### 1.8 Root Header

**File:** `src/VirtualNeoPixelBus.h`

```cpp
#pragma once

// Colors
#include "virtual/colors/Color.h"

// Emitters (includes internal transform details)
#include "virtual/emitters/IEmitPixels.h"
#include "virtual/emitters/PrintEmitter.h"

// Bus
#include "virtual/IPixelBus.h"
#include "virtual/PixelBus.h"
```

### 1.9 Smoke Test

**File:** `examples-virtual/smoke-phase1/smoke-phase1.ino` (Arduino IDE)  
**File:** `examples-virtual/smoke-phase1/main.cpp` (PlatformIO — identical content)

```cpp
#include <Arduino.h>
#include <VirtualNeoPixelBus.h>

static constexpr uint16_t PixelCount = 8;

void setup()
{
    Serial.begin(115200);

    auto emitter = std::make_unique<npb::PrintEmitter>(
        PixelCount,
        nullptr,
        npb::PrintEmitterSettings{
            Serial,
            npb::ColorOrderTransformConfig{
                .channelCount = 3,
                .channelOrder = {1, 0, 2, 0, 0}  // GRB
            }
        });

    static npb::PixelBus bus(PixelCount, std::move(emitter));
    bus.begin();
}

void loop()
{
    static uint8_t value = 0;

    // bus is static in setup — access via pointer or global
    // (simplified for illustration)
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
| 1 | `src/virtual/colors/Color.h` | header-only |
| 2 | `src/virtual/emitters/ITransformColorToBytes.h` | header-only interface |
| 3 | `src/virtual/emitters/ColorOrderTransform.h` | header + inline impl |
| 4 | `src/virtual/emitters/IEmitPixels.h` | header-only interface |
| 5 | `src/virtual/emitters/PrintEmitter.h` | header + inline impl |
| 6 | `src/virtual/IPixelBus.h` | header-only interface |
| 7 | `src/virtual/PixelBus.h` | header + inline impl |
| 8 | `src/VirtualNeoPixelBus.h` | umbrella header |
| 9 | `examples-virtual/smoke-phase1/smoke-phase1.ino` | smoke test |
| 10 | `platformio.ini` | add `[env:pico2w-virtual]` |

---

## Phase 2 — Shader Pipeline + GammaShader + CurrentLimiterShader

### 2.1 IShader

**File:** `src/virtual/shaders/IShader.h`

Per-pixel shader interface with a lifecycle: `begin(colors)` is called once before the pixel loop (receiving the full `span<const Color>` for pre-pass analysis), `apply()` is called for each pixel, and `end()` is called after all pixels are processed. Emitters own a `std::unique_ptr<IShader>` (nullable). Also contains `NilShader` (identity pass-through) and a `nilShader` global instance.

```cpp
namespace npb
{

class IShader
{
public:
    virtual ~IShader() = default;
    virtual void begin(std::span<const Color> /*colors*/){}
    virtual const Color apply(uint16_t index, const Color color) = 0;
    virtual void end() {}
};

class NilShader : public IShader
{
public:
    const Color apply(uint16_t, const Color color) override
    {
        return color;
    }
};

} // namespace npb
```

### 2.2 Gamma Methods (duck-typed, compile-time)

Gamma methods are lightweight structs with a static `correct()` method — no virtual dispatch. The compiler inlines them into the per-channel loop. All operate at 8-bit precision (the internal `Color` width).

**Required interface (duck-typed):**
```cpp
struct SomeGammaMethod
{
    static constexpr uint8_t correct(uint8_t value);
    // — or non-constexpr for table/dynamic methods
};
```

| # | Class | File | Approach | Memory | Configurable |
|---|-------|------|----------|--------|--------------|
| 1 | `GammaEquationMethod` | `GammaEquationMethod.h` | `pow(x, 1/0.45)` ≈ γ 2.222 | 0 | No |
| 2 | `GammaCieLabMethod` | `GammaCieLabMethod.h` | CIE L* piecewise curve | 0 | No |
| 3 | `GammaTableMethod` | `GammaTableMethod.h` | Static 256-entry LUT (flash) | 256 B flash | No |
| 4 | `GammaDynamicTableMethod` | `GammaDynamicTableMethod.h` | Runtime-filled 256 LUT | 256 B RAM | Yes — custom curve function passed to `initialize()` |
| 5 | `GammaNullMethod` | `GammaNullMethod.h` | Identity pass-through | 0 | No |
| 6 | `GammaInvertMethod<T>` | `GammaInvertMethod.h` | Decorator template — `~T::correct(v)` | per `T` | Yes — wraps any method |

All files live under `src/virtual/shaders/`.

**Key design points:**
- All methods work at 8-bit natively (same precision as `Color` channels).
- Static method dispatch via templates — zero vtable overhead in the hot per-pixel loop.
- `GammaInvertMethod<T>` is a template decorator wrapping any gamma method at compile time.
- `GammaDynamicTableMethod` is the only stateful method (non-static `correct()` after `initialize()`).

**GammaEquationMethod** implementation:
```cpp
struct GammaEquationMethod
{
    static uint8_t correct(uint8_t value)
    {
        if (value == 0) return 0;
        if (value == 255) return 255;
        float unit = static_cast<float>(value) / 255.0f;
        float corrected = powf(unit, 1.0f / 0.45f);
        return static_cast<uint8_t>(corrected * 255.0f + 0.5f);
    }
};
```

**GammaCieLabMethod** implementation:
```cpp
struct GammaCieLabMethod
{
    static uint8_t correct(uint8_t value)
    {
        if (value == 0) return 0;
        if (value == 255) return 255;
        float unit = static_cast<float>(value) / 255.0f;
        float corrected;
        if (unit <= 0.08f)
            corrected = unit / 9.033f;
        else
            corrected = powf((unit + 0.16f) / 1.16f, 3.0f);
        return static_cast<uint8_t>(corrected * 255.0f + 0.5f);
    }
};
```

**GammaNullMethod** implementation:
```cpp
struct GammaNullMethod
{
    static constexpr uint8_t correct(uint8_t value) { return value; }
};
```

**GammaInvertMethod** implementation:
```cpp
template<typename T_METHOD>
struct GammaInvertMethod
{
    static constexpr uint8_t correct(uint8_t value)
    {
        return static_cast<uint8_t>(~T_METHOD::correct(value));
    }
};
```

### 2.3 GammaShader

**File:** `src/virtual/shaders/GammaShader.h`

Template class that applies a compile-time gamma method per-channel, per-pixel. Type-erases into `IShader` for the shader pipeline.

```cpp
namespace npb
{

template<typename T_GAMMA>
class GammaShader : public IShader
{
public:
    const Color apply(uint16_t /*index*/, const Color color) override
    {
        Color result;
        for (size_t ch = 0; ch < Color::ChannelCount; ++ch)
        {
            result[ch] = T_GAMMA::correct(color[ch]);
        }
        return result;
    }
};

} // namespace npb
```

The virtual call happens once per pixel at the `IShader` level. The inner `T_GAMMA::correct()` calls are fully inlined by the compiler — no vtable dispatch in the hot loop.

### 2.4 CurrentLimiterShader

**File:** `src/virtual/shaders/CurrentLimiterShader.h`

Currently **disabled** (`#if 0`). The original two-pass approach (sum all pixels then scale proportionally) does not fit the per-pixel `apply()` model cleanly. With `begin(span<const Color>)` now receiving the full color buffer, the shader can compute the scaling factor in `begin()` from the current frame's colors directly, then apply the scale per-pixel.

```cpp
namespace npb
{

class CurrentLimiterShader : public IShader
{
public:
    CurrentLimiterShader(uint32_t maxMilliamps,
                         uint16_t milliampsPerChannel);

    void begin(std::span<const Color> colors) override; // compute scale from colors
    const Color apply(uint16_t index,
                      const Color color) override;       // scale per-pixel
    void end() override;                                 // no-op or stats

private:
    uint32_t _maxMilliamps;
    uint16_t _milliampsPerChannel;
    // frame state TBD
};

} // namespace npb
```

### 2.6 ShaderChain

**File:** `src/virtual/shaders/ShadedTransform.h`

Chains multiple `IShader*` instances into a single `IShader`. Implements `IShader` (not `ITransformColorToBytes`). Delegates `begin(colors)`/`apply()`/`end()` to each shader in sequence.

```cpp
namespace npb
{

class ShaderChain : public IShader
{
public:
    explicit ShaderChain(std::span<IShader* const> shaders);

    void begin(std::span<const Color> colors) override;
    const Color apply(uint16_t index, const Color color) override;
    void end() override;

private:
    std::span<IShader* const> _shaders;
};

} // namespace npb
```

`apply()` runs each shader's `apply()` in order, threading the color through. `begin(colors)`/`end()` delegates to all shaders.

### 2.7 Smoke Test

**File:** `examples-virtual/gamma-and-limiter/gamma-and-limiter.ino` (Arduino IDE)  
**File:** `examples-virtual/gamma-and-limiter/main.cpp` (PlatformIO)

Verify gamma curves with multiple methods, current budget clamping, and that original `Color` values in `PixelBus` remain unmodified after `show()`.

### Phase 2 File Manifest

| # | File | Type |
|---|------|------|
| 1 | `src/virtual/shaders/IShader.h` | header-only interface (includes NilShader) |
| 2 | `src/virtual/shaders/GammaEquationMethod.h` | header-only |
| 3 | `src/virtual/shaders/GammaCieLabMethod.h` | header-only |
| 4 | `src/virtual/shaders/GammaTableMethod.h` | header + static data |
| 5 | `src/virtual/shaders/GammaDynamicTableMethod.h` | header + impl |
| 6 | `src/virtual/shaders/GammaNullMethod.h` | header-only |
| 7 | `src/virtual/shaders/GammaInvertMethod.h` | header-only template |
| 8 | `src/virtual/shaders/GammaShader.h` | header-only template |
| 9 | `src/virtual/shaders/CurrentLimiterShader.h` | header + inline impl (disabled) |
| 10 | `src/virtual/shaders/ShadedTransform.h` | header + inline impl (ShaderChain) |
| 11 | `examples-virtual/gamma-and-limiter/main.cpp` | smoke test |

## Phase 3 — Two-Wire Infrastructure

- `IClockDataBus` interface (`src/virtual/buses/IClockDataBus.h`) 
- `ClockDataProtocol` descriptor struct (`src/virtual/buses/ClockDataProtocol.h`)
- `ClockDataEmitter` (`src/virtual/emitters/ClockDataEmitter.h`) — protocol-descriptor-driven framing + `IClockDataBus` delegation. Takes `IClockDataBus&`, `ClockDataProtocol&`, `ITransformColorToBytes&`, `unique_ptr<IShader>`, and `pixelCount`. Owns byte buffer internally.
- `DebugClockDataBus` (`src/virtual/buses/DebugClockDataBus.h`) — `Print`-based debug implementation. Takes an optional pointer to an inner `IClockDataBus` to wrap.
- `SpiClockDataBus` (`src/virtual/buses/SpiClockDataBus.h`) — `SPI`-based implementation. Will panic for `transmitBit` calls.
- `DotStarTransform` (`src/virtual/emitters/DotStarTransform.h`) — APA102/HD108 serialization (0xFF/0xE0 prefix, luminance byte). Internal to emitters.
- Example: `examples-virtual/dotstar-debug/main.cpp` — verify DotStar framing, start/end frames, pixel byte layout

### Phase 3 File Manifest

| # | File | Type |
|---|------|------|
| 1 | `src/virtual/buses/IClockDataBus.h` | header-only interface |
| 2 | `src/virtual/buses/ClockDataProtocol.h` | header-only struct |
| 3 | `src/virtual/buses/DebugClockDataBus.h` | header + inline impl |
| 4 | `src/virtual/buses/SpiClockDataBus.h` | header + impl |
| 5 | `src/virtual/emitters/ClockDataEmitter.h` | header + inline impl |
| 6 | `src/virtual/emitters/DotStarTransform.h` | header + inline impl |
| 7 | `examples-virtual/dotstar-debug/main.cpp` | smoke test |

---

## Phase 4 — Remaining Two-Wire Chip Emitters

Goal: implement emitter classes for all remaining two-wire LED chips. Each chip-specific emitter **rolls the transform logic into the emitter class itself** — there is no standalone transform file. The emitter internally knows how to serialize pixels for its chip and how to frame the transmission.

All chip emitters follow the same pattern: they implement `IEmitPixels`, accept a `pixelCount`, `unique_ptr<IShader>`, and a chip-specific settings struct (containing `IClockDataBus&` plus any per-chip options), own their byte buffer internally, and handle chip-specific serialization + protocol framing in `update()`.

### 4.1 Chip Emitters

| # | Class | File | Wire Format | Notes |
|---|-------|------|-------------|-------|
| 1 | `Lpd8806Emitter` | `emitters/Lpd8806Emitter.h` | 7-bit per channel, MSB set (3 bytes/pixel) | `(color >> 1) \| 0x80` per channel. Latch: `ceil(N/32)` zero bytes |
| 2 | `Lpd6803Emitter` | `emitters/Lpd6803Emitter.h` | 5-5-5 packed (2 bytes/pixel) | 15-bit color, 1-bit flag. Start: 4×0x00, end: `ceil(N/8)` zeros |
| 3 | `P9813Emitter` | `emitters/P9813Emitter.h` | Checksum prefix + 3 color bytes (4 bytes/pixel) | Prefix: `0xC0 \| (~B5B4 << 4) \| (~G5G4 << 2) \| ~R5R4`. Start/end: 4×0x00 |
| 4 | `Ws2801Emitter` | `emitters/Ws2801Emitter.h` | Raw 3-byte passthrough | 500µs latch. Uses `ColorOrderTransform` internally |
| 5 | `Tlc59711Emitter` | `emitters/Tlc59711Emitter.h` | Reversed 16-bit per channel + 4-byte inline header per 4 pixels | Per-chip command/brightness config in header |
| 6 | `Tlc5947Emitter` | `emitters/Tlc5947Emitter.h` | 12-bit packing, reversed per module (24 channels/chip) | Requires latch pin toggling |
| 7 | `Sm16716Emitter` | `emitters/Sm16716Emitter.h` | 48-bit start frame + per-pixel HIGH bit, bit-level framing | Uses `transmitBit()` on bus |
| 8 | `Mbi6033Emitter` | `emitters/Mbi6033Emitter.h` | 6-byte data per chip, chip-count-aligned | Reset protocol (21µs toggle) |

All files: `src/virtual/emitters/`

### 4.2 Emitter Constructor Pattern

All emitters follow a uniform constructor signature:
`FooEmitter(uint16_t pixelCount, std::unique_ptr<IShader> shader, FooEmitterSettings settings)`

The settings struct aggregates the bus reference and any chip-specific options.

```cpp
struct Lpd8806EmitterSettings
{
    IClockDataBus& bus;
    std::array<uint8_t, 3> channelOrder = {1, 0, 2};  // GRB default
};

class Lpd8806Emitter : public IEmitPixels
{
public:
    Lpd8806Emitter(uint16_t pixelCount,
                   std::unique_ptr<IShader> shader,
                   Lpd8806EmitterSettings settings);

    void update(std::span<const Color> colors) override
    {
        // 1. Serialize each pixel: optionally shade, then encode
        //    (7-bit with MSB set, in configured channel order)
        // 2. Transmit: pixel bytes + ceil(N/32) latch zeros
    }

private:
    IClockDataBus& _bus;
    std::unique_ptr<IShader> _shader;
    std::vector<uint8_t> _byteBuffer;
    std::array<uint8_t, 3> _channelOrder;
    uint16_t _pixelCount;
};
```

### 4.3 Smoke Test

**File:** `examples-virtual/two-wire-chips/main.cpp`

Exercise each chip emitter with `DebugClockDataBus` → Serial. Verify byte layout matches the original library's output for known pixel values.

### Phase 4 File Manifest

| # | File | Type |
|---|------|------|
| 1 | `src/virtual/emitters/Lpd8806Emitter.h` | header + inline impl |
| 2 | `src/virtual/emitters/Lpd6803Emitter.h` | header + inline impl |
| 3 | `src/virtual/emitters/P9813Emitter.h` | header + inline impl |
| 4 | `src/virtual/emitters/Ws2801Emitter.h` | header + inline impl |
| 5 | `src/virtual/emitters/Tlc59711Emitter.h` | header + inline impl |
| 6 | `src/virtual/emitters/Tlc5947Emitter.h` | header + inline impl |
| 7 | `src/virtual/emitters/Sm16716Emitter.h` | header + inline impl |
| 8 | `src/virtual/emitters/Mbi6033Emitter.h` | header + inline impl |
| 9 | `examples-virtual/two-wire-chips/main.cpp` | smoke test |

---

## Phase 5 — In-Band Settings + Additional Buses

### 5.1 In-Band Settings

In-band settings (per-chip current limits, gain values, mode bytes) are owned by the emitter as plain constructor config and encoded inline during `update()`. There is no separate settings type hierarchy, no `SettingsData` shuttle struct, and no routing through the transform.

Each chip-specific emitter that needs in-band settings defines a simple config struct with the user-facing fields. The emitter's `update()` method encodes the config into the byte stream at the appropriate position — it already knows the channel order and wire format.

**Two-wire emitters (implemented — Phase 5.1):**

| # | Emitter | Config Struct | Position | Status |
|---|---------|---------------|----------|--------|
| 1 | `Tlc59711Emitter` | `Tlc59711Config{bcRed, bcGreen, bcBlue, outtmg, ...}` | Inline per-chip 4-byte header | ✅ Done |
| 2 | `Tlc5947Emitter` | N/A (no in-band settings, GPIO latch) | N/A | ✅ Done |

**One-wire emitters (deferred — built inline when Phase 6 platform emitters are created):**

These chips use one-wire NRZ protocols. Their config structs and serialization logic will be built directly into each chip's emitter (not as separate transform files). The emitter owns its config, encodes settings bytes, and serializes pixel data — all in `update()`.

| # | Future Emitter | Config Fields | Encoding Summary |
|---|----------------|---------------|------------------|
| 1 | TM1814 variant | `{redCurrent, greenCurrent, blueCurrent, whiteCurrent}` | 8 bytes prepended: C1 gain `(clamp(v,65,380)-65)/5` + C2 `~C1`, WRGB fixed order |
| 2 | TM1914 variant | `{mode}` enum (DinFdin/DinOnly/FdinOnly) | 6 bytes prepended: C1 `{0xFF,0xFF,mode_byte}` + C2 `~C1` |
| 3 | SM168x 3-ch variant | `{gainR, gainG, gainB}` 4-bit, packing enum | 2 bytes appended: SM16803PB `0x0R,0xGB` or SM16823E `0xRG,0xB0` |
| 4 | SM168x 4-ch variant | `{gainR, gainG, gainB, gainW}` 4-bit | 2 bytes appended: `0xRG,0xBW` |
| 5 | SM16825E 5-ch variant | `{gainR, gainG, gainB, gainWW, gainCW}` 5-bit | 4 bytes appended: 25 bits packed + control, 16-bit BE pixel data |

Previously created standalone transform files (`Tm1814Transform.h`, etc.) and settings files (`Tm1814Settings.h`, `SettingsData.h`, etc.) have all been deleted — their logic will be inlined into the emitters.

### 5.2 Additional IClockDataBus Implementations

| # | Class | File | Platform | Notes |
|---|-------|------|----------|-------|
| 1 | `Esp32DmaSpiClockDataBus` | `buses/Esp32DmaSpiClockDataBus.h` | ESP32 | DMA-accelerated SPI via `spi_master.h`; see `DotStarEsp32DmaSpiMethod.h` |

`HspiClockDataBus` is not needed — `SpiClockDataBus` already accepts any `SPIClass&` (pass HSPI instance on ESP32).

### 5.3 Smoke Test

**File:** `examples-virtual/in-band-settings/main.cpp`

Verify settings bytes appear at correct stream positions using `ClockDataEmitter` + `DebugClockDataBus`.

### Phase 5 File Manifest

| # | File | Type |
|---|------|------|
| 1 | `src/virtual/emitters/Tlc59711Emitter.h` | new — includes inline brightness config |
| 2 | `src/virtual/emitters/Tlc5947Emitter.h` | new — includes GPIO latch pin |
| 3 | `src/virtual/buses/Esp32DmaSpiClockDataBus.h` | header + impl |
| 6 | `examples-virtual/in-band-settings/main.cpp` | smoke test |

---

## Phase 6 — Platform Emitters (One-Wire)

All one-wire emitters implement `IEmitPixels`. They accept `span<const Color>`, own a `ColorOrderTransform` and optional `unique_ptr<IShader>` internally, manage their own byte/DMA buffers, and handle platform-specific bit timing.

### 6.1 Common

**File:** `src/virtual/emitters/OneWireTiming.h`

`OneWireTiming` describes only the NRZ bit encoding durations and reset interval. Signal inversion is **not** part of timing — it is a separate hardware-output concern handled by each platform emitter (see § 6.1.1).

```cpp
namespace npb
{

struct OneWireTiming
{
    uint32_t t0hNs, t0lNs, t1hNs, t1lNs;
    uint32_t resetUs;
};

namespace timing
{
    inline constexpr OneWireTiming Ws2812x   = { 400, 850, 800, 450, 300 };
    inline constexpr OneWireTiming Ws2811     = { 500, 2000, 1200, 1300, 50 };
    inline constexpr OneWireTiming Ws2805     = { 300, 790, 790, 300, 300 };
    inline constexpr OneWireTiming Sk6812     = { 400, 850, 800, 450, 80 };
    inline constexpr OneWireTiming Tm1814     = { 360, 720, 720, 360, 200 };
    inline constexpr OneWireTiming Tm1914     = { 360, 720, 720, 360, 200 };
    inline constexpr OneWireTiming Tm1829     = { 300, 800, 800, 300, 500 };
    inline constexpr OneWireTiming Apa106     = { 350, 1360, 1360, 350, 50 };
    inline constexpr OneWireTiming Tx1812     = { 300, 600, 600, 300, 80 };
    inline constexpr OneWireTiming Gs1903     = { 300, 900, 900, 300, 40 };
    inline constexpr OneWireTiming Generic800 = { 400, 850, 800, 450, 50 };
    inline constexpr OneWireTiming Generic400 = { 500, 2000, 1200, 1300, 50 };

    // Aliases — identical timing, different chip branding
    inline constexpr auto Ws2816 = Ws2812x;
    inline constexpr auto Ws2813 = Ws2812x;
    inline constexpr auto Ws2814 = Ws2805;
    inline constexpr auto Lc8812 = Sk6812;
} // namespace timing

} // namespace npb
```

#### 6.1.1 Signal Inversion

Signal inversion compensates for **external inverting hardware** between the MCU and the LED strip — typically an inverting level-shifter or buffer IC. When `invert` is set, the emitter produces the logical NOT of its normal output so that after the external hardware flips the signal again, the LED chip receives the correct waveform.

This is purely about the electrical path outside the microcontroller. Every chip's protocol (including TM1814, TM1829, TM1914) defines its own signal encoding and timing — the emitter always produces the correct protocol signals for its chip. Inversion is orthogonal: it tells the emitter "produce everything backwards because something downstream will flip the signal before it reaches the LED."

It is configured per-emitter at construction time via a `bool invert` constructor parameter and is **not** embedded in `OneWireTiming`.

**How each platform applies inversion:**

- **ESP32 RMT** — swaps the idle-level (`idle_level`) and high/low durations in the RMT translator function. The original library encodes this as separate `NeoEsp32RmtSpeed*` vs `NeoEsp32RmtInvertedSpeed*` types.
- **ESP32 I2S** — applied at the GPIO matrix level via `i2sSetPins(..., inverted)`. DMA buffer encoding is unchanged; inversion happens in the output hardware.
- **ESP8266 UART** — a UART-register flag (`NeoEsp8266UartInverted` vs `NeoEsp8266UartNotInverted`).
- **RP2040 PIO** — pin inversion in the PIO pinctrl config or `set pins` / `side-set` directives.

**Default `invert = false` for all chips.** No chip *inherently requires* inversion — inversion is always a property of the external circuit, not the protocol. The user sets `invert = true` when their wiring includes an inverting element.

**Constructor pattern (all platform one-wire emitters):**

```cpp
struct Esp32RmtOneWireEmitterSettings
{
    uint8_t pin;
    OneWireTiming timing;
    bool invert = false;               // compensate for external inverting hardware
    ColorOrderTransformConfig colorConfig;
};

Esp32RmtOneWireEmitter(uint16_t pixelCount,
                       std::unique_ptr<IShader> shader,
                       Esp32RmtOneWireEmitterSettings settings);
```

Factory functions (Phase 8) default `invert` to `false`; the user overrides it when their hardware requires it:

```cpp
auto bus = npb::makeNeoPixelBus(pixelCount, pin);                   // invert=false (default)
auto bus = npb::makeNeoPixelBus(pixelCount, pin, GrbConfig, nullptr, true);  // invert=true for inverting level-shifter
```

### 6.2 RP2040

| # | Class | File | Description |
|---|-------|------|-------------|
| 1 | `RpPioOneWireEmitter` | `emitters/RpPioOneWireEmitter.h` | PIO + DMA, single strip per instance. See `NeoRp2040x4Method.h` |

There is no separate parallel emitter class. Each `RpPioOneWireEmitter` instance drives one strip on one pin. Internally, the emitter **automatically claims the next available state machine** on the user-selected PIO block. Each RP2040 PIO block has 4 state machines, so up to 4 strips can share one PIO block — one `RpPioOneWireEmitter` per strip, each auto-allocated to its own state machine.

The user is responsible for choosing which PIO block to use (PIO0 or PIO1) and for not exceeding 4 instances per block. The emitter does **not** manage cross-instance scheduling or validate block capacity.

**DMA sharing:** The `RpPioOneWireEmitter` must gracefully share DMA channels with other possible `RpPio*Emitter` variants that may be added in the future (e.g. a hypothetical `RpPioClockDataEmitter` for two-wire protocols over PIO). DMA channel allocation should be centralized or use a cooperative claim/release mechanism so that multiple PIO-based emitter types can coexist without conflicts.

Constructor pattern:
```cpp
struct RpPioOneWireEmitterSettings
{
    uint8_t pin;
    uint8_t pioIndex;                  // 0 = PIO0, 1 = PIO1
    OneWireTiming timing;
    bool invert = false;
    ColorOrderTransformConfig colorConfig;
};

RpPioOneWireEmitter(uint16_t pixelCount,
                    std::unique_ptr<IShader> shader,
                    RpPioOneWireEmitterSettings settings);
```

For chips with in-band settings (TM1814, TM1914, SM168x family), the emitter will own a chip-specific config struct and encode settings bytes directly in `update()`, alongside the pixel data produced by the transform. The settings encoding logic (header prepend / gain append) is built into the emitter — **not** in separate transform files. See the Phase 5.1 one-wire table for encoding specs.

Example: `examples-virtual/rp2040-neopixel/main.cpp` — integration test on pico2w hardware.

### 6.3 ESP32

| # | Class | File | Description |
|---|-------|------|-------------|
| 1 | `Esp32RmtOneWireEmitter` | `emitters/Esp32RmtOneWireEmitter.h` | RMT peripheral, one channel per strip. Inversion changes RMT idle-level + translator. See `NeoEsp32RmtMethod.h` |
| 2 | `Esp32I2sOneWireEmitter` | `emitters/Esp32I2sOneWireEmitter.h` | I2S single-channel DMA (ESP32 original only). Inversion via GPIO matrix. See `NeoEsp32I2sMethod.h` |
| 3 | `Esp32I2sParallelOneWireEmitter` | `emitters/Esp32I2sParallelOneWireEmitter.h` | I2S parallel multi-channel DMA (ESP32 original). Inversion via GPIO matrix. See `NeoEsp32I2sXMethod.h` |
| 4 | `Esp32LcdParallelOneWireEmitter` | `emitters/Esp32LcdParallelOneWireEmitter.h` | LCD-CAM + GDMA parallel (ESP32-S3, up to 8 pins). See `NeoEsp32LcdXMethod.h` |

All ESP32 one-wire emitters accept `bool invert` in their constructor (see § 6.1.1).

### 6.4 ESP8266

| # | Class | File | Description |
|---|-------|------|-------------|
| 1 | `Esp8266DmaOneWireEmitter` | `emitters/Esp8266DmaOneWireEmitter.h` | I2S + DMA (fixed GPIO3). Inversion via I2S output config. See `NeoEsp8266DmaMethod.h` |
| 2 | `Esp8266UartOneWireEmitter` | `emitters/Esp8266UartOneWireEmitter.h` | UART TX bit-shaping (sync & async). Inversion via UART register flag. See `NeoEsp8266UartMethod.h` |

All ESP8266 one-wire emitters accept `bool invert` in their constructor (see § 6.1.1).

### 6.5 nRF52

| # | Class | File | Description |
|---|-------|------|-------------|
| 1 | `Nrf52PwmOneWireEmitter` | `emitters/Nrf52PwmOneWireEmitter.h` | nRF52840 hardware PWM + DMA. See `NeoNrf52xMethod.h` |

### Phase 6 File Manifest

| # | File | Type |
|---|------|------|
| 1 | `src/virtual/emitters/OneWireTiming.h` | header-only |
| 2 | `src/virtual/emitters/RpPioOneWireEmitter.h` | header + impl |
| 3–6 | ESP32 one-wire emitters (4 files) | header + impl |
| 7–8 | ESP8266 one-wire emitters (2 files) | header + impl |
| 9 | `src/virtual/emitters/Nrf52PwmOneWireEmitter.h` | header + impl |
| 10 | `examples-virtual/rp2040-neopixel/main.cpp` | integration test |

---

## Phase 7 — Serial-Protocol Emitters

Serial-protocol emitters that are **not** one-wire NRZ. These are handled separately because they use fundamentally different framing (async serial / DMX512) rather than NRZ bit timing.

### 7.1 Pixie

| # | Class | File | Description |
|---|-------|------|-------------|
| 1 | `PixieStreamEmitter` | `emitters/PixieStreamEmitter.h` | Pixie LEDs over any Arduino `Stream` (115.2 Kbps, 1 ms latch). `alwaysUpdate()` returns true. See `PixieStreamMethod.h` |

### 7.2 DMX512

| # | Class | File | Description |
|---|-------|------|-------------|
| 1 | `Esp8266I2sDmx512Emitter` | `emitters/Esp8266I2sDmx512Emitter.h` | I2S DMX512 (250 Kbps, ESP8266 only). See `NeoEsp8266I2sDmx512Method.h` |

### Phase 7 File Manifest

| # | File | Type |
|---|------|------|
| 1 | `src/virtual/emitters/PixieStreamEmitter.h` | header-only |
| 2 | `src/virtual/emitters/Esp8266I2sDmx512Emitter.h` | header + impl |

---

## Phase 8 — Convenience Aliases + Migration

### 8.1 Factory Functions

Pre-configured factory functions that construct a `PixelBus` with the right emitter, transform, and shader for common LED types. Return `std::unique_ptr<PixelBus>` or `PixelBus` by value.

```cpp
namespace npb
{
    // Example factory (exact API TBD):
    std::unique_ptr<PixelBus> makeNeoPixelBus(
        size_t pixelCount,
        uint8_t pin,
        const ColorOrderTransformConfig& colorConfig = GrbConfig,
        std::unique_ptr<IShader> shader = nullptr);
}
```

**Signal inversion and two-wire buses:** For one-wire emitters, the `invert` flag is handled directly by the platform peripheral (PIO pinctrl, RMT idle-level, etc.). For two-wire chip emitters built on `IClockDataBus`, the factory must propagate the `invert` flag down to the underlying bus implementation. In the SPI case this means selecting a different `SPI_MODE` — normal operation uses `SPI_MODE0` (CPOL=0, CPHA=0) while inverted operation requires `SPI_MODE2` (CPOL=1, CPHA=0) or `SPI_MODE3` (CPOL=1, CPHA=1) so that the idle clock and data polarity are flipped. The factory function is responsible for mapping the user-facing `invert` boolean to the correct bus-level configuration.

### 8.2 Pre-defined Color Configs

Common `ColorOrderTransformConfig` constants:

```cpp
namespace npb
{
    inline constexpr ColorOrderTransformConfig GrbConfig  = { 3, {1, 0, 2, 0, 0} };
    inline constexpr ColorOrderTransformConfig GrbwConfig = { 4, {1, 0, 2, 3, 0} };
    inline constexpr ColorOrderTransformConfig RgbConfig  = { 3, {0, 1, 2, 0, 0} };
    inline constexpr ColorOrderTransformConfig RgbwConfig = { 4, {0, 1, 2, 3, 0} };
    // ... etc.
}
```

### 8.3 Optional Compatibility Adapter

Maps old `NeoPixelBus<F,M>` API surface onto new `PixelBus`. This is a thin wrapper that translates `SetPixelColor(index, RgbColor)` → `setPixelColor(index, Color)`, etc. Low priority — may be deferred or dropped.

---

## Future Work (Deferred)

Items identified but intentionally out of scope for Phases 1–8:

- **UCS8903 / UCS8904 support** — 16-bit-per-channel one-wire chips (6 bytes/pixel RGB, 8 bytes/pixel RGBW). Use standard WS2812x timing but require 8→16 bit up-conversion per channel during serialization. Needs either a dedicated `Ucs8903Emitter` / `Ucs8904Emitter` or a `bitsPerChannel` extension to `ColorOrderTransform`.

- **HD108 emitter** — 16-bit-per-channel two-wire chip, distinct from APA102's 8-bit format. Currently grouped under DotStar; may need its own emitter or a parameterized bit-width in the DotStar transform.
- **DMX512 / WS2821 emitter** — DMX512 is a standard 250 Kbps async serial protocol (8N2 + Break/MAB preamble); WS2821 is a 750 Kbps variant with the same framing. The original library only implements these on ESP8266 (via I2S bit-shaping), but the protocol is platform-agnostic. A portable `Dmx512StreamEmitter` over any Arduino `Stream`/UART at 250 Kbps would cover most platforms. Platform-optimized DMA variants (ESP32 RMT, RP2040 PIO) could follow if needed.
