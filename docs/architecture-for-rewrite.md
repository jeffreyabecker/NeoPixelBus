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

## 3. Rewrite Architecture (Current)

### 3.1 Top-Level Types

```
Color              — unified 8-bit RGBWW color
IShader            — per-pixel Color→Color shader with begin()/end() lifecycle
ShaderChain        — chains multiple IShader instances into one
ITransformColorToBytes — internal to emitters: span<const Color> → span<uint8_t>
IClockDataBus      — SPI-like byte/bit transmission interface
IEmitPixels        — accepts span<const Color>, owns transform + byte buffer + optional shader
IPixelBus          — top-level bus interface: owns Color buffer
PixelBus           — concrete IPixelBus: owns Color buffer + unique_ptr<IEmitPixels>
```

### 3.2 Color

Single class. Internally stores 5 × `uint8_t` channels: R, G, B, WW (warm white), CW (cool white). 8-bit matches actual LED chip capability. Unused channels default to 0.

```cpp
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
```

### 3.3 IShader (Per-Pixel Shader Pipeline)

Replaces `NeoPixelBusLg` and its `T_GAMMA`-templated `LuminanceShader`. Shaders are per-pixel transforms with a lifecycle: `begin()` is called once before the pixel loop, `apply()` is called for each pixel, and `end()` is called after all pixels are processed.

```cpp
class IShader
{
public:
    virtual ~IShader() = default;
    virtual void begin() {}
    virtual const Color apply(uint16_t index, const Color color) = 0;
    virtual void end() {}
};
```

**Key design points:**

- **Per-pixel.** Each `apply()` call processes one pixel and returns the transformed color. The emitter calls this in its inner loop.
- **Non-destructive.** Shaders receive and return `Color` by value — the user's color buffer is never modified.
- **Lifecycle hooks.** `begin()`/`end()` allow stateful shaders to initialize and finalize per-frame (e.g., accumulating totals for current limiting).
- **Owned by emitters.** Each emitter holds a `std::unique_ptr<IShader>` (nullable). If null, colors pass through unshaded.

**Concrete implementations:**

| Class | Template | Description |
|-------|----------|-------------|
| `GammaShader<T_GAMMA>` | Yes | Applies a compile-time gamma method per-channel. `T_GAMMA` is a duck-typed struct with `static uint8_t correct(uint8_t)`. |
| `NilShader` | No | Identity pass-through (returns color unchanged). |
| `ShaderChain` | No | Chains multiple `IShader*` instances. Delegates `begin()`/`apply()`/`end()` to each shader in order. |

**Gamma methods** (duck-typed, compile-time, zero vtable overhead in hot loop):

| Class | Approach |
|-------|----------|
| `GammaNullMethod` | Identity pass-through |
| `GammaEquationMethod` | `pow(x, 1/0.45)` ≈ γ 2.222 |
| `GammaCieLabMethod` | CIE L* piecewise curve |
| `GammaTableMethod` | Static 256-entry LUT (flash) |
| `GammaDynamicTableMethod` | Runtime-filled 256 LUT |
| `GammaInvertMethod<T>` | Decorator: `~T::correct(v)` |

### 3.4 ITransformColorToBytes (Emitter-Internal)

This is an **implementation detail of emitters**, not a top-level architectural concept. It serializes `Color` values into the wire-format byte stream. Emitters own a concrete transform instance and a byte buffer internally.

```cpp
class ITransformColorToBytes
{
public:
    virtual ~ITransformColorToBytes() = default;

    virtual void apply(std::span<uint8_t> pixels,
                       std::span<const Color> colors) = 0;

    virtual size_t bytesNeeded(size_t pixelCount) const = 0;
};
```

**Concrete implementations** (all live in `emitters/`):

| Implementation | Description |
|---------------|-------------|
| `ColorOrderTransform` | Generic N-channel byte reordering (3/4/5 channels). Configurable via `ColorOrderTransformConfig{channelCount, channelOrder}`. |
| `DotStarTransform` | APA102/HD108: 0xFF or 0xE0\|lum prefix + 3 reordered channels (4 bytes/pixel). |

