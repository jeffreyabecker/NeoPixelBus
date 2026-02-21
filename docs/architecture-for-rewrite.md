# NeoPixelBus Architecture Analysis for Rewrite

## Scope

This document analyzes only the architecture needed to replace `NeoPixelBus<T_COLOR_FEATURE, T_METHOD>` and its supporting types. It excludes topologies, animations, segment features, non-RGB color models (HSB/HSL/HTML), and gamma utilities.

---

## 1. Current Architecture Overview

### 1.1 Template Structure

The library is organized around a single top-level template:

```cpp
template<typename T_COLOR_FEATURE, typename T_METHOD>
class NeoPixelBus { ... };
```

- **T_COLOR_FEATURE** — a static utility class that knows how to serialize/deserialize a `ColorObject` to/from a raw `uint8_t*` buffer, in the byte/bit order the target LED chip expects. It also carries optional per-chip settings (current control, gain, mode) that get prepended or appended to the pixel data stream.
- **T_METHOD** — owns the raw data buffer, knows how to initialize hardware, and knows how to send the buffer out to the wire.

These two template parameters are **type-system orthogonal** — neither references the other. `NeoPixelBus` glues them together by:
1. Forwarding `T_COLOR_FEATURE::PixelSize` and `T_COLOR_FEATURE::SettingsSize` as plain integers to `T_METHOD`'s constructor (so the method allocates the right buffer size).
2. Calling `T_COLOR_FEATURE::applyPixelColor()` / `T_COLOR_FEATURE::applySettings()` against the buffer pointer returned by `T_METHOD::getData()`. *(In the rewrite, `applySettings()` is eliminated — all configuration is constructor-injected.)*

**However, there is strong semantic coupling** — certain features only produce valid results with compatible methods (e.g., DotStar features with DotStar methods, TM1814 features with one-wire timing methods). The library relies entirely on the user to pair them correctly.

### 1.2 Buffer Ownership

In the current design, **T_METHOD owns all pixel data**. `T_METHOD` allocates a single `uint8_t*` buffer of size `(pixelCount * pixelSize) + settingsSize` via `malloc()`. Some methods (RP2040 PIO, ESP32 RMT) double-buffer with `_dataEditing` / `_dataSending` and expose `SwapBuffers()`. Feature classes never allocate anything; they operate purely on raw pointers passed in.

### 1.3 Feature Inheritance Tree

Features use dual inheritance: one base for pixel serialization, one for settings.

```
NeoElementsBase<PixelSize, ColorObject, CopyType>
├── NeoByteElements  (8-bit element types)
│   ├── Neo3ByteFeature<IC1,IC2,IC3>         → RgbColor, 3 bytes
│   ├── Neo4ByteFeature<IC1,IC2,IC3,IC4>     → RgbwColor, 4 bytes
│   ├── Neo5ByteFeature<IC1..IC5>            → RgbwwColor, 5 bytes
│   ├── Neo6ByteFeature<IC1..IC6>            → RgbwwwColor, 6 bytes
│   ├── Neo6xByteFeature<IC1..IC5>           → RgbwwColor, 6 bytes (5 used + 1 pad)
│   ├── Neo6xxByteFeature<IC1..IC4>          → RgbwColor, 6 bytes (4 used + 2 pad)
│   ├── Neo2Byte555Feature<IC1,IC2,IC3>      → RgbColor, 2 bytes (packed 5-5-5)
│   ├── Neo3Byte777Feature<IC1,IC2,IC3>      → RgbColor, 3 bytes (7-bit channels)
│   ├── DotStarX4ByteFeature<IC1,IC2,IC3>    → RgbColor, 4 bytes (0xFF prefix + 3 color)
│   ├── DotStarL4ByteFeature<IC1,IC2,IC3>    → RgbwColor, 4 bytes (5-bit lum + 3 color)
│   └── P9813BgrFeature                      → RgbColor, 4 bytes (checksum + BGR)
│
└── NeoWordElements  (16-bit element types)
    ├── Neo3WordFeature<IC1,IC2,IC3>          → Rgb48Color, 6 bytes
    ├── Neo4WordFeature<IC1..IC4>             → Rgbw64Color, 8 bytes
    ├── Neo5WordFeature<IC1..IC5>             → Rgbww80Color, 10 bytes
    ├── DotStarX4WordFeature<IC1,IC2,IC3>     → Rgb48Color, 8 bytes
    └── DotStarL4WordFeature<IC1,IC2,IC3>     → Rgbw64Color, 8 bytes
```

Settings mixin (second base):
```
NeoElementsNoSettings                      → SettingsSize = 0  (majority of features)
NeoElementsTm1814Settings<IC1..IC4>        → SettingsSize = 8  (prepended: RGBW current control)
Neo3ByteElementsTm1914Settings<IC1..IC3>   → SettingsSize = 6  (prepended: DIN/FDIN mode)
Tlc59711ElementsSettings                   → SettingsSize = 4  (prepended: brightness/control)
NeoSm168x{3,4,5}SettingsBase              → SettingsSize = 2-4 (appended: gain control)
```

Concrete features (e.g., `NeoGrbFeature`) are just empty classes inheriting from both a serialization base and a settings base, with the IC-index template arguments set for a specific channel order.

### 1.4 Color Class Hierarchy

| Class | Channels | Element Type | Members |
|-------|----------|-------------|---------|
| `RgbColor` | 3 | `uint8_t` | R, G, B |
| `RgbwColor` | 4 | `uint8_t` | R, G, B, W |
| `RgbwwColor` | 5 | `uint8_t` | R, G, B, WW, CW |
| `RgbwwwColor` | 6 | `uint8_t` | R, G, B, W1, W2, W3 |
| `Rgb48Color` | 3 | `uint16_t` | R, G, B |
| `Rgbw64Color` | 4 | `uint16_t` | R, G, B, W |
| `Rgbww80Color` | 5 | `uint16_t` | R, G, B, WW, CW |

