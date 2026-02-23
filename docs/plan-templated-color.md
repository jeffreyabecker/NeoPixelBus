# Plan: Templated Color with Variable Channel Count and Bit Depth

## 1. Problem Statement

The current `Color` class is fixed at 5 × `uint8_t` channels (R, G, B, WW, CW).
This creates two inefficiencies:

1. **Memory waste.** A WS2812 RGB strip pays for 5 bytes per pixel but only uses
   3. For 300 pixels: 1500 bytes stored, 900 used — 40% waste. On RP2040 this is
   tolerable, but it adds up on dense installations (e.g., 2000+ pixels).

2. **No 16-bit support.** LED chips like WS2816, HD108, and TLC59711 accept
   16-bit-per-channel data. The current 8-bit `Color` permanently clamps their
   precision. Users of these chips lose half their dynamic range.

Additionally, every emitter that does manual serialization (DotStar, HD108,
LPD6803, LPD8806, P9813, SM16716, TLC5947, TLC59711, WS2801) hardcodes
3-channel RGB assumptions. This must be unwound regardless.

## 2. Design: `Color<N, T>`

```cpp
template <size_t NChannels, typename TComponent = uint8_t>
class Color
{
public:
    std::array<TComponent, NChannels> Channels{};
    static constexpr size_t ChannelCount = NChannels;
    using ComponentType = TComponent;

    // Channel access uses character literals (`'R'`, `'G'`, `'B'`, `'W'`, `'C'`)
    // rather than named numeric index constants.

    constexpr Color() = default;

    template <typename T, typename... Args>
    struct AllConvertible;

    template <typename T>
    struct AllConvertible<T>
    {
        static const bool value = true;
    };

    template <typename T, typename Head, typename... Tail>
    struct AllConvertible<T, Head, Tail...>
    {
        static const bool value = std::is_convertible<Head, T>::value
                               && AllConvertible<T, Tail...>::value;
    };

    // Variadic constructor accepts exactly NChannels components
    template <typename... Args>
    constexpr Color(Args... args,
                    typename std::enable_if<
                        sizeof...(Args) == NChannels
                        && AllConvertible<TComponent, Args...>::value,
                        int
                    >::type = 0)
        : Channels{static_cast<TComponent>(args)...}
    {
    }

    constexpr TComponent  operator[](size_t idx) const { return Channels[idx]; }
    TComponent&           operator[](size_t idx)       { return Channels[idx]; }

    bool operator==(const Color& other) const
    {
        for (size_t idx = 0; idx < NChannels; ++idx)
        {
            if (Channels[idx] != other.Channels[idx])
            {
                return false;
            }
        }
        return true;
    }
};
```

### 2.1 Standard Type Aliases

```cpp
using RgbColor    = Color<3, uint8_t>;
using RgbwColor   = Color<4, uint8_t>;
using RgbcwColor  = Color<5, uint8_t>;   // R, G, B, WW, CW

using Rgb16Color  = Color<3, uint16_t>;
using Rgbw16Color = Color<4, uint16_t>;
using Rgbcw16Color = Color<5, uint16_t>;   // R, G, B, WW, CW
```

These aliases are the user-facing vocabulary. Code that previously used the
bare `Color` class migrates to one of these aliases. `RgbColor` is the most
common — it serves the WS2812/SK6812 RGB family.

### 2.2 `MaxComponentValue` Trait

Blending, dimming, and gamma correction all need the maximum value of a
channel component. This is a `constexpr` computed from `TComponent`:

```cpp
template <size_t N, typename T>
class Color
{
public:
    static constexpr T MaxComponent = std::numeric_limits<T>::max();
    // uint8_t  → 255
    // uint16_t → 65535
    // ...
};
```

## 3. Propagation Through the Architecture

The Color template parameter propagates through the entire type stack.
Every layer that stores, passes, or transforms Color must be parameterized
on the same Color type. This is enforced at compile time — there is no
runtime color conversion between mismatched types.

### 3.1 Type Propagation Map