Future transforms (not yet implemented): `Lpd8806Transform`, `Lpd6803Transform`, `P9813Transform`, etc.

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
    virtual void transmitBit(uint8_t bit) = 0;
};
```

Implementations:

| Class | Replaces |
|-------|----------|
| `SpiClockDataBus` | `TwoWireSpiImple` |
| `DebugClockDataBus` | `TwoWireDebugImple` (wraps optional inner bus) |

### 3.6 IEmitPixels

Replaces the T_METHOD concept. **Accepts `span<const Color>`, owns the byte buffer and transform internally.** Optionally applies an `IShader` per-pixel before serialization.

```cpp
class IEmitPixels
{
public:
    virtual ~IEmitPixels() = default;

    virtual void initialize() = 0;
    virtual void update(std::span<const Color> colors) = 0;
    virtual bool isReadyToUpdate() const = 0;
    virtual bool alwaysUpdate() const = 0;
};
```

**Key changes from original T_METHOD:**
- Takes `span<const Color>` instead of `span<const uint8_t>`.
- Owns the byte buffer and transform — PixelBus does not deal with bytes.
- Owns an optional `std::unique_ptr<IShader>` for per-pixel color processing.

**Emitter `update()` pattern:**
```cpp
void update(std::span<const Color> colors) override
{
    _byteBuffer.resize(_transform.bytesNeeded(colors.size()));

    if (_shader)
    {
        _shader->begin();
        size_t offset = 0;
        const size_t bpp = _transform.bytesNeeded(1);
        for (uint16_t i = 0; i < colors.size(); ++i)
        {
            Color shaded = _shader->apply(i, colors[i]);
            _transform.apply(
                std::span<uint8_t>(_byteBuffer).subspan(offset, bpp),
                std::span<const Color>(&shaded, 1));
            offset += bpp;
        }
        _shader->end();
    }
    else
    {
        _transform.apply(_byteBuffer, colors);
    }

    // ... transmit _byteBuffer to hardware ...
}
```

**Current implementations:**

| Class | Description | Constructor |
|-------|-------------|-------------|
| `PrintEmitter` | Debug hex-dump to `Print&` | `(Print&, unique_ptr<IShader>, ColorOrderTransformConfig)` |
| `ClockDataEmitter` | Two-wire protocol framing via `IClockDataBus` | `(IClockDataBus&, ClockDataProtocol&, ITransformColorToBytes&, unique_ptr<IShader>, pixelCount)` |

### 3.7 IPixelBus / PixelBus

PixelBus is a thin orchestrator. It **owns only the Color buffer** and a `unique_ptr<IEmitPixels>`. No byte buffer, no transform reference.

```cpp
class PixelBus : public IPixelBus
{
public:
    PixelBus(size_t pixelCount,
             std::unique_ptr<IEmitPixels> emitter);

    void show() override
    {
        if (!_dirty && !_emitter->alwaysUpdate()) return;
        _emitter->update(_colors);
        _dirty = false;
    }

private:
    std::vector<Color>           _colors;
    std::unique_ptr<IEmitPixels> _emitter;
    bool                         _dirty{false};
};
```

### 3.8 Example Construction

```cpp
// DotStar strip with gamma correction
static DotStarTransform dotStarTransform(
    DotStarTransformConfig{
        .channelOrder = {2, 1, 0},  // BGR
        .mode = DotStarMode::FixedBrightness,
    });

static DebugClockDataBus debugBus(Serial);

auto emitter = std::make_unique<ClockDataEmitter>(
    debugBus, protocol::DotStar, dotStarTransform,
    std::make_unique<GammaShader<GammaCieLabMethod>>(),
    PixelCount);