All support `operator[]` for indexed access. Higher-precision types can construct from lower-precision equivalents. All have a nested `SettingsObject` for current measurement.

### 1.5 Method Classification

Methods fall into two wire-type families plus one outlier:

**One-Wire (data only, timing-based protocol):** These are platform-specific, using DMA/PIO/RMT/I2S hardware.

| Platform | Base Class | Key Template Params | Double-Buffered |
|----------|-----------|-------------------|-----------------|
| RP2040 | `NeoRp2040x4MethodBase` | `<T_SPEED, T_PIO_INSTANCE, V_INVERT, V_IRQ>` | Yes |
| ESP32 RMT | `NeoEsp32RmtMethodBase` | `<T_SPEED, T_CHANNEL>` | Yes |
| ESP32 I2S | `NeoEsp32I2sMethodBase` | `<T_SPEED, T_BUS, T_INVERT, T_CADENCE>` | No |
| ESP32 I2sX/LcdX | `NeoEsp32I2sXMethodBase` / `NeoEsp32LcdXMethodBase` | `<T_SPEED, T_BUS, T_INVERT>` | No (AlwaysUpdate) |
| ARM | `NeoArmMethodBase` | `<T_SPEED>` | No |
| AVR | `NeoAvrMethodBase` | `<T_SPEED>` | No |
| ESP8266 | `NeoEsp8266DmaMethodBase` / `NeoEsp8266UartMethodBase` | `<T_SPEED, ...>` | Yes |

`T_SPEED` encodes per-chip timing constants (T0H, T1H, period, reset time) as `constexpr` values. Each LED chip gets a speed class (e.g., `NeoEsp32RmtSpeedWs2812x`, `NeoRp2040PioSpeedWs2812x`).

**Two-Wire (clock + data, SPI-like protocol):** These use `T_TWOWIRE` for the physical transport.

| Protocol Method Base | Framing | Notes |
|---------------------|---------|-------|
| `DotStarMethodBase<T_TWOWIRE>` | Start(4×0x00) + data + reset(4×0x00) + end(1bit/2px) | APA102 |
| `Ws2801MethodBase<T_TWOWIRE>` | Raw data, 500µs latch | WS2801 |
| `Lpd8806MethodBase<T_TWOWIRE>` | Start(N×0x00) + data + end(N×0xFF) | LPD8806 |
| `Lpd6803MethodBase<T_TWOWIRE>` | Start(4×0x00) + data + end(1bit/px) | LPD6803 |
| `P9813MethodBase<T_TWOWIRE>` | Start(4×0x00) + data + end(4×0x00) | P9813 |
| `Hd108MethodBase<T_TWOWIRE>` | Start(16×0x00) + data + end(4×0xFF) | HD108 |
| `Tlc59711MethodBase<T_TWOWIRE>` | Inline per-chip headers + reversed 16-bit data, 20µs latch | TLC59711 |
| `Tlc5947MethodBase<T_BITCONVERT, T_TWOWIRE>` | Latch-pin toggle, reverse-order 12-bit packed | TLC5947 |
| `Sm16716MethodBase<T_TWOWIRE>` | Bit-level framing (cannot use HW SPI) | SM16716 |
| `Mbi6033MethodBase<T_TWOWIRE>` | Reset protocol + 6-byte header per chip | MBI6033 |

**Serial stream:** `PixieStreamMethod` — uses `Stream*`, `AlwaysUpdate = true`, standalone (no template param).

### 1.6 T_TWOWIRE Implementations (IClockDataBus predecessors)

All T_TWOWIRE classes satisfy this duck-typed interface:

```cpp
// Implicit T_TWOWIRE contract:
typedef <type> SettingsObject;
void begin();                                              // Initialize hardware
void beginTransaction();                                   // Start SPI-like transaction
void endTransaction();                                     // End transaction
void transmitByte(uint8_t data);                           // Send one byte
void transmitBytes(const uint8_t* data, size_t dataSize);  // Send byte array
void transmitBit(uint8_t bit);                             // Send one bit (used by SM16716, MBI6033)
void applySettings(const SettingsObject& settings);        // Runtime config — DROPPED in rewrite
```

> **Rewrite note:** `applySettings()` and `SettingsObject` are eliminated entirely from `IClockDataBus`. Transport configuration (SPI speed, pin assignments) is passed as a constructor parameter. In-band chip settings (current limits, gain) are owned by `ITransformColorToBytes`, not the bus.

| Implementation | Transport | Settings |
|---------------|-----------|----------|
| `TwoWireBitBangImple` | GPIO bit-bang | `NeoNoSettings` |
| `TwoWireBitBangImpleAvr` | AVR-optimized GPIO | `NeoNoSettings` |
| `TwoWireSpiImple<T_SPISPEED>` | Arduino `SPI` (VSPI on ESP32) | speed-dependent |
| `TwoWireHspiImple<T_SPISPEED>` | ESP32 HSPI peripheral | speed-dependent |
| `TwoWireDebugImple` | Serial.print (debug output) | `NeoNoSettings` |

ESP32-specific DMA SPI (`DotStarEsp32DmaSpiMethodBase`) bypasses the `T_TWOWIRE` pattern entirely and directly manages the SPI peripheral with DMA.

---

## 2. Key Findings for the Rewrite

### 2.1 The Feature's Real Job Is Only Serialization