```
                    Color<N,T>
                        │
          ┌─────────────┼─────────────────┐
          ▼             ▼                 ▼
    ColorIterator<C>  IShader<C>    IProtocol<C>
          │             │                 │
          ▼             ▼                 ▼
    IPixelBus<C>   GammaShader<C>   PrintProtocol<C>
          │        CurrentLimiter<C> DotStarProtocol<C>
          │        ShaderChain<C>   RpPioOneWire<C>
          │                         ...
    ┌─────┼──────┐
    ▼     ▼      ▼
PixelBus<C>  ConcatBus<C>  MosaicBus<C>
```

All children of a `ConcatBus<C>` or `MosaicBus<C>` must be `IPixelBus<C>` — 
same Color type. **Divergent color depths within one aggregate bus are not
supported.** This is a deliberate constraint: mixing RGB and RGBW strips in
one aggregated bus is not a meaningful hardware scenario.

### 3.2 Changes Per File

| File | Current | Templated |
|------|---------|-----------|
| `colors/Color.h` | `class Color` (fixed 5×u8) | `template<size_t N, typename T> class Color` + aliases |
| `colors/ColorIterator.h` | `ColorIterator` hardcoded to `Color` | `template<typename TColor> class ColorIterator` |
| `shaders/IShader.h` | `IShader` with `span<Color>` | `template<typename TColor> class IShader` |
| `shaders/GammaShader.h` | `GammaShader<T_GAMMA>` loops `Color::ChannelCount` | `template<typename TColor, typename T_GAMMA>` — loop becomes `TColor::ChannelCount`; calls `T_GAMMA::correct()` which dispatches by `ComponentType` (see §4.2) |
| `shaders/CurrentLimiterShader.h` | `std::array<uint16_t, Color::ChannelCount>` | `template<typename TColor>` — array sized to `TColor::ChannelCount`; accumulator widened to `uint64_t` for 16-bit (see §4.3) |
| All `Gamma*Method.h` files | `uint8_t correct(uint8_t)` only | Add `uint16_t correct(uint16_t)` overload — different algorithm per method (see §4.2) |
| `shaders/ShadedTransform.h` | `ShaderChain` with `span<Color>` | `template<typename TColor> class ShaderChain` |
| `emitters/IProtocol.h` | `update(span<const Color>)` | `template<typename TColor>` — `update(span<const TColor>)` |
| `emitters/ColorOrderTransform.h` | `channelOrder` as fixed index array | `template<typename TColor>` — string channel order (`const char*`, e.g. `ChannelOrder::GRB`) with explicit `channelCount` |
| `emitters/PrintProtocol.h` | `PrintProtocol` | `template<typename TColor> class PrintProtocol` |
| `emitters/DotStarProtocol.h` | hardcoded 3-ch | `template<typename TColor> class DotStarProtocol` — accepts any ≥3-ch 8-bit Color |
| `emitters/Hd108Protocol.h` | hardcoded 3-ch, 16-bit wire | `class Hd108Protocol : IProtocol<Rgb16Color>` — fixed 3×u16 |
| `emitters/Lpd6803Protocol.h` | hardcoded 3-ch, 5-5-5 | `class Lpd6803Protocol : IProtocol<RgbColor>` — fixed 3×u8; 5-bit narrowing in serialization |
| `emitters/Lpd8806Protocol.h` | hardcoded 3-ch, 7-bit wire | `class Lpd8806Protocol : IProtocol<RgbColor>` — fixed 3×u8; 7-bit narrowing in serialization |
| `emitters/P9813Protocol.h` | hardcoded BGR | `class P9813Protocol : IProtocol<RgbColor>` — fixed 3×u8 BGR |
| `emitters/Sm16716Protocol.h` | hardcoded 3-ch | `class Sm16716Protocol : IProtocol<RgbColor>` — fixed 3×u8 |
| `emitters/Tlc5947Protocol.h` | hardcoded 3-ch, 12-bit wire | `class Tlc5947Protocol : IProtocol<Rgb16Color>` — fixed 3×u16; 12-bit narrowing in serialization |
| `emitters/Tlc59711Protocol.h` | hardcoded 3-ch, 16-bit wire | `class Tlc59711Protocol : IProtocol<Rgb16Color>` — fixed 3×u16 |
| `emitters/Ws2801Protocol.h` | hardcoded 3-ch | `class Ws2801Protocol : IProtocol<RgbColor>` — fixed 3×u8 |
| All OneWire emitters | use `ColorOrderTransform` + `span<const Color>` | `template<typename TColor>` — use `ColorOrderTransform<TColor>` |
| `IPixelBus.h` | `class IPixelBus` | `template<typename TColor> class IPixelBus` |
| `PixelBus.h` | `class PixelBus` — `vector<Color>` | `template<typename TColor> class PixelBus` — `vector<TColor>` |
| `ConcatBus.h` | `class ConcatBus` | `template<typename TColor> class ConcatBus` |
| `MosaicBus.h` | `class MosaicBus` | `template<typename TColor> class MosaicBus` |
| `SegmentBus.h` | `class SegmentBus` | `template<typename TColor> class SegmentBus` |