PixelBus bus(PixelCount, std::move(emitter));
```

```cpp
// Simple NeoPixel (WS2812 GRB) debug output, no shader
auto emitter = std::make_unique<PrintEmitter>(
    Serial, nullptr,
    ColorOrderTransformConfig{
        .channelCount = 3,
        .channelOrder = {1, 0, 2, 0, 0}  // GRB
    });

PixelBus bus(PixelCount, std::move(emitter));
```

---

## 4. Buffer Ownership

### 4.1 Per-Class Breakdown

**`PixelBus`** owns a single buffer:

1. **`std::vector<Color> _colors`** — The logical pixel buffer. This is the user-facing API surface. Users read and write `Color` values here via `setPixelColor()`, `getPixelColor()`, and `colors()`. Sized to `pixelCount` elements at construction. This buffer exists at full 8-bit-per-channel RGBWW precision. **PixelBus does not own a byte buffer** — serialization is entirely the emitter's responsibility.

**`IShader` implementations** own no pixel buffers. They receive a `Color` by value and return a transformed `Color`. They may hold configuration state (gamma curve, brightness level, current budget) but never allocate pixel storage. `ShaderChain` holds a `std::vector<IShader*>` of pointers to shader instances but owns no pixel data.

**`IEmitPixels` implementations** own the byte buffer and transform:

- **`ClockDataEmitter`** owns a `std::vector<uint8_t> _byteBuffer` sized to `transform.bytesNeeded(pixelCount)`. During `update()`, it applies the optional shader per-pixel, serializes through its `ITransformColorToBytes&`, and transmits via `IClockDataBus`.
- **`PrintEmitter`** owns an internal `ColorOrderTransform` and serializes per-pixel on the fly to `Print&`. No persistent byte buffer.
- **DMA/double-buffering emitters** (future: `Rp2040PioEmitter`, `Esp32RmtEmitter`): Own DMA-aligned internal buffers. They copy serialized data and initiate async transfers.

**`ITransformColorToBytes` implementations** own no buffers. They are stateless transforms (aside from their immutable configuration). They read from input color data and write into the output byte span, both passed into `apply()`.

**`IClockDataBus` implementations** own no pixel-related buffers. They may hold hardware handles (SPI peripheral, pin numbers) but do not buffer pixel data.

### 4.2 Data Flow

```
User code                  PixelBus                    Emitter (IEmitPixels)
    │                          │                                │
    ├─ setPixelColor() ───────►│ writes to _colors              │
    │                          │                                │
    ├─ show() ────────────────►│                                │
    │                          ├─ update(_colors) ─────────────►│
    │                          │                                ├─ if shader:
    │                          │                                │   shader.begin()
    │                          │                                │   for each pixel:
    │                          │                                │     color = shader.apply(i, color)
    │                          │                                │     transform → _byteBuffer
    │                          │                                │   shader.end()
    │                          │                                ├─ else:
    │                          │                                │   transform.apply(_byteBuffer, colors)
    │                          │                                │
    │                          │                                ├─ transmit _byteBuffer
    │                          │                                │   [non-DMA] iterates, transmits via bus
    │                          │                                │   [DMA] copies to DMA buf, starts transfer
    │                          │