A T_COLOR_FEATURE does exactly three things:
1. **Serialize** a `ColorObject` → bytes at a pixel offset (`applyPixelColor`)
2. **Deserialize** bytes at a pixel offset → `ColorObject` (`retrievePixelColor`) — *dropped in the rewrite; read directly from the Color vector*
3. **Embed chip settings** into the data stream (`applySettings` + `pixels()` pointer offset) — *dropped in the rewrite; settings become a constructor parameter on `ITransformColorToBytes`, serialized internally during `apply()`*

The channel-reordering logic (the `V_IC_1..N` template parameters) is the core of serialization. Every concrete feature is just a permutation of channel indices.

### 2.2 Feature-Method Coupling Is Protocol-Level, Not Type-Level

The coupling breaks down into protocol families:

| Protocol Family | Compatible Feature Bases | Compatible Method Bases | Why Coupled |
|----------------|------------------------|------------------------|-------------|
| **NeoPixel-style** (WS2812, SK6812, TM1814, etc.) | `Neo{3,4,5,6,6x,6xx}ByteFeature`, `Neo{3,4,5}WordFeature`, TM/SM settings | All one-wire methods | Timing-based protocol, data sent linearly |
| **DotStar** (APA102, HD108) | `DotStar{X,L}4{Byte,Word}Feature` | `DotStarMethodBase`, `Hd108MethodBase`, `DotStarEsp32DmaSpi` | 0xFF/0xE0 per-pixel prefix byte, start/end frames |
| **LPD8806** | `Neo3Byte777Feature` (Lpd8806 features) | `Lpd8806MethodBase` | 7-bit channels (MSB set), specific framing |
| **LPD6803** | `Neo2Byte555Feature` (Lpd6803 features) | `Lpd6803MethodBase` | 5-5-5 packed, specific framing |
| **P9813** | `P9813BgrFeature` | `P9813MethodBase` | Checksum byte per pixel |
| **WS2801** | `Neo3ByteFeature` (standard RGB) | `Ws2801MethodBase` | Raw 3-byte, clock-latched |
| **TLC59711** | `Neo{3,4,5}WordFeature` + `Tlc59711ElementsSettings` | `Tlc59711MethodBase` | Reversed 16-bit data, inline headers — **tightest coupling** |
| **TLC5947** | Standard 3-byte features | `Tlc5947MethodBase` | 8→12 or 16→12 bit conversion in method |
| **SM16716** | Standard 3-byte features | `Sm16716MethodBase` | Bit-level framing |

### 2.3 What "Settings" Really Are

Settings fall into two categories:

1. **In-band chip configuration** (embedded in the pixel data stream):
   - TM1814: 8 bytes at stream front — per-channel current limits
   - TM1914: 6 bytes at stream front — input pin mode (DIN/FDIN)
   - SM168x: 2-4 bytes at stream end — per-channel gain
   - TLC59711: 4 bytes at stream front — brightness/command register

2. **Transport configuration** (never touches the pixel buffer):
   - SPI clock speed (`NeoSpiSettings`)
   - Method-specific settings (currently all `NeoNoSettings` for one-wire)

In the rewrite, in-band settings are a configuration parameter passed to the `ITransformColorToBytes` implementation at construction time. The transform produces the complete byte stream including any settings headers/footers. There is no separate settings inheritance branch — the transform owns the settings values and serializes them as part of `apply()`. Transport configuration (SPI speed, etc.) is a constructor parameter on `IEmitPixels` / `IClockDataBus`.

### 2.4 T_SPEED: Per-Chip Timing Constants

Each one-wire LED chip requires specific pulse timings. In the current design, each platform has its own set of speed classes encoding the same chip timing in platform-specific formats:

- RP2040: PIO cycle counts
- ESP32 RMT: RMT tick counts
- ESP32 I2S: bit patterns
- ARM: CPU cycle counts

These all describe the same logical information (T0H, T1H, T0L, T1L, reset time) but in incompatible platform-specific formats. The rewrite should abstract this to a single timing descriptor that each platform method translates internally.

---

## 3. Proposed Rewrite Architecture

### 3.1 Top-Level Types

```
Color                    — unified 16-bit RGBWW color (mirrors Rgbww80Color layout)
ITransformColors         — Color→Color shader pass (gamma, brightness, etc.); chainable
ITransformColorToBytes   — forward-only: span<const Color> → span<uint8_t>
IClockDataBus            — SPI-like byte/bit transmission interface
IEmitPixels              — emits a complete pixel buffer to hardware
IPixelBus                — top-level bus: owns buffers, orchestrates shaders + transform + emit
```

### 3.2 Color

Single class. Internally stores 5 × `uint16_t` channels: R, G, B, WW (warm white), CW (cool white). All internal math operates at 16-bit precision. Construction from 8-bit or partial-channel values scales up (e.g., 8-bit `0xFF` → 16-bit `0xFFFF`). Unused channels default to 0.

```cpp
class Color
{
    uint16_t R{0}, G{0}, B{0}, WW{0}, CW{0};
public:
    constexpr Color() = default;
    constexpr Color(uint16_t r, uint16_t g, uint16_t b,
                    uint16_t ww = 0, uint16_t cw = 0);
    // from 8-bit channels (auto-scales to 16-bit)
    static constexpr Color fromRgb8(uint8_t r, uint8_t g, uint8_t b);
    static constexpr Color fromRgbw8(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    // indexed access
    constexpr uint16_t operator[](size_t idx) const;
    uint16_t& operator[](size_t idx);
};
```

### 3.3 ITransformColors (Shader Pipeline)