### 3.3 Files That Don't Change

| File | Why unchanged |
|------|---------------|
| `topologies/PanelLayout.h` | Pure coordinate math, no Color reference |
| `topologies/PanelTopology.h` | Pure coordinate math, no Color reference |
| `topologies/TiledTopology.h` | Pure coordinate math, no Color reference |
| `buses/IClockDataTransport.h` | Byte-level transport, no Color reference |
| `buses/SpiClockDataTransport.h` | Byte-level transport, no Color reference |
| `buses/DebugClockDataTransport.h` | Byte-level transport, no Color reference |
| `emitters/OneWireTiming.h` | Timing constants only |
| `emitters/RpPioDmaState.h` | DMA hardware state only |
| `emitters/RpPioMonoProgram.h` | PIO program only |

### 3.4 Transport Agnosticism Confirmation (Audit)

Transport agnosticism is already true in the current virtual architecture:

- `IClockDataTransport` and `ISelfClockingTransport` operate on
    `std::span<const uint8_t>` payloads only.
- Concrete transport implementations serialize timing/signaling, not Color fields.
- No transport currently depends on channel count or channel component type.

Therefore, the Color templating migration does **not** require transport API
changes. Templating work starts at protocol and shader boundaries
(`IProtocol`, `IShader`, and callers).

## 4. Design Details

### 4.1 Emitter Color Type Strategy

Emitters fall into two categories based on how they bind to a Color type:

**Fixed-Color emitters (two-wire/SPI chips):** Each hardcodes its Color type
in the class declaration. The wire protocol dictates exactly which Color is
accepted — there is no template parameter, no `static_assert`, and no
ambiguity. The `PixelBus` that owns the emitter must use the same Color type.

```cpp
// Fixed: HD108 is always 3×u16
class Hd108Protocol : public IProtocol<Rgb16Color> { /* ... */ };

// Fixed: WS2801 is always 3×u8
class Ws2801Protocol : public IProtocol<RgbColor> { /* ... */ };

// Fixed: TLC59711 is always 3×u16
class Tlc59711Protocol : public IProtocol<Rgb16Color> { /* ... */ };

// Fixed: TLC5947 is always 3×u16 (12-bit narrowing in serialization)
class Tlc5947Protocol : public IProtocol<Rgb16Color> { /* ... */ };

// Fixed: LPD6803 is always 3×u8 (5-bit narrowing in serialization)
class Lpd6803Protocol : public IProtocol<RgbColor> { /* ... */ };

// Fixed: LPD8806 is always 3×u8 (7-bit narrowing in serialization)
class Lpd8806Protocol : public IProtocol<RgbColor> { /* ... */ };

// Fixed: P9813 is always 3×u8 BGR
class P9813Protocol : public IProtocol<RgbColor> { /* ... */ };

// Fixed: SM16716 is always 3×u8
class Sm16716Protocol : public IProtocol<RgbColor> { /* ... */ };
```

**Templated emitters (one-wire + DotStar + Print):** These accept any Color
type because their wire format is configured at construction time via
`channelOrder` (`const char*`, e.g. `ChannelOrder::GRB`). Channel count is
derived from the order string and must be `<= TColor::ChannelCount`.

```cpp
// Templated: DotStar accepts any ≥3-channel 8-bit Color
template <typename TColor>
class DotStarProtocol : public IProtocol<TColor>
{
    static_assert(TColor::ChannelCount >= 3,
        "DotStar requires at least 3 color channels");
    static_assert(std::is_same<typename TColor::ComponentType, uint8_t>::value,
        "DotStar only supports 8-bit color components");
    // ...
};

// Templated: Ws2812xProtocol accepts any Color via ColorOrderTransform
template <typename TColor>
class Ws2812xProtocol : public IProtocol<TColor> { /* ... */ };

// Templated: PrintProtocol accepts any Color
template <typename TColor>
class PrintProtocol : public IProtocol<TColor> { /* ... */ };
```