```

### 4.3 Design Rationale

This is a deliberate inversion from the original design where `T_METHOD` owned the single raw `uint8_t*` buffer and `T_COLOR_FEATURE` read/wrote into it via pointer arithmetic.

The new design:

- **Gives the user a typed `Color` buffer** — no raw byte manipulation needed. `getPixelColor()` reads directly from the `Color` vector instead of deserializing from wire-format bytes, eliminating both the `retrievePixelColor()` method and precision loss from round-tripping through wire encoding.
- **Encapsulates serialization in emitters** — the emitter owns its byte buffer and transform. PixelBus never touches bytes. This means different emitters can manage their serialization strategy independently (batch transform, per-pixel with shader, DMA staging, etc.).
- **Isolates DMA concerns** — only emitters that actually need DMA-aligned or double-buffered memory pay for it. The PixelBus Color buffer lives in normal heap memory; the emitter manages special memory internally.
- **Makes buffer sizes explicit** — `_colors` is always `pixelCount × sizeof(Color)` (5 bytes per pixel at 8-bit RGBWW). The emitter's `_byteBuffer` is sized by its own transform.

### 4.4 Memory Cost

For a 300-pixel WS2812 strip (3 bytes/pixel on wire, 8-bit):

| Buffer | Size | Owner |
|--------|------|-------|
| `_colors` | 300 × 5 = 1,500 bytes | PixelBus |
| `_byteBuffer` | 300 × 3 = 900 bytes | Emitter |
| DMA buffer (if applicable) | 900 bytes | Emitter |
| **Total (no DMA)** | **2,400 bytes** | |
| **Total (with DMA)** | **3,300 bytes** | |

Compare to the original: 300 × 3 = 900 bytes (single buffer, method-owned). The new design uses ~2.7× more memory due to the `Color` vector. This is acceptable on RP2040 (264 KB SRAM) but would be a concern on AVR (2 KB SRAM) — which is not a target for this rewrite.

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

This matrix maps which transform implementations work with which emitters. In the rewrite, transforms are **internal to emitters** — you configure the emitter with the appropriate transform at construction time. Shaders are orthogonal: any `IShader` can be used with any emitter.

| Transform (emitter-internal) | Emitter(s) | Features Consumed |
|------------------------------|-----------|-------------------|
| `ColorOrderTransform` (3/4/5-byte) | Any one-wire emitter, `PrintEmitter` | All NeoPixel-family LEDs (WS28xx, SK6812, TM18xx, SM168x, etc.) |
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
| Color types | 7 distinct classes (RgbColor, RgbwColor, ..., Rgbww80Color) | 1 unified `Color` (8-bit RGBWW, 5 channels) |
| Channel ordering | Template params `V_IC_1..N` on ~15 base feature classes | `constexpr std::array<uint8_t, N>` on configurable transform (internal to emitters) |
| Feature hierarchy | 38 feature files, deep dual-inheritance | ~8 transform implementations, flat, inside `emitters/` |
| Color depth | Separate byte/word hierarchies | Single 8-bit-native `Color`; matches LED chip precision |
| Buffer ownership | T_METHOD owns raw `uint8_t*` buffer; PixelBus owns both Color vector and byte buffer in previous rewrite drafts | PixelBus owns only `Color` vector; emitters own byte buffer and transform |
| Read-back | `retrievePixelColor()` deserializes from byte buffer | Not needed — read directly from `Color` vector |
| Settings | Mixed into feature inheritance (separate settings base class) | Constructor parameter on the transform (emitter-internal) |
| SPI speed | One template class per frequency (9 classes) | Runtime `uint32_t` parameter |
| Wire interface | Duck-typed `T_TWOWIRE` template param | `IClockDataBus` virtual interface |
| Method interface | Duck-typed `T_METHOD` template param | `IEmitPixels` virtual interface; takes `span<const Color>` |
| Platform dispatch | `#ifdef` at include level selects method files | Same (unavoidable for hardware peripherals), but behind `IEmitPixels` |
| Double buffering | Method-internal, exposed via `SwapBuffers()` | Emitter-internal, transparent to PixelBus |
| Gamma / luminance | `NeoPixelBusLg` subclass with `T_GAMMA` template, destructive per-pixel `Apply()` | `IShader` per-pixel pipeline owned by emitters — non-destructive, chainable via `ShaderChain` |
| Shader ownership | N/A (gamma baked into bus subclass) | Emitters own `unique_ptr<IShader>` (nullable) |
| Transform ownership | N/A (feature baked into bus template) | Emitters own `ITransformColorToBytes` (internal implementation detail) |
| 6-byte/word features | Supported (pad bytes, 6-channel) | Dropped |
| `retrievePixelColor_P` (PROGMEM) | Supported | Dropped (not relevant to RP2040) |