Replaces `NeoPixelBusLg` and its `T_GAMMA`-templated `LuminanceShader`. In the original design, gamma/luminance correction was baked into a separate subclass (`NeoPixelBusLg<T_COLOR_FEATURE, T_METHOD, T_GAMMA>`) that overrode `SetPixelColor` / `ClearTo` to apply a per-pixel `Dim()` + `NeoGamma::Correct()` before writing to the byte buffer. This had two problems: the corrected values were written destructively into the wire buffer (so `GetPixelColor` returned the corrected value, not the original), and only one shader could be applied.

The rewrite replaces this with a chainable shader pipeline that operates on `Color` values in logical space, between the user's `_colors` buffer and the `ITransformColorToBytes` serialization step. Shaders never touch the byte buffer — they transform `Color` → `Color`.

```cpp
class ITransformColors
{
public:
    virtual ~ITransformColors() = default;

    // Transform colors in-place. Called during show() on a scratch copy
    // of the color buffer — the user's _colors vector is never modified.
    virtual void apply(std::span<Color> colors) = 0;
};
```

**Key design points:**

- **Non-destructive.** Shaders operate on a scratch copy of the color buffer, not the user's `_colors` vector. The user always reads back their original unmodified colors.
- **Chainable.** `ShadedTransform` holds a `std::span<ITransformColors* const>` pipeline (pointing to static shader instances). During `apply()`, each shader's `apply()` is called in sequence on the batch scratch buffer.
- **Per-pixel or bulk.** The interface receives the entire span, so implementations can choose per-pixel processing, SIMD, or lookup-table strategies.
- **Stateful.** Shader instances can hold configuration (gamma curve, brightness level, current limits) as member state, mutable at runtime.

**Concrete implementations:**

| Class | Replaces | Configuration |
|-------|----------|---------------|
| `GammaCorrectionShader` | `NeoGammaEquationMethod`, `NeoGammaCieLabEquationMethod`, `NeoGammaTableMethod` | Gamma equation or table, applied per-channel at 16-bit precision |
| `BrightnessShader` | `LuminanceShader` (the `Dim()` call in `NeoPixelBusLg`) | Global brightness 0–65535 (16-bit), applied as a multiply-shift |
| `CurrentLimiterShader` | `CalcTotalMilliAmpere` + manual clamping | Max total mA budget; scales all pixels proportionally if over budget |

**Example pipeline:**

```cpp
// Build a shaded transform that chains gamma → brightness → limiter
// before serialization. No separate scratch buffer needed in PixelBus.
// All components are static — known at compile time, no heap allocation.
static NeoPixelTransform innerTransform(
    NeoPixelTransformConfig{
        .channelCount = 3,
        .channelOrder = {1, 0, 2},  // GRB
        .bitsPerChannel = 8
    });

static GammaCorrectionShader gamma(2.8f);
static BrightnessShader brightness(0.75f);
static CurrentLimiterShader limiter(2000); // 2A max

static ITransformColors* shaders[] = { &gamma, &brightness, &limiter };
static ShadedTransform transform(innerTransform, shaders);

auto bus = PixelBus(300, transform, emitter);

// During show():
//   1. ITransformColorToBytes::apply(_byteBuffer, _colors)
//      → ShadedTransform processes _colors in batches:
//        a. Copy batch to stack scratch array
//        b. GammaCorrectionShader::apply(batch)
//        c. BrightnessShader::apply(batch)
//        d. CurrentLimiterShader::apply(batch)
//        e. Inner transform serializes batch into _byteBuffer
//   2. IEmitPixels::update(_byteBuffer)
```

### 3.4 ITransformColorToBytes

Replaces both T_COLOR_FEATURE's serialization logic and its settings embedding. A single interface with one method:

```cpp
class ITransformColorToBytes
{
public:
    virtual ~ITransformColorToBytes() = default;

    // Transform logical colors into the raw byte stream ready for emission.
    // `pixels` is pre-sized by the caller (IPixelBus) to hold the complete
    // output including any protocol headers/footers.
    virtual void apply(std::span<uint8_t> pixels,
                       std::span<const Color> colors) = 0;

    // Total bytes needed for `count` pixels (includes any settings/framing overhead).
    virtual size_t bytesNeeded(size_t pixelCount) const = 0;
};
```

**Concrete implementations** replace the current feature explosion with a small set of configurable transforms:

| Implementation | Replaces | Configuration |
|---------------|----------|---------------|
| `NeoPixelTransform` | All `Neo{3,4,5}Byte`, `Neo{3,4,5}Word` features | channel count, channel order (array of indices), bits-per-channel (8 or 16), optional pad bytes, optional in-band settings (see below) |
| `DotStarTransform` | `DotStarX4Byte`, `DotStarL4Byte`, `DotStarX4Word`, `DotStarL4Word` | channel order, use-luminance flag, bits-per-channel |
| `Lpd6803Transform` | `Neo2Byte555Feature` | channel order, 5-bit packing |
| `Lpd8806Transform` | `Neo3Byte777Feature` | channel order, 7-bit with MSB set |
| `P9813Transform` | `P9813BgrFeature` | channel order, checksum prefix |
| `Tlc59711Transform` | `Tlc59711` features | Reversed 16-bit, per-chip brightness/command config (constructor param) |
| `Tlc5947Transform` | `Tlc5947` features | 12-bit packing, reverse order |

The channel-order permutation (the IC index templates in the original) becomes a `constexpr std::array<uint8_t, N>` passed at construction time.

**Settings embedding** (TM1814 current control, SM168x gain, TLC59711 brightness, etc.) is a configuration parameter on the relevant transform, passed at construction time. There is no separate settings inheritance branch or composable wrapper. For example:

```cpp
// TM1814 current limits are a constructor parameter on NeoPixelTransform:
struct Tm1814CurrentSettings
{
    uint16_t redMa, greenMa, blueMa, whiteMa;
};

static NeoPixelTransform transform(
    NeoPixelTransformConfig{
        .channelCount = 4,
        .channelOrder = {3, 0, 1, 2},  // WRGB
        .bitsPerChannel = 8,
        .inBandSettings = Tm1814CurrentSettings{150, 150, 150, 150}
    });
```

`NeoPixelTransform` uses an `std::optional<std::variant<...>>` (or similar) to hold the configured settings type. During `apply()`, it serializes the settings into the appropriate position (front or end of the byte stream) and then serializes the pixel color data. Transforms that never have settings (e.g., `Lpd6803Transform`) simply have no settings parameter.

#### ShadedTransform (Decorator)

`ShadedTransform` is a decorator that wraps any `ITransformColorToBytes` implementation together with a shader pipeline (`ITransformColors` chain). It implements `ITransformColorToBytes` itself, so PixelBus sees a single transform — it does not need to know about shaders at all.

The key benefit is **eliminating the full-size `_shadedColors` scratch buffer** from PixelBus. Instead, `ShadedTransform` processes colors in fixed-size batches using a small stack-allocated scratch array. For each batch, it copies a chunk of input colors, applies the shader pipeline to the chunk, then delegates to the inner transform to serialize that chunk into the corresponding region of the output byte buffer. This amortizes the scratch memory to a small constant regardless of strip length.

```cpp
class ShadedTransform : public ITransformColorToBytes
{
public:
    ShadedTransform(ITransformColorToBytes& inner,
                    std::span<ITransformColors* const> shaders);

    void apply(std::span<uint8_t> pixels,
               std::span<const Color> colors) override
    {
        if (_shaders.empty())
        {
            // No shaders — pass through directly (zero-copy)
            _inner.apply(pixels, colors);
            return;
        }

        // Process in fixed-size batches to keep scratch memory small
        size_t remaining = colors.size();
        size_t srcOffset = 0;
        size_t dstOffset = _inner.headerBytes();  // skip settings header if any

        // Serialize header/footer via inner transform's full apply on first call,
        // then overwrite pixel region in batches. Alternatively, inner transform
        // exposes separate header/pixel/footer methods (implementation detail).

        while (remaining > 0)
        {
            size_t batchSize = std::min(remaining, BatchSize);
            std::array<Color, BatchSize> scratch;
            std::copy_n(colors.begin() + srcOffset, batchSize,
                        scratch.begin());

            std::span<Color> batchSpan(scratch.data(), batchSize);
            for (auto* shader : _shaders)
            {
                shader->apply(batchSpan);
            }

            _inner.applyBatch(pixels, batchSpan, srcOffset);

            srcOffset += batchSize;
            remaining -= batchSize;
        }
    }

    size_t bytesNeeded(size_t pixelCount) const override
    {
        return _inner.bytesNeeded(pixelCount);
    }

private:
    static constexpr size_t BatchSize = 32;  // 32 × 10 bytes = 320 bytes on stack

    ITransformColorToBytes& _inner;
    std::span<ITransformColors* const> _shaders;
};
```

**Design points:**

- **Constant scratch memory.** The batch buffer is `BatchSize × sizeof(Color)` on the stack (320 bytes at `BatchSize = 32`). This replaces a heap allocation of `pixelCount × sizeof(Color)` (3,000 bytes for 300 pixels).
- **Composable.** Because `ShadedTransform` implements `ITransformColorToBytes`, it slots into PixelBus without any special handling. PixelBus holds a reference to `ITransformColorToBytes` — it may or may not be a `ShadedTransform`.
- **Shader-aware inner transform.** The inner transform needs an `applyBatch()` method (or equivalent) that serializes a sub-span of colors starting at a given pixel offset. This is a minor extension to the base interface — basic transforms already iterate linearly and can trivially support offset-based writes.
- **Zero-copy when no shaders.** If the shader list is empty, `apply()` delegates directly to the inner transform with no copying.

**Example construction:**

```cpp
static NeoPixelTransform innerTransform(
    NeoPixelTransformConfig{
        .channelCount = 3,
        .channelOrder = {1, 0, 2},  // GRB
        .bitsPerChannel = 8
    });

static GammaCorrectionShader gamma(2.8f);
static BrightnessShader brightness(0.75f);
static CurrentLimiterShader limiter(2000);

static ITransformColors* shaders[] = { &gamma, &brightness, &limiter };
static ShadedTransform transform(innerTransform, shaders);

auto bus = PixelBus(300, transform, emitter);
```

### 3.5 IClockDataBus

Formalizes the duck-typed `T_TWOWIRE` contract as a proper interface:

```cpp
class IClockDataBus
{
public:
    virtual ~IClockDataBus() = default;

    virtual void begin() = 0;
    virtual void beginTransaction() = 0;
    virtual void endTransaction() = 0;
    virtual void transmitByte(uint8_t data) = 0;
    virtual void transmitBytes(std::span<const uint8_t> data) = 0;
    virtual void transmitBit(uint8_t bit) = 0;  // needed by SM16716, MBI6033
};
```

Implementations:
| Class | Replaces |
|-------|----------|
| `BitBangClockDataBus` | `TwoWireBitBangImple` |
| `SpiClockDataBus` | `TwoWireSpiImple` (configurable clock speed) |
| `HspiClockDataBus` | `TwoWireHspiImple` (ESP32 only) |
| `DebugClockDataBus` | `TwoWireDebugImple` |