### 4.2 Gamma Methods: Component-Width Adaptation

The current gamma methods are all `uint8_t → uint8_t`. For 16-bit Color
support, each method gains a `uint16_t correct(uint16_t)` overload with a
**different implementation** appropriate to its correction strategy:

| Method | 8-bit Implementation | 16-bit Implementation |
|--------|---------------------|----------------------|
| `GammaNullMethod` | Identity pass-through | Identity pass-through |
| `GammaEquationMethod` | `pow(x/255, γ) * 255` | `pow(x/65535, γ) * 65535` — same formula, different scale |
| `GammaCieLabMethod` | Piecewise CIE L* curve over 0–255 | Piecewise CIE L* curve over 0–65535 — same breakpoints, wider range |
| `GammaTableMethod` | 256-entry `constexpr` LUT (flash) | **Equation-based fallback** — a 65536-entry LUT is impractical (128 KB); use `GammaCieLabMethod` math at 16-bit |
| `GammaDynamicTableMethod` | Runtime-filled 256-entry LUT | Runtime-filled 256-entry LUT + **interpolation** for 16-bit — look up adjacent 8-bit entries, linearly interpolate for the fractional part |
| `GammaInvertMethod<T>` | `MaxComponent - T::correct(v)` | Same pattern, `uint16_t` overload on inner method |

Each method provides both overloads as explicit static functions:

```cpp
struct GammaCieLabMethod
{
    static constexpr uint8_t correct(uint8_t value);    // existing 8-bit path
    static constexpr uint16_t correct(uint16_t value);  // new 16-bit path
};

struct GammaTableMethod // be sure to reference src\original\internal\colors\NeoGammaTableMethod.h for implementation deails on uint16_t
{
    static constexpr uint8_t correct(uint8_t value);    // 256-entry LUT
    static constexpr uint16_t correct(uint16_t value);  // equation fallback
};

struct GammaDynamicTableMethod
{
    // ...
    uint8_t correct(uint8_t value) const;               // direct LUT lookup
    uint16_t correct(uint16_t value) const;             // LUT + interpolation
};
```

`GammaShader<TColor, T_GAMMA>` calls `T_GAMMA::correct(color[ch])` and
overload resolution picks the right width automatically based on
`TColor::ComponentType`. The 8-bit path is unchanged — no template
instantiation overhead, no behavioral difference from today.

`GammaShader` then calls `T_GAMMA::correct(color[ch])` and overload
resolution picks the right width automatically based on `TColor::ComponentType`.

### 4.3 CurrentLimiterShader: Component-Width Adaptation

The current limiter accumulates `value × milliamps` across all channels.
For `uint16_t` components, the accumulator needs wider arithmetic to avoid
overflow:

```cpp
template <typename TColor>
class CurrentLimiterShader : public IShader<TColor>
{
    // For uint8_t:  max per pixel = 255 * 20 * 5 = 25,500 → uint32_t fine
    // For uint16_t: max per pixel = 65535 * 20 * 5 = 6,553,500 → uint32_t fine
    // For 2000 pixels at 16-bit: 2000 * 6,553,500 = 13,107,000,000 → uint64_t needed
    //
    // Use uint64_t accumulator unconditionally. On ARM Cortex-M33 (RP2350),
    // 64-bit addition is two instructions. This runs once per frame, not per pixel.
    uint64_t totalDrawWeighted = 0;
    // ...
    // Scale factor: (max * MaxComponent+1) / total
    // to handle both 255 and 65535 max values.
};
```

### 4.4 ColorOrderTransform: Component-Width Adaptation

`ColorOrderTransform` currently writes `uint8_t` values to the byte buffer.
With 16-bit components, it needs to write 2 bytes per channel (MSB-first or
LSB-first depending on the wire protocol).