SPI clock speed becomes a runtime constructor parameter (a `uint32_t` Hz value) instead of a separate speed template class per frequency.

### 3.6 IEmitPixels

Replaces the T_METHOD concept. Responsible for:
1. Sending a complete byte buffer to the hardware
2. Managing timing/latching
3. Reporting readiness

```cpp
class IEmitPixels
{
public:
    virtual ~IEmitPixels() = default;

    virtual void initialize() = 0;
    virtual void update(std::span<const uint8_t> data) = 0;
    virtual bool isReadyToUpdate() const = 0;
    virtual bool alwaysUpdate() const = 0;
};
```

**Key change from original:** The emitter no longer owns the data buffer. It receives a `span` to emit. This separates buffer ownership (IPixelBus) from transmission (IEmitPixels).

Implementations:

| Class | Wire Type | Platform | Replaces |
|-------|-----------|----------|----------|
| `ClockDataEmitter` | Two-wire | Any | All `*MethodBase<T_TWOWIRE>` — delegates to `IClockDataBus`, owns framing logic |
| `Rp2040PioEmitter` | One-wire | RP2040 | `NeoRp2040x4MethodBase` |
| `Esp32RmtEmitter` | One-wire | ESP32 | `NeoEsp32RmtMethodBase` |
| `Esp32I2sEmitter` | One-wire | ESP32 | `NeoEsp32I2sMethodBase` |
| `ArmBitBangEmitter` | One-wire | ARM | `NeoArmMethodBase` |
| `SerialStreamEmitter` | Serial | Any | `PixieStreamMethod` |

`ClockDataEmitter` is itself parameterized by a protocol descriptor (framing, byte order) and an `IClockDataBus` instance:

```cpp
class ClockDataEmitter : public IEmitPixels
{
public:
    ClockDataEmitter(IClockDataBus& bus,
                     const ClockDataProtocol& protocol);
    // ...
};

struct ClockDataProtocol
{
    std::span<const uint8_t> startFrame;
    std::span<const uint8_t> endFrame;
    size_t endFramePerPixelBits{0};  // e.g., DotStar: 1 bit per 2 pixels
    std::optional<uint8_t> latchPin;
    uint32_t latchDelayUs{0};        // e.g., WS2801: 500µs
};
```

One-wire emitters take timing descriptors at construction:

```cpp
struct OneWireTiming
{
    uint32_t t0hNs, t0lNs, t1hNs, t1lNs;
    uint32_t resetUs;
    bool invert{false};
};
```

Each platform translates this to its native representation internally.

### 3.7 IPixelBus

The top-level user-facing class. **Owns the Color buffer and the byte buffer.** Orchestrates transform → emit.

```cpp
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

    virtual void setPixelColor(size_t offset, std::span<Color> pixelData) = 0;
    virtual void getPixelColor(size_t offset, std::span<Color> pixelData) const = 0;
};
```

### 3.8 Concrete PixelBus

PixelBus is now a thin orchestrator. Shader management is **not** part of PixelBus — it is handled by `ShadedTransform` (see §3.4). PixelBus simply holds a single `ITransformColorToBytes` (which may or may not be a `ShadedTransform`) and delegates to it.

```cpp
class PixelBus : public IPixelBus
{
public:
    PixelBus(size_t pixelCount,
             ITransformColorToBytes& transform,
             IEmitPixels& emitter);

    void begin() override;

    void show() override
    {
        if (!_dirty && !_emitter.alwaysUpdate()) return;

        _transform.apply(_byteBuffer, _colors);
        _emitter.update(_byteBuffer);
        _dirty = false;
    }

    // ... color accessors delegate to _colors vector ...

private:
    std::vector<Color> _colors;            // logical color buffer (user-facing)
    std::vector<uint8_t> _byteBuffer;      // serialized wire data (sized by transform)
    ITransformColorToBytes& _transform;
    IEmitPixels& _emitter;
    bool _dirty{false};
};
```

---

## 4. Buffer Ownership

### 4.1 Per-Class Breakdown

**`PixelBus`** (the concrete `IPixelBus` implementation) owns two buffers:

1. **`std::vector<Color> _colors`** — The logical pixel buffer. This is the user-facing API surface. Users read and write `Color` values here via `setPixelColor()`, `getPixelColor()`, and `colors()`. Sized to `pixelCount` elements at construction. This buffer exists at full 16-bit-per-channel RGBWW precision regardless of what the target LED chip actually supports — the transform handles truncation and channel selection. **Shaders never modify this buffer** (they operate on a stack-allocated scratch copy inside `ShadedTransform`).

2. **`std::vector<uint8_t> _byteBuffer`** — The serialized wire-format buffer. Sized at construction to `transform->bytesNeeded(pixelCount)` bytes, which accounts for per-pixel wire bytes plus any protocol overhead (settings headers/footers, pad bytes). This buffer is written by the byte transform during `show()` and then passed as a `span` to the emitter. PixelBus never interprets the contents — it is opaque serialized data.

**`ITransformColors` implementations** (shader pipeline) own no buffers. They are passed a `span<Color>` (the stack-allocated batch scratch inside `ShadedTransform`) and modify it in-place. They may hold configuration state (gamma curve, brightness level, current budget) but never allocate pixel storage.

**`ShadedTransform`** owns no heap-allocated pixel buffers. It holds the shader pipeline and the inner `ITransformColorToBytes`. During `apply()`, it uses a small stack-allocated `std::array<Color, BatchSize>` (320 bytes at `BatchSize = 32`) as scratch space for shader processing. Colors are copied into this scratch array one batch at a time, the shader pipeline runs on each batch, and the inner transform serializes each batch into the output byte span. The scratch array is a local variable — no persistent allocation.

**`ITransformColorToBytes` implementations** (other than `ShadedTransform`) own no buffers. They are stateless transforms (aside from their immutable configuration: channel order, bits-per-channel, in-band settings values). They read from the input color span and write into the output byte span, both passed into `apply()`. No internal allocation occurs during `apply()`.

**`IClockDataBus` implementations** own no pixel-related buffers. They may hold hardware handles (SPI peripheral, pin numbers) but do not buffer pixel data. Data flows through them byte-by-byte or in bulk via `transmitBytes()` — they are pure transmission interfaces.

**`IEmitPixels` implementations** fall into two categories:

- **Non-buffering emitters** (e.g., `ClockDataEmitter`, `ArmBitBangEmitter`, `SerialStreamEmitter`): Own no pixel data buffers. They iterate over the `span<const uint8_t>` passed to `update()` and transmit it directly (via `IClockDataBus`, GPIO bit-bang, or serial write). The span remains valid for the duration of `update()` because PixelBus holds it.

- **DMA/double-buffering emitters** (e.g., `Rp2040PioEmitter`, `Esp32RmtEmitter`): Own one or two internal hardware buffers (DMA-aligned memory, PIO FIFO staging, etc.). During `update()`, they copy from the passed `span<const uint8_t>` into their internal buffer, then initiate an async DMA transfer. The internal buffer must persist until the DMA transfer completes, which is why the emitter owns it. These emitters manage their own double-buffering internally — PixelBus is unaware of it.

### 4.2 Data Flow

```
User code                  PixelBus                    ShadedTransform (ITransformColorToBytes)
    │                          │                                │
    ├─ setPixelColor() ───────►│ writes to _colors              │
    │                          │                                │
    ├─ show() ────────────────►│                                │
    │                          ├─ apply(_byteBuffer, _colors) ─►│
    │                          │                                ├─ for each batch of colors:
    │                          │                                │   copy batch → stack scratch
    │                          │                                │   shader 1 (e.g. gamma)
    │                          │                                │   shader 2 (e.g. brightness)
    │                          │                                │   shader N (e.g. limiter)
    │                          │                                │   inner transform → _byteBuffer
    │                          │                                │
    │                          │◄── _byteBuffer written ────────┤
    │                          │                                │
    │                          │          IEmitPixels            │
    │                          │              │                  │
    │                          ├─ update(_byteBuffer) ─────────►│
    │                          │              ├─ [non-DMA] iterates span, transmits
    │                          │              └─ [DMA] copies to internal buf, starts DMA
    │                          │
```

### 4.3 Design Rationale

This is a deliberate inversion from the original design where `T_METHOD` owned the single raw `uint8_t*` buffer and `T_COLOR_FEATURE` read/wrote into it via pointer arithmetic.

The new design:

- **Gives the user a typed `Color` buffer** — no raw byte manipulation needed. `getPixelColor()` reads directly from the `Color` vector instead of deserializing from wire-format bytes, which eliminates both the `retrievePixelColor()` method and the precision loss from round-tripping through 8-bit wire encoding.
- **Decouples serialization from transmission** — the transform writes into a span it does not own, and the emitter reads from a span it does not own. Neither needs to coordinate buffer lifecycle with the other.
- **Isolates DMA concerns** — only emitters that actually need DMA-aligned or double-buffered memory pay for it. The PixelBus buffer can live in normal heap memory; the emitter copies into special memory internally.
- **Makes buffer sizes explicit** — `_colors` is always `pixelCount * sizeof(Color)` (10 bytes per pixel at 16-bit RGBWW). `_byteBuffer` is always `transform->bytesNeeded(pixelCount)`. There is no combined "pixels + settings" buffer with implicit offsets.

### 4.4 Memory Cost

For a 300-pixel WS2812 strip (3 bytes/pixel on wire, 8-bit):

| Buffer | Size | Owner |
|--------|------|-------|
| `_colors` | 300 × 10 = 3,000 bytes | PixelBus |
| `_byteBuffer` | 300 × 3 = 900 bytes | PixelBus |
| Shader scratch (stack, if `ShadedTransform`) | 32 × 10 = 320 bytes | ShadedTransform (stack) |
| DMA buffer (if applicable) | 900 bytes | Emitter |
| **Total (no shaders)** | **3,900 – 4,800 bytes** | |
| **Total (with shaders)** | **3,900 – 4,800 bytes + 320 stack** | |

Compare to the original: 300 × 3 = 900 bytes (single buffer, method-owned). The new design uses ~4× more memory due to the full-precision `Color` vector. This is acceptable on RP2040 (264 KB SRAM) but would be a concern on AVR (2 KB SRAM) — which is not a target for this rewrite.

Compare to the previous architecture (before `ShadedTransform`): shaders required a full `_shadedColors` heap allocation of 3,000 bytes. `ShadedTransform` replaces this with 320 bytes of stack, saving 2,680 bytes for a 300-pixel strip. The savings scale linearly with strip length.

---

## 5. Complete Method Catalog with Settings

### 5.1 One-Wire Methods (Timing-Based)

All one-wire methods share the same constructor pattern and `NeoNoSettings`. They differ only by timing constants and platform.