```cpp
template <typename TColor>
class ColorOrderTransform
{
public:
    void apply(std::span<uint8_t> pixels, std::span<const TColor> colors)
    {
        size_t offset = 0;
        for (const auto& color : colors)
        {
            for (uint8_t ch = 0; ch < _config.channelCount; ++ch)
            {
                auto value = color[_config.channelOrder[ch]];
                writeComponent(pixels, offset, value,
                               typename TColor::ComponentType());
            }
        }
    }

    size_t bytesNeeded(size_t pixelCount) const
    {
        return pixelCount * _config.channelCount * sizeof(typename TColor::ComponentType);
    }

private:
    static void writeComponent(std::span<uint8_t> pixels,
                               size_t& offset,
                               uint8_t value,
                               uint8_t)
    {
        pixels[offset++] = value;
    }

    static void writeComponent(std::span<uint8_t> pixels,
                               size_t& offset,
                               uint16_t value,
                               uint16_t)
    {
        // MSB first (most LED protocols)
        pixels[offset++] = static_cast<uint8_t>(value >> 8);
        pixels[offset++] = static_cast<uint8_t>(value & 0xFF);
    }
};
```

### 4.5 ColorIterator Templating

`ColorIterator` becomes `ColorIterator<TColor>`, holding a
`std::function<TColor&(uint16_t)>`. The source helpers follow:

```cpp
template <typename TColor>
struct SolidColorSource
{
    TColor   color;
    uint16_t pixelCount;
    // ...
};

template <typename TColor>
struct SpanColorSource
{
    std::span<TColor> data;
    // ...
};
```

### 4.6 IPixelBus / PixelBus Templating

```cpp
template <typename TColor>
class IPixelBus
{
public:
    virtual void setPixelColors(size_t offset,
                                ColorIterator<TColor> first,
                                ColorIterator<TColor> last) = 0;
    // ...
    virtual void setPixelColor(size_t index, const TColor& color);
    virtual TColor getPixelColor(size_t index) const;
    // ...
};

template <typename TColor>
class PixelBus : public IPixelBus<TColor>
{
    std::vector<TColor> _colors;
    ResourceHandle<IProtocol<TColor>> _emitter;
    // ...
};
```

### 4.7 MosaicBus / ConcatBus / SegmentBus

All aggregate buses are templated on the same `TColor`:

```cpp
template <typename TColor>
class MosaicBus : public IPixelBus<TColor>
{
    // All children are IPixelBus<TColor>& — same Color type enforced
    // ...
};

template <typename TColor>
class ConcatBus : public IPixelBus<TColor>
{
    // All children are IPixelBus<TColor>& — same Color type enforced
    // ...
};
```

**No runtime color conversion between children.** If you want to mix RGB
and RGBW strips, you use separate bus instances with separate Color types.

## 5. Two-Wire Emitter Serialization: Channel Count Handling

Each two-wire emitter has a fixed wire-level channel count and bit depth
determined by its LED chip. Because the Color type is hardcoded in the
emitter's class declaration (see §4.1), there is no type mismatch to
handle — the emitter's `update()` receives exactly the Color type it
expects. Serialization converts directly from the fixed Color to wire bytes.

| Emitter | Color Type | Wire Channels | Wire Bits/Ch | Serialization |
|---------|-----------|--------------|-------------|---------------|
| DotStar | `TColor` (templated, ≥3-ch u8) | 3 (+1 prefix) | 8 | `channelOrder[0..2]` from string order (for example `ChannelOrder::BGR`), optional `'W'` for luminance |
| HD108 | `Rgb16Color` | 3 | 16 | `channelOrder[0..2]` from string order (for example `ChannelOrder::BGR`) — direct 16-bit write |
| LPD6803 | `RgbColor` | 3 (packed 5-5-5) | 5 | `channelOrder[0..2]` from string order (for example `ChannelOrder::RGB`) — narrow u8 → 5-bit |
| LPD8806 | `RgbColor` | 3 | 7 | `channelOrder[0..2]` from string order (for example `ChannelOrder::GRB`) — narrow u8 → 7-bit, MSB set |
| P9813 | `RgbColor` | 3 | 8 | `'B', 'G', 'R'` (fixed BGR) — direct |
| SM16716 | `RgbColor` | 3 | 8 | `channelOrder[0..2]` from string order (for example `ChannelOrder::RGB`) — direct |
| TLC5947 | `Rgb16Color` | 3 | 12 | `'R', 'G', 'B'` — narrow u16 → 12-bit |
| TLC59711 | `Rgb16Color` | 3 | 16 | `'R', 'G', 'B'` — direct 16-bit write |
| WS2801 | `RgbColor` | 3 | 8 | `channelOrder[0..2]` from string order (for example `ChannelOrder::RGB`) — direct |