| LED Chip / Speed | T0H (ns) | T1H (ns) | Reset (µs) | Notes |
|-----------------|----------|----------|------------|-------|
| WS2811 | 500 | 1200 | 50 | 400 kHz data rate |
| WS2812 / WS2812B | 400 | 800 | 300 | Most common NeoPixel |
| WS2816 | 300 | 790 | 300 | 16-bit variant |
| WS2805 | 300 | 790 | 300 | Same timing as WS2816 |
| SK6812 | 400 | 800 | 80 | RGBW variant common |
| TM1814 | 360 | 720 | 200 | RGBW + current settings |
| TM1829 | 300 | 800 | 500 | |
| TM1914 | 360 | 720 | 200 | RGB + mode settings |
| APA106 | 350 | 1360 | 50 | |
| TX1812 | 300 | 600 | 80 | |
| GS1903 | 300 | 900 | 40 | |
| 800 Kbps Generic | 400 | 800 | 50 | |
| 400 Kbps Generic | 500 | 1200 | 50 | |

Inversion: All one-wire timings have an `inverted` variant (active-low signal). This is a boolean flag, not a separate method class.

### 5.2 Two-Wire Methods (Clock + Data)

| Protocol | Framing | Transport Options | Special Pins |
|----------|---------|-------------------|-------------|
| **DotStar (APA102)** | Start: 4×0x00, End: reset(4×0x00) + ceil(N/16) × 0x00 | BitBang, SPI, HSPI, ESP32 DMA SPI | — |
| **WS2801** | None (500µs latch) | BitBang, SPI, HSPI | — |
| **LPD8806** | Start: ceil(N/32)×0x00, End: same count ×0xFF | BitBang, SPI | — |
| **LPD6803** | Start: 4×0x00, End: ceil(N/8)×0x00 | BitBang, SPI | — |
| **P9813** | Start: 4×0x00, End: 4×0x00 | BitBang, SPI | — |
| **HD108** | Start: 16×0x00, End: 4×0xFF | BitBang, SPI, HSPI | — |
| **TLC59711** | Per-chip inline header (4 bytes), reversed 16-bit data, 20µs latch | BitBang, SPI, HSPI | — |
| **TLC5947** | Data reversed per module, 12-bit packing | BitBang, SPI | Latch pin, optional OE pin |
| **SM16716** | 48-bit start frame + per-pixel HIGH bit, bit-level framing | BitBang only | — |
| **MBI6033** | Reset protocol (21µs toggle), 6-byte header per chip | BitBang only | — |

### 5.3 Serial Methods

| Protocol | Transport | AlwaysUpdate | Notes |
|----------|-----------|-------------|-------|
| **Pixie** | `Stream*` (UART) | Yes (must send every <2s) | No framing, 1ms silence = latch |

---

## 6. Feature-Method Compatibility Matrix (Semantic Pairing)

This matrix maps which transform implementations work with which emitters. In the rewrite, this pairing is enforced by construction — you pick a `ITransformColorToBytes` and an `IEmitPixels` that agree on protocol.

| Transform | Emitter(s) | Features Consumed |
|-----------|-----------|-------------------|
| `NeoPixelTransform` (3/4/5/6-byte, 3/4/5-word) | Any one-wire emitter | All NeoPixel-family LEDs (WS28xx, SK6812, TM18xx, SM168x, etc.) |
| `DotStarTransform` | `ClockDataEmitter` with DotStar protocol | APA102, HD108 |
| `Lpd8806Transform` | `ClockDataEmitter` with LPD8806 protocol | LPD8806 |
| `Lpd6803Transform` | `ClockDataEmitter` with LPD6803 protocol | LPD6803 |
| `P9813Transform` | `ClockDataEmitter` with P9813 protocol | P9813 |
| `Ws2801Transform` | `ClockDataEmitter` with WS2801 protocol | WS2801 |
| `Tlc59711Transform` | `ClockDataEmitter` with TLC59711 protocol | TLC59711 |
| `Tlc5947Transform` | `ClockDataEmitter` with TLC5947 protocol | TLC5947 |

---

## 7. What Changes from the Original

| Aspect | Original | Rewrite |
|--------|----------|---------|
| Color types | 7 distinct classes (RgbColor, RgbwColor, ..., Rgbww80Color) | 1 unified `Color` (16-bit RGBWW) |
| Channel ordering | Template params `V_IC_1..N` on ~15 base feature classes | `constexpr std::array<uint8_t, N>` on a single configurable transform |
| Feature hierarchy | 38 feature files, deep dual-inheritance | ~8 transform implementations, flat |
| Color depth | Separate byte/word hierarchies | Single 16-bit-native `Color`; transform truncates to 8-bit where needed |
| Buffer ownership | T_METHOD owns raw `uint8_t*` buffer | PixelBus owns both `Color` vector and byte buffer |
| Read-back | `retrievePixelColor()` deserializes from byte buffer | Not needed — read directly from `Color` vector |
| Settings | Mixed into feature inheritance (separate settings base class) | Constructor parameter on the transform — no settings inheritance branch |
| SPI speed | One template class per frequency (9 classes) | Runtime `uint32_t` parameter |
| Wire interface | Duck-typed `T_TWOWIRE` template param | `IClockDataBus` virtual interface |
| Method interface | Duck-typed `T_METHOD` template param | `IEmitPixels` virtual interface |
| Platform dispatch | `#ifdef` at include level selects method files | Same (unavoidable for hardware peripherals), but behind `IEmitPixels` |
| Double buffering | Method-internal, exposed via `SwapBuffers()` | Emitter-internal, transparent to PixelBus |
| Gamma / luminance | `NeoPixelBusLg` subclass with `T_GAMMA` template, destructive per-pixel `Apply()` | `ITransformColors` shader pipeline via `ShadedTransform` decorator — non-destructive, chainable, stack-based batch scratch (no heap allocation) |
| 6-byte/word features | Supported (pad bytes, 6-channel) | Dropped |
| `retrievePixelColor_P` (PROGMEM) | Supported | Dropped (not relevant to RP2040) |