For one-wire emitters (templated), `ColorOrderTransform<TColor>` config
specifies `channelCount` (3, 4, or 5) and `channelOrder`, selecting which
channels from the Color to emit. `channelCount` must be ≤
`TColor::ChannelCount`.

### 5.1 Bit-Width Narrowing in Fixed-Color Emitters

Since each fixed-Color emitter knows its exact ComponentType at compile
time, bit-width conversion is straightforward — no `if constexpr` needed.
The emitter simply narrows its known type to the wire width:

```cpp
// LPD6803 (RgbColor → 5-bit): narrow u8 → 5-bit
uint8_t fiveBit = value >> 3;

// LPD8806 (RgbColor → 7-bit): narrow u8 → 7-bit, set MSB
uint8_t sevenBit = (value >> 1) | 0x80;

// TLC5947 (Rgb16Color → 12-bit): narrow u16 → 12-bit
uint16_t twelveBit = value >> 4;
```

### 5.2 Bit-Width Conversion in Templated Emitters

Templated emitters (one-wire, DotStar, Print) use compile-time overload/tag
dispatch on `ComponentType` to serialize correctly for both 8-bit and 16-bit
Color:

```cpp
// ColorOrderTransform<TColor>::apply()
auto value = color[_config.channelOrder[ch]];
writeComponent(pixels, offset, value, typename TColor::ComponentType());
```

### 5.3 Current Protocol Capability Matrix (Audit Snapshot)

The table below captures current implementation behavior and target templated
Color behavior where migration is required.

| Protocol / Chip Family | Supported Channels | Current Input Component Type | Wire Bit Depth | Target Input Component Type |
|------------------------|--------------------|------------------------------|----------------|-----------------------------|
| `Ws2812xProtocol` (WS2812x / SK6812 / WS2813 / WS2816 timing family) | 3-5 (from `channelOrder`, capped at 5) | `uint8_t` | 8-bit/ch | `uint8_t` and `uint16_t` |
| `Tm1814Protocol` | 4 | `uint8_t` | 8-bit/ch (+ settings bytes) | `uint8_t` (fixed) |
| `Tm1914Protocol` | 3 | `uint8_t` | 8-bit/ch (+ settings bytes) | `uint8_t` (fixed) |
| `Sm168xProtocol` | 3 / 4 / 5 (variant) | `uint8_t` | 8-bit/ch (+ gain config bits) | `uint8_t` (fixed) |
| `Sm16716Protocol` | 3 | `uint8_t` | 8-bit/ch (bit-packed stream) | `uint8_t` (fixed) |
| `DotStarProtocol` (APA102/DotStar) | 3 color + optional 5-bit global brightness | `uint8_t` | 8-bit/ch + 5-bit brightness prefix | `uint8_t` (templated, >=3 channels) |
| `Hd108Protocol` | 3 | `uint8_t` (expanded internally) | 16-bit/ch | `uint16_t` (fixed `Rgb16Color`) |
| `Ws2801Protocol` | 3 | `uint8_t` | 8-bit/ch | `uint8_t` (fixed) |
| `PixieProtocol` | 3 | `uint8_t` | 8-bit/ch | `uint8_t` (fixed) |
| `P9813Protocol` | 3 | `uint8_t` | 8-bit/ch (+ checksum prefix) | `uint8_t` (fixed) |
| `Lpd6803Protocol` | 3 | `uint8_t` | 5-bit/ch packed | `uint8_t` (fixed) |
| `Lpd8806Protocol` | 3 | `uint8_t` | 7-bit/ch (+ MSB set) | `uint8_t` (fixed) |
| `Tlc5947Protocol` | 3 | `uint8_t` (expanded to 12-bit) | 12-bit/ch | `uint16_t` (fixed `Rgb16Color`, narrowed to 12-bit) |
| `Tlc59711Protocol` | 3 | `uint8_t` (expanded to 16-bit) | 16-bit/ch | `uint16_t` (fixed `Rgb16Color`) |
| `Dmx512Protocol` | Config field exists, update path currently emits first 3 channels | `uint8_t` | 8-bit slot | `uint8_t` now; revisit after core migration |

Notes:

- The WS2812x family is the highest-priority templated protocol because it
    must support both `uint8_t` and `uint16_t` component widths with 3-5 channels.
- Two-wire fixed-color protocols should hardcode their Color alias to prevent
    accidental mismatched channel/bit-depth combinations.

## 6. Interoperability Between Color Types

While `ConcatBus`/`MosaicBus` do not support mixed Color types, users still
need to convert between them (e.g., computing in `Rgb16Color` and displaying
on an 8-bit bus). This is a free function, not part of the bus infrastructure:

```cpp
namespace npb
{

    // Widen: Color<N, uint8_t> → Color<N, uint16_t>
    template <size_t N>
    constexpr Color<N, uint16_t> widen(const Color<N, uint8_t>& src)
    {
        Color<N, uint16_t> result;
        for (size_t ch = 0; ch < N; ++ch)
        {
            result[ch] = (static_cast<uint16_t>(src[ch]) << 8) | src[ch];
        }
        return result;
    }

    // Narrow: Color<N, uint16_t> → Color<N, uint8_t>
    template <size_t N>
    constexpr Color<N, uint8_t> narrow(const Color<N, uint16_t>& src)
    {
        Color<N, uint8_t> result;
        for (size_t ch = 0; ch < N; ++ch)
        {
            result[ch] = static_cast<uint8_t>(src[ch] >> 8);
        }
        return result;
    }

    // Expand channels: Color<M, T> → Color<N, T> where N > M
    // Extra channels default to 0.
    template <size_t N, size_t M, typename T,
              typename std::enable_if<(N > M), int>::type = 0>
    constexpr Color<N, T> expand(const Color<M, T>& src)
    {
        Color<N, T> result;
        for (size_t ch = 0; ch < M; ++ch)
        {
            result[ch] = src[ch];
        }
        return result;
    }

    // Compress channels: Color<M, T> → Color<N, T> where N < M
    // Extra channels discarded.
    template <size_t N, size_t M, typename T,
              typename std::enable_if<(N < M), int>::type = 0>
    constexpr Color<N, T> compress(const Color<M, T>& src)
    {
        Color<N, T> result;
        for (size_t ch = 0; ch < N; ++ch)
        {
            result[ch] = src[ch];
        }
        return result;
    }

}
```

## 7. HSB/HSL/HTML Color Models

These conversion types remain non-templated. They convert to/from `RgbColor`
(= `Color<3, uint8_t>`) which covers their use case (user-facing color
selection, web UI). A user working with 16-bit strips would convert
`HsbColor → RgbColor → widen() → Rgb16Color`.

If 16-bit HSB/HSL conversions are needed later, they can be added as
overloaded `toColor<TColor>()` methods without breaking the design.

## 8. Memory Impact

| Scenario | Current (5×u8) | With `RgbColor` (3×u8) | Savings |
|----------|---------------|----------------------|---------|
| 300 px WS2812 | 1,500 B | 900 B | 600 B (40%) |
| 300 px SK6812 RGBW | 1,500 B | 1,200 B | 300 B (20%) |
| 300 px WS2816 (16-bit) | 1,500 B (lossy!) | 1,800 B (`Rgb16Color`) | −300 B but lossless |
| 2000 px WS2812 | 10,000 B | 6,000 B | 4,000 B (40%) |

The memory savings for the common case (RGB 8-bit) are substantial. The 16-bit
case trades ~20% more memory for correct precision — a worthwhile trade on chips
with ≥264 KB SRAM.

## 9. Implementation Phases

### Phase A — Core Color Template + Aliases (no downstream changes)

1. Convert `Color` to `template<size_t N, typename T> class Color`
2. Add type aliases (`RgbColor`, `RgbwColor`, `RgbcwColor`, `Rgb16Color`, `Rgbw16Color`)
3. Add `widen()`, `narrow()`, `expand()`, `compress()` free functions
4. Temporarily add `using Color = RgbcwColor;` so all existing code compiles unchanged

**Validation:** All existing smoke tests compile and pass unchanged.

### Phase B — Template ColorIterator + Source Helpers

1. Template `ColorIterator<TColor>`
2. Template `SolidColorSource<TColor>`, `SpanColorSource<TColor>`
3. Update `IPixelBus<TColor>`, `PixelBus<TColor>`

**Validation:** Existing tests updated to use `RgbcwColor` explicitly; compile and pass.

### Phase C — Template Shaders

1. Template `IShader<TColor>`
2. Template `GammaShader<TColor, T_GAMMA>`
3. Template `CurrentLimiterShader<TColor>`
4. Template `ShaderChain<TColor>` / `NilShader<TColor>`
5. Add `uint16_t` overloads to gamma methods

Detailed Phase C rollout:

6. Introduce `using ComponentType = typename TColor::ComponentType` inside each
    shader and remove direct assumptions of `uint8_t`.
7. Update `GammaShader` loops to use `TColor::ChannelCount` and rely on overload
    resolution for `T_GAMMA::correct(ComponentType)`.
8. Update `CurrentLimiterShader` arithmetic to use `uint64_t` accumulation and a
    scale formula based on `std::numeric_limits<ComponentType>::max()`.
9. Update `WhiteBalanceShader` to keep Kelvin correction tables in `uint8_t`,
    but apply scaling in widened intermediate math and normalize to
    `ComponentType` max range.
10. Template `WithShaderProtocol` so scratch storage uses `std::vector<TColor>`
     and the shader handle is `ResourceHandle<IShader<TColor>>`.

**Validation:** Shader smoke tests with both `RgbColor` and `Rgb16Color`.

### Phase D — Template and Fix Emitters

1. Template `IProtocol<TColor>`
2. Template `ColorOrderTransform<TColor>` with component-width byte serialization
3. Template `PrintProtocol<TColor>` (simplest — do first)
4. Template `DotStarProtocol<TColor>` with `static_assert` for ≥3-ch 8-bit
5. Template all one-wire emitters (they use `ColorOrderTransform<TColor>`)
6. Convert fixed-Color two-wire emitters — remove templating, hardcode Color type:
   - `Hd108Protocol : IProtocol<Rgb16Color>`
   - `Tlc59711Protocol : IProtocol<Rgb16Color>`
   - `Tlc5947Protocol : IProtocol<Rgb16Color>`
   - `Lpd6803Protocol : IProtocol<RgbColor>`
   - `Lpd8806Protocol : IProtocol<RgbColor>`
   - `P9813Protocol : IProtocol<RgbColor>`
   - `Sm16716Protocol : IProtocol<RgbColor>`
   - `Ws2801Protocol : IProtocol<RgbColor>`

**Validation:** Each emitter tested with its fixed Color type.

### Phase E — Template Aggregate Buses

1. Template `ConcatBus<TColor>`
2. Template `MosaicBus<TColor>`
3. Template `SegmentBus<TColor>`

**Validation:** Mosaic smoke tests with `RgbColor`.

### Phase F — Remove Compatibility Shim

1. Remove `using Color = RgbcwColor;`
2. Update all examples to use explicit type aliases
3. Update smoke tests

## 10. Risk Assessment

| Risk | Mitigation |
|------|------------|
| Template bloat — each Color type instantiates the full emitter | Only 2-3 Color types are commonly used; dead code elimination handles the rest. On RP2040 flash is 2-16 MB. |
| Complexity of compile-time width dispatch helpers | Each emitter has exactly one serialization loop — the width-specific helpers are small and self-contained. |
| Breaking existing examples | Phase A's compatibility `using` alias keeps everything compiling during migration. |
| Header-only template compilation time | The emitters are already header-only. Each translation unit typically includes only one emitter. |

## 11. Non-Goals

- **Runtime-variable channel count.** The channel count is a compile-time template
  parameter. This is intentional — it enables loop unrolling, eliminates branches
  in hot pixel loops, and allows `static_assert` for emitter compatibility.

- **Mixed-type aggregate buses.** `ConcatBus` and `MosaicBus` require all children
  to share the same `TColor`. Mixing RGB and RGBW strips in one aggregate bus is
  not supported. Use separate buses.

- **Float or double components.** `TComponent` is expected to be an unsigned
  integer (`uint8_t` or `uint16_t`). Float-per-channel LEDs don't exist, and
  float Color would be used only for intermediate computation — which is better
  served by the existing HSB/HSL types.

- **Templating topology classes.** `PanelTopology`, `TiledTopology`, and
  `PanelLayout` are pure coordinate mappers with no Color dependency. They
  remain non-templated.
