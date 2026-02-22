# Architecture & Implementation Plan: Colors, Blending, and Topology

This document covers four deferred feature areas from the API gap analysis
that are now ready for design and implementation in the `src/virtual/` rewrite:

1. **Color blending & utility methods** — `LinearBlend`, `BilinearBlend`, `Dim`, `Brighten`, `Darken`, `Lighten`, `CalculateBrightness`, `CompareTo`
2. **Non-RGB color models** — `HsbColor`, `HslColor`, `HtmlColor`, hue-blend strategies
3. **2D topology / linearization** — `NeoTopology` equivalent: `(x, y)` → linear index with pluggable layouts
4. **Multi-bus aggregation / mosaic** — `NeoMosaic`/`NeoTiles` equivalent: combining multiple `IPixelBus` instances into a single logical 2D pixel surface

---

## 1. Color Blending & Utility Methods

### 1.1 What Exists in the Original

Every concrete color class (`RgbColor`, `RgbwColor`, `Rgb48Color`, etc.) independently
implements the same set of utility methods:

| Method | Signature | Description |
|--------|-----------|-------------|
| `CalculateBrightness()` | `uint8_t () const` | Average of all channels |
| `Dim(ratio)` | `T (uint8_t) const` | Blend toward black (0 = black, 255 = original) |
| `Brighten(ratio)` | `T (uint8_t) const` | Blend toward white (0 = white, 255 = original) |
| `Darken(delta)` | `void (uint8_t)` | Subtract delta from each channel (clamped to 0) |
| `Lighten(delta)` | `void (uint8_t)` | Add delta to each channel (clamped to 255) |
| `LinearBlend(left, right, progress)` | `static T (T, T, float)` | Per-channel linear interpolation |
| `LinearBlend(left, right, progress)` | `static T (T, T, uint8_t)` | Integer-only variant (>>8 fixed-point) |
| `BilinearBlend(c00,c01,c10,c11,x,y)` | `static T (T,T,T,T, float, float)` | 2D weighted interpolation (4 corners) |
| `CompareTo(other, epsilon)` | `int16_t (T, uint8_t) const` | Greatest per-channel difference |

These are duplicated across all 7 color types with minor variations (channel count,
element width). The math is identical — only the number of channels and the max
value differ.

### 1.2 Design for the Rewrite

Since all pixel data is now stored as `npb::Color` (5×uint8_t RGBWW), these methods
belong on `Color` itself. There is exactly one color type, so there is no duplication.

**Key decisions:**

- **Blend operates on all 5 channels uniformly.** Unused channels are 0 and blending
  0↔0 is a no-op, so there is no cost for unused white channels.
- **Integer-only fast path.** The `uint8_t progress` overload uses fixed-point
  arithmetic (`(delta * progress + 1) >> 8`) — no floats. This is the preferred
  path on microcontrollers.
- **`Dim`/`Brighten` use the original's proven integer math.** `_elementDim` =
  `(value * (ratio + 1)) >> 8`. `_elementBrighten` = `((value + 1) << 8) / (ratio + 1) - 1`.
- **`Darken`/`Lighten` are mutating** (match original behavior) — they modify `this`.
- **All blend methods are `constexpr` where possible.**
- **`BilinearBlend` is useful for 2D gradient fills** on matrix panels and is worth
  keeping as a static method.

### 1.3 API Surface on `Color`

```cpp
namespace npb
{

class Color
{
public:
    // ... existing members ...

    // --- Brightness ---
    constexpr uint8_t calculateBrightness() const;

    // --- Blend toward black/white (non-destructive, return new Color) ---
    constexpr Color dim(uint8_t ratio) const;       // 255 = original, 0 = black
    constexpr Color brighten(uint8_t ratio) const;   // 255 = original, 0 = white

    // --- Shift toward black/white (mutating) ---
    constexpr void darken(uint8_t delta);
    constexpr void lighten(uint8_t delta);

    // --- Linear blend ---
    static constexpr Color linearBlend(const Color& left, const Color& right, float progress);
    static constexpr Color linearBlend(const Color& left, const Color& right, uint8_t progress);

    // --- Bilinear blend (2D) ---
    static constexpr Color bilinearBlend(
        const Color& c00, const Color& c01,
        const Color& c10, const Color& c11,
        float x, float y);

    // --- Comparison ---
    constexpr int16_t compareTo(const Color& other, uint8_t epsilon = 1) const;
    static constexpr int16_t compare(const Color& left, const Color& right, uint8_t epsilon = 1);

private:
    static constexpr uint8_t _elementDim(uint8_t value, uint8_t ratio);
    static constexpr uint8_t _elementBrighten(uint8_t value, uint8_t ratio);
};

} // namespace npb
```

### 1.4 Implementation Notes

All methods iterate over the `Channels` array using a loop, making them
automatically correct for all 5 channels:

```cpp
constexpr Color Color::dim(uint8_t ratio) const
{
    Color result;
    for (size_t ch = 0; ch < ChannelCount; ++ch)
    {
        result.Channels[ch] = _elementDim(Channels[ch], ratio);
    }
    return result;
}

static constexpr Color Color::linearBlend(const Color& left, const Color& right, uint8_t progress)
{
    Color result;
    for (size_t ch = 0; ch < ChannelCount; ++ch)
    {
        result.Channels[ch] = left.Channels[ch]
            + (((static_cast<int32_t>(right.Channels[ch]) - left.Channels[ch])
                * static_cast<int32_t>(progress) + 1) >> 8);
    }
    return result;
}
```

### 1.5 File Changes

| File | Change |
|------|--------|
| `src/virtual/colors/Color.h` | Add all methods listed in §1.3 — header-only, inline `constexpr` |

No new files needed. No `.cpp` file needed (everything is `constexpr`/inline).

---

## 2. Non-RGB Color Models (HsbColor, HslColor, HtmlColor)

### 2.1 What Exists in the Original

Three auxiliary color classes provide alternative color spaces with bidirectional
conversion to/from `RgbColor`:

| Class | Members | Conversions | Blending |
|-------|---------|-------------|----------|
| `HsbColor` | `float H, S, B` (0.0–1.0) | `RgbColor → HsbColor`, `HsbColor → RgbColor` | `LinearBlend<T_NEOHUEBLEND>`, `BilinearBlend<T_NEOHUEBLEND>` |
| `HslColor` | `float H, S, L` (0.0–1.0) | `RgbColor → HslColor`, `HslColor → RgbColor` | `LinearBlend<T_NEOHUEBLEND>`, `BilinearBlend<T_NEOHUEBLEND>` |
| `HtmlColor` | `uint32_t Color` (0xRRGGBB) | `RgbColor → HtmlColor`, `HtmlColor → RgbColor` | None — parsing/formatting only |

**Hue blend strategies** (template parameter `T_NEOHUEBLEND` on HSB/HSL blend methods):

| Strategy | Behavior |
|----------|----------|
| `NeoHueBlendShortestDistance` | Wraps around the hue circle via the shortest arc |
| `NeoHueBlendLongestDistance` | Wraps via the longest arc |
| `NeoHueBlendClockwiseDirection` | Always increases hue (wraps at 1.0→0.0) |
| `NeoHueBlendCounterClockwiseDirection` | Always decreases hue (wraps at 0.0→1.0) |

**Color name tables** (`HtmlColorNames`, `HtmlShortColorNames`):
- Stored in PROGMEM on AVR; regular flash on ARM/Pico
- `HtmlColor::Parse<T_HTMLCOLORNAMES>(name)` — resolve `"red"` → `0xFF0000`
- `HtmlColor::ToString<T_HTMLCOLORNAMES>(buf, size)` — resolve `0xFF0000` → `"red"`
- Also parses `#rgb` and `#rrggbb` hex notation

**Conversion pipeline** (original):
```
RgbColor → HsbColor: (R,G,B) as floats → max/min → hue/saturation/brightness
HsbColor → RgbColor: sector-based piecewise (6 sectors of hue wheel)
RgbColor → HslColor: (R,G,B) as floats → max/min → hue/saturation/lightness
HslColor → RgbColor: CalcColor(p, q, t) piecewise helper
```

The math lives in `RgbColorBase` (shared static methods `_HslToRgb`, `_HsbToRgb`,
`_CalcColor`) and the constructors of each class.

### 2.2 Design for the Rewrite

These color models are **user-facing convenience types** — they are never stored in
the pixel buffer (which is always `Color`). Their purpose is:

1. Provide a perceptually intuitive way to define and blend colors
2. Convert to `Color` for actual use with `IPixelBus`
3. Parse/format color strings (web UI integration)

**Key decisions:**

- **Standalone classes in `npb` namespace.** `HsbColor`, `HslColor`, `HtmlColor` are
  independent types, not subclasses of `Color`.
- **Conversion is via explicit constructors and `toColor()` methods.** We use `Color`
  instead of `RgbColor` — only the R, G, B channels are populated (WW/CW = 0).
- **Hue blend strategies become an enum + free function** instead of template parameters.
  The original uses templates because it was C++11 and needed zero-overhead static dispatch.
  With C++23 and `constexpr`, we can use a simple `enum class HueBlendMode` and a single
  `hueBlend(float left, float right, float progress, HueBlendMode)` function. The
  compiler will optimize the switch into the same code as the template dispatch.
- **`HtmlColor` drops PROGMEM dependency.** On RP2040, flash is memory-mapped — no
  `pgm_read_*` needed. Name tables become `constexpr` arrays of `std::string_view` pairs.
- **`Rgb16Color` is dropped.** It was only relevant for 565 displays, not LED strips.

### 2.3 API Surface

#### 2.3.1 HueBlend

```cpp
// File: src/virtual/colors/HueBlend.h

namespace npb
{

enum class HueBlendMode
{
    ShortestDistance,
    LongestDistance,
    Clockwise,
    CounterClockwise
};

constexpr float hueBlend(float left, float right, float progress,
                         HueBlendMode mode = HueBlendMode::ShortestDistance);

} // namespace npb
```

Implementation: a single function with a `switch` on the mode, containing the
same math as the four original strategy classes. The `FixWrap` helper is inlined.

#### 2.3.2 HsbColor

```cpp
// File: src/virtual/colors/HsbColor.h

namespace npb
{

class HsbColor
{
public:
    float H{0.0f};  // 0.0 – 1.0
    float S{0.0f};  // 0.0 – 1.0
    float B{0.0f};  // 0.0 – 1.0

    constexpr HsbColor() = default;
    constexpr HsbColor(float h, float s, float b);

    // Convert from Color (uses R,G,B channels)
    explicit HsbColor(const Color& color);

    // Convert to Color (populates R,G,B; WW=CW=0)
    Color toColor() const;

    // Blending
    static HsbColor linearBlend(const HsbColor& left, const HsbColor& right,
                                float progress,
                                HueBlendMode mode = HueBlendMode::ShortestDistance);

    static HsbColor bilinearBlend(const HsbColor& c00, const HsbColor& c01,
                                  const HsbColor& c10, const HsbColor& c11,
                                  float x, float y,
                                  HueBlendMode mode = HueBlendMode::ShortestDistance);

    constexpr bool operator==(const HsbColor&) const = default;
};

} // namespace npb
```

#### 2.3.3 HslColor

```cpp
// File: src/virtual/colors/HslColor.h

namespace npb
{

class HslColor
{
public:
    float H{0.0f};
    float S{0.0f};
    float L{0.0f};

    constexpr HslColor() = default;
    constexpr HslColor(float h, float s, float l);

    explicit HslColor(const Color& color);
    Color toColor() const;

    static HslColor linearBlend(const HslColor& left, const HslColor& right,
                                float progress,
                                HueBlendMode mode = HueBlendMode::ShortestDistance);

    static HslColor bilinearBlend(const HslColor& c00, const HslColor& c01,
                                  const HslColor& c10, const HslColor& c11,
                                  float x, float y,
                                  HueBlendMode mode = HueBlendMode::ShortestDistance);

    constexpr bool operator==(const HslColor&) const = default;
};

} // namespace npb
```

#### 2.3.4 HtmlColor

```cpp
// File: src/virtual/colors/HtmlColor.h

namespace npb
{

struct HtmlColorPair
{
    std::string_view name;
    uint32_t color;  // 0xRRGGBB
};

namespace detail
{
    // CSS3 named colors — stored as constexpr sorted array
    inline constexpr HtmlColorPair FullColorTable[] = { /* 148 entries */ };
    inline constexpr HtmlColorPair ShortColorTable[] = { /* 17 entries */ };
}

enum class HtmlColorNameSet
{
    Full,   // 148 CSS3 named colors
    Short   // 17 basic HTML4 colors
};

class HtmlColor
{
public:
    uint32_t Value{0};  // 0xRRGGBB

    constexpr HtmlColor() = default;
    constexpr HtmlColor(uint32_t color);

    // Convert from Color (uses R,G,B channels)
    constexpr explicit HtmlColor(const Color& color);

    // Convert to Color (populates R,G,B; WW=CW=0)
    constexpr Color toColor() const;

    // Parse "#rgb", "#rrggbb", or named color
    // Returns number of characters consumed, 0 on failure
    size_t parse(std::string_view name,
                 HtmlColorNameSet nameSet = HtmlColorNameSet::Full);

    // Format to string — tries name lookup first, falls back to "#rrggbb"
    size_t toString(std::span<char> buf,
                    HtmlColorNameSet nameSet = HtmlColorNameSet::Full) const;

    // Format to numeric "#rrggbb" only
    size_t toNumericalString(std::span<char> buf) const;

    constexpr bool operator==(const HtmlColor&) const = default;
};

} // namespace npb
```

### 2.4 Conversion Math — Shared Helpers

The HSB↔RGB and HSL↔RGB conversion math currently lives in `RgbColorBase`
(static protected methods). In the rewrite, these become free functions in
a `detail` namespace:

```cpp
// File: src/virtual/colors/ColorConversions.h (internal header)

namespace npb::detail
{
    void rgbToHsb(float r, float g, float b, float& h, float& s, float& bri);
    void hsbToRgb(float h, float s, float b, float& r, float& g, float& bri);
    void rgbToHsl(float r, float g, float b, float& h, float& s, float& l);
    void hslToRgb(float h, float s, float l, float& r, float& g, float& b);
    float calcColor(float p, float q, float t);  // HslToRgb helper
}
```

These functions contain the same proven math from the original `RgbColorBase.cpp`,
`HsbColor.cpp`, and `HslColor.cpp`.

### 2.5 File Manifest

| # | File | Type |
|---|------|------|
| 1 | `src/virtual/colors/HueBlend.h` | header-only (enum + `constexpr` function) |
| 2 | `src/virtual/colors/HsbColor.h` | header + inline impl |
| 3 | `src/virtual/colors/HslColor.h` | header + inline impl |
| 4 | `src/virtual/colors/HtmlColor.h` | header + inline impl |
| 5 | `src/virtual/colors/ColorConversions.h` | header-only detail (conversion math) |
| 6 | `src/virtual/colors/HtmlColorData.h` | header-only (constexpr name tables) |

---

## 3. 2D Topology / Linearization

### 3.1 What Exists in the Original

`NeoTopology<T_LAYOUT>` maps 2D panel coordinates `(x, y)` to a linear 1D strip
index. The layout is a compile-time template parameter:

| Layout Base | Variants (Rotations) | Description |
|-------------|---------------------|-------------|
| `RowMajorLayout` | `RowMajor{90,180,270}Layout` | Left-to-right, top-to-bottom |
| `RowMajorAlternatingLayout` | `RowMajorAlternating{90,180,270}Layout` | Serpentine (zig-zag) rows |
| `ColumnMajorLayout` | `ColumnMajor{90,180,270}Layout` | Top-to-bottom, left-to-right |
| `ColumnMajorAlternatingLayout` | `ColumnMajorAlternating{90,180,270}Layout` | Serpentine columns |

Each layout class has a `static uint16_t Map(width, height, x, y)` method.

Additionally, each layout base carries a `TilePreference` inner typedef set that
tells `NeoMosaic` which rotation to use for even/odd row/column tile positions
(to minimize inter-tile wiring distance).

`NeoRingTopology<T_LAYOUT>` handles concentric ring arrangements — a separate
1D-to-1D mapping (ring, pixel) → index using a user-provided ring-size table.

**Usage pattern** (topology is always external to the bus):
```cpp
NeoTopology<RowMajorAlternatingLayout> topo(8, 8);
strip.SetPixelColor(topo.Map(x, y), color);
```

The topology is a coordinate mapper — it does not own or manage pixels.

### 3.2 Design for the Rewrite

The topology system is orthogonal to `IPixelBus` — it is a pure (x,y)→index
mapping utility. In the rewrite we want to:

1. **Preserve the layout algebra** — the 16 layout classes encode real-world wiring
   patterns that users depend on.
2. **Make layouts runtime-selectable** — use an enum + switch instead of templates.
   Most users configure panel layout once at startup and never change it. The
   per-pixel cost of a switch is negligible compared to the LED write time.
3. **Integrate cleanly with `IPixelBus`** — topology returns an index, user
   passes it to `setPixelColor(index, color)`.
4. **Support use as a building block for Mosaic** — the tile-preference rotation
   scheme must be preserved.

**Key decisions:**

- **Layouts become an enum `PanelLayout`** with 16 values (4 bases × 4 rotations).
- **`PanelTopology` is a runtime class** (not templated) that stores width, height,
  and layout enum. Its `map(x, y) → uint16_t` method uses a switch or function
  pointer table internally.
- **`mapProbe(x, y)` returns `std::optional<uint16_t>`** instead of returning
  `width*height` as an out-of-bounds sentinel. This is safer and more idiomatic C++23.
- **`RingTopology` is preserved as a separate class**, since it has fundamentally
  different input semantics (ring index + pixel-within-ring).
- **Tile preferences are a constexpr lookup table** indexed by `PanelLayout` and
  tile position parity, returning the appropriate rotated layout.

### 3.3 API Surface

```cpp
// File: src/virtual/topologies/PanelLayout.h

namespace npb
{

enum class PanelLayout : uint8_t
{
    RowMajor,
    RowMajor90,
    RowMajor180,
    RowMajor270,
    RowMajorAlternating,
    RowMajorAlternating90,
    RowMajorAlternating180,
    RowMajorAlternating270,
    ColumnMajor,
    ColumnMajor90,
    ColumnMajor180,
    ColumnMajor270,
    ColumnMajorAlternating,
    ColumnMajorAlternating90,
    ColumnMajorAlternating180,
    ColumnMajorAlternating270,
};

// Pure mapping function — no bounds checking
constexpr uint16_t mapLayout(PanelLayout layout,
                             uint16_t width, uint16_t height,
                             uint16_t x, uint16_t y);

// Tile preference: given a base panel layout and the tile's row/column parity,
// returns the rotated layout that minimizes inter-tile wiring.
constexpr PanelLayout tilePreferredLayout(PanelLayout baseLayout,
                                          bool oddRow, bool oddColumn);

} // namespace npb
```

```cpp
// File: src/virtual/topologies/PanelTopology.h

namespace npb
{

class PanelTopology
{
public:
    constexpr PanelTopology(uint16_t width, uint16_t height, PanelLayout layout);

    // Map with clamping (out-of-bounds coordinates clamped to edges)
    constexpr uint16_t map(int16_t x, int16_t y) const;

    // Map with bounds checking (returns nullopt if out of bounds)
    constexpr std::optional<uint16_t> mapProbe(int16_t x, int16_t y) const;

    constexpr uint16_t width() const;
    constexpr uint16_t height() const;
    constexpr uint16_t pixelCount() const;

private:
    uint16_t _width;
    uint16_t _height;
    PanelLayout _layout;
};

} // namespace npb
```

```cpp
// File: src/virtual/topologies/RingTopology.h

namespace npb
{

class RingTopology
{
public:
    // rings: array of starting indices, with one extra entry for total count
    // e.g., {0, 1, 7, 19, 35, 59, 100} for 6 rings totaling 100 pixels
    explicit RingTopology(std::span<const uint16_t> rings);

    uint16_t map(uint8_t ring, uint16_t pixel) const;
    std::optional<uint16_t> mapProbe(uint8_t ring, uint16_t pixel) const;

    uint8_t ringCount() const;
    uint16_t pixelCountAtRing(uint8_t ring) const;
    uint16_t pixelCount() const;

    // Shift pixel index within a ring (clamped to ring bounds)
    uint16_t ringPixelShift(uint8_t ring, uint16_t pixel, int16_t shift) const;

    // Rotate pixel index within a ring (wraps around)
    uint16_t ringPixelRotate(uint8_t ring, uint16_t pixel, int16_t rotate) const;

private:
    std::vector<uint16_t> _rings;
};

} // namespace npb
```

### 3.4 `mapLayout` Implementation Sketch

The 16 layout formulas are small enough to inline into a `constexpr switch`:

```cpp
constexpr uint16_t mapLayout(PanelLayout layout,
                             uint16_t width, uint16_t height,
                             uint16_t x, uint16_t y)
{
    switch (layout)
    {
    case PanelLayout::RowMajor:
        return x + y * width;

    case PanelLayout::RowMajor90:
        return (width - 1 - x) * height + y;

    case PanelLayout::RowMajor180:
        return (width - 1 - x) + (height - 1 - y) * width;

    case PanelLayout::RowMajor270:
        return x * height + (height - 1 - y);

    case PanelLayout::RowMajorAlternating:
    {
        uint16_t index = y * width;
        return (y & 1) ? index + (width - 1 - x) : index + x;
    }
    case PanelLayout::RowMajorAlternating90:
    {
        uint16_t mx = (width - 1) - x;
        uint16_t index = mx * height;
        return (mx & 1) ? index + (height - 1 - y) : index + y;
    }
    // ... remaining 10 cases follow the same pattern from the original layout classes
    }
}
```

### 3.5 File Manifest

| # | File | Type |
|---|------|------|
| 1 | `src/virtual/topologies/PanelLayout.h` | header-only (`enum`, `mapLayout`, `tilePreferredLayout`) |
| 2 | `src/virtual/topologies/PanelTopology.h` | header-only class |
| 3 | `src/virtual/topologies/RingTopology.h` | header-only class |

---

## 4. Multi-Bus Aggregation / Mosaic

### 4.1 What Exists in the Original

The original has two aggregation concepts:

**`NeoTiles<T_MATRIX_LAYOUT, T_TILE_LAYOUT>`** — arranges identical panels in a grid.
Each panel uses `T_MATRIX_LAYOUT` internally. Tiles are arranged using
`T_TILE_LAYOUT`. Both are compile-time template parameters.

- `Map(x, y)`: splits global (x,y) into tile-local coordinates, maps the local
  position through `NeoTopology<T_MATRIX_LAYOUT>`, then computes the tile offset
  via `T_TILE_LAYOUT::Map()`.
- All tiles share a single `NeoPixelBus` — tiles are just index ranges within
  one linear strip.

**`NeoMosaic<T_LAYOUT>`** — similar to `NeoTiles` but with automatic rotation
optimization. Tiles in the mosaic are laid out using `RowMajorAlternating`. Each
tile's internal layout is rotated based on its row/column parity to minimize
inter-tile wire lengths. The `TilePreference` typedefs on each layout class
determine which rotation applies at each position.

Both `NeoTiles` and `NeoMosaic`:
- Are pure coordinate mappers (like `NeoTopology`)
- Do NOT own pixel data
- Do NOT manage multiple buses
- Assume all tiles are on a single bus, wired in sequence

### 4.2 Requirements for the Rewrite

The rewrite needs to support two distinct use cases:

**Use Case A: Single-bus tiled panels** (matching original behavior).
Multiple physical panels wired in series on one data line. One `PixelBus` instance,
one contiguous pixel array. The aggregator is a pure coordinate mapper, exactly
like the original `NeoTiles`/`NeoMosaic`.

**Use Case B: Multi-bus aggregation** (new capability).
Multiple independent `PixelBus` instances (potentially on different pins or even
different protocols) combined into a single logical 2D surface. Each bus owns its
own pixel buffer. The aggregator must:
- Map global 2D coordinates to the correct bus + local pixel index
- Delegate `setPixelColor`/`getPixelColor` to the appropriate bus
- Provide a unified `show()` that triggers all underlying buses

### 4.3 Design

We address these with two separate classes:

1. **`TiledTopology`** — pure coordinate mapper for Use Case A (single bus)
2. **`MosaicBus`** — multi-bus aggregator for Use Case B (implements `IPixelBus`)

#### 4.3.1 TiledTopology (Single-Bus Tiling)

This replaces both `NeoTiles` and `NeoMosaic`. It is a pure coordinate mapper
like `PanelTopology`, but adds tile-grid logic.

```cpp
// File: src/virtual/topologies/TiledTopology.h

namespace npb
{

struct TiledTopologyConfig
{
    uint16_t panelWidth;         // pixels per panel horizontally
    uint16_t panelHeight;        // pixels per panel vertically
    uint16_t tilesWide;          // number of panels horizontally
    uint16_t tilesHigh;          // number of panels vertically
    PanelLayout panelLayout;     // layout within each panel
    PanelLayout tileLayout;      // layout of panels in the grid
    bool mosaicRotation = false; // if true, auto-rotate panels per tile-preference
                                 // (NeoMosaic behavior)
};

class TiledTopology
{
public:
    constexpr explicit TiledTopology(const TiledTopologyConfig& config);

    // Map global (x,y) to linear index
    constexpr uint16_t map(int16_t x, int16_t y) const;
    constexpr std::optional<uint16_t> mapProbe(int16_t x, int16_t y) const;

    // Which panel does this global coordinate fall on? (for topology hints)
    enum class TopologyHint
    {
        FirstOnPanel,
        InPanel,
        LastOnPanel,
        OutOfBounds
    };
    constexpr TopologyHint topologyHint(int16_t x, int16_t y) const;

    constexpr uint16_t width() const;   // panelWidth * tilesWide
    constexpr uint16_t height() const;  // panelHeight * tilesHigh
    constexpr uint16_t pixelCount() const;

private:
    TiledTopologyConfig _config;
};

} // namespace npb
```

**Internal logic** (same as original `NeoMosaic::calculate()`):
1. `tileX = x / panelWidth`, `localX = x % panelWidth`
2. `tileY = y / panelHeight`, `localY = y % panelHeight`
3. `tileOffset = mapLayout(tileLayout, tilesWide, tilesHigh, tileX, tileY) * panelWidth * panelHeight`
4. If `mosaicRotation`:
   - `effectiveLayout = tilePreferredLayout(panelLayout, tileY & 1, tileX & 1)`
5. Else: `effectiveLayout = panelLayout`
6. `localIndex = mapLayout(effectiveLayout, panelWidth, panelHeight, localX, localY)`
7. Return `localIndex + tileOffset`

**Usage (Use Case A):**
```cpp
npb::TiledTopology mosaic({
    .panelWidth = 8,
    .panelHeight = 8,
    .tilesWide = 4,
    .tilesHigh = 2,
    .panelLayout = npb::PanelLayout::ColumnMajorAlternating,
    .tileLayout = npb::PanelLayout::RowMajorAlternating,
    .mosaicRotation = true
});

bus.setPixelColor(mosaic.map(x, y), color);
```

#### 4.3.2 MosaicBus (Multi-Bus Aggregation)

`MosaicBus` implements `IPixelBus` and manages multiple child buses arranged in
a 2D grid. Each child bus corresponds to one panel/tile in the mosaic.

```cpp
// File: src/virtual/buses/MosaicBus.h

namespace npb
{

struct MosaicPanel
{
    IPixelBus& bus;           // the panel's bus (must outlive MosaicBus)
    uint16_t panelWidth;      // pixels wide on this panel
    uint16_t panelHeight;     // pixels tall on this panel
    PanelLayout layout;       // pixel layout within this panel
};

struct MosaicBusConfig
{
    uint16_t tilesWide;       // grid columns
    uint16_t tilesHigh;       // grid rows
    PanelLayout tileLayout;   // how panels are arranged in the grid
                              // (only used for determining traversal order)
    bool mosaicRotation = false;
};

class MosaicBus : public IPixelBus
{
public:
    // panels: tilesWide * tilesHigh entries, in row-major order
    MosaicBus(std::span<MosaicPanel> panels, const MosaicBusConfig& config);

    // --- IPixelBus ---
    void begin() override;
    void show() override;                   // calls show() on all child buses
    bool canShow() const override;          // true only if ALL children can show

    size_t pixelCount() const override;     // sum of all panel pixel counts

    // 2D access (the primary interface for MosaicBus)
    void setPixelColor(int16_t x, int16_t y, const Color& color);
    Color getPixelColor(int16_t x, int16_t y) const;

    uint16_t width() const;
    uint16_t height() const;

    // --- Linear IPixelBus accessors (map through the tiled topology) ---
    std::span<Color> colors() override;            // not directly supported — returns empty
    std::span<const Color> colors() const override; // not directly supported — returns empty

    void setPixelColors(size_t offset,
                        std::span<const Color> pixelData) override;
    void getPixelColors(size_t offset,
                        std::span<Color> pixelData) const override;

private:
    std::vector<MosaicPanel> _panels;
    MosaicBusConfig _config;
    // internal mappings computed at construction
    uint16_t _totalWidth;
    uint16_t _totalHeight;

    // resolve global (x,y) → panel index + local pixel index
    struct ResolvedPixel
    {
        size_t panelIndex;
        uint16_t localIndex;
    };
    std::optional<ResolvedPixel> resolve(int16_t x, int16_t y) const;
};

} // namespace npb
```

**Design notes:**

- **`colors()` returns an empty span.** Unlike `PixelBus`, a `MosaicBus` does not
  own a contiguous pixel buffer — each panel has its own buffer. Direct span access
  across discontiguous memory is not meaningful. Users should use the 2D accessors
  or the indexed `setPixelColors`/`getPixelColors` methods.

- **`show()`** iterates all child buses and calls `show()` on each. For buses on
  different pins, this can be parallelized in a future optimization. For now,
  sequential is correct and simple.

- **`canShow()`** returns `true` only when ALL child buses report ready. This
  prevents partial updates.

- **Heterogeneous panels are supported.** Each `MosaicPanel` can have a different
  bus type (different emitter, different protocol) and even different dimensions,
  though in practice uniform panels are the common case.

- **Linear access (`setPixelColors` with offset)** traverses panels in tile-layout
  order, computing which panel each linear index falls into. This is a convenience
  for compatibility — the 2D interface is preferred.

**Usage (Use Case B):**
```cpp
// Three 8x8 panels on different GPIO pins
npb::PixelBus panel0(64, std::move(emitter0));
npb::PixelBus panel1(64, std::move(emitter1));
npb::PixelBus panel2(64, std::move(emitter2));

npb::MosaicPanel panels[] = {
    {panel0, 8, 8, npb::PanelLayout::ColumnMajorAlternating},
    {panel1, 8, 8, npb::PanelLayout::ColumnMajorAlternating},
    {panel2, 8, 8, npb::PanelLayout::ColumnMajorAlternating},
};

npb::MosaicBus mosaic(panels, {
    .tilesWide = 3,
    .tilesHigh = 1,
    .tileLayout = npb::PanelLayout::RowMajor,
});

mosaic.begin();
mosaic.setPixelColor(12, 5, npb::Color(255, 0, 0));  // global coordinate
mosaic.show();  // triggers show() on all three panels
```

### 4.4 File Manifest

| # | File | Type |
|---|------|------|
| 1 | `src/virtual/topologies/TiledTopology.h` | header-only class |
| 2 | `src/virtual/buses/MosaicBus.h` | header + inline impl |

---

## 5. Implementation Phasing

These features slot into the existing phase plan as follows:

### Phase 9 — Color Utilities & Blending

**Prerequisites:** Phase 1 (Color class exists)

| Step | Task | Files |
|------|------|-------|
| 9.1 | Add blend/dim/brighten/darken/lighten/compare to `Color` | `Color.h` |
| 9.2 | Add smoke test for blending | `examples-virtual/color-blend-test/main.cpp` |

### Phase 10 — Non-RGB Color Models

**Prerequisites:** Phase 9 (Color blend methods exist — used for HSB/HSL→Color verification)

| Step | Task | Files |
|------|------|-------|
| 10.1 | Create `HueBlend.h` | `colors/HueBlend.h` |
| 10.2 | Create `ColorConversions.h` | `colors/ColorConversions.h` |
| 10.3 | Create `HsbColor.h` | `colors/HsbColor.h` |
| 10.4 | Create `HslColor.h` | `colors/HslColor.h` |
| 10.5 | Create `HtmlColorData.h` with name tables | `colors/HtmlColorData.h` |
| 10.6 | Create `HtmlColor.h` with parse/format | `colors/HtmlColor.h` |
| 10.7 | Smoke test: round-trip conversions | `examples-virtual/color-models-test/main.cpp` |

### Phase 11 — Topology & Mosaic

**Prerequisites:** Phase 1 (`IPixelBus`/`PixelBus` exist)

| Step | Task | Files |
|------|------|-------|
| 11.1 | Create `PanelLayout.h` | `topologies/PanelLayout.h` |
| 11.2 | Create `PanelTopology.h` | `topologies/PanelTopology.h` |
| 11.3 | Create `RingTopology.h` | `topologies/RingTopology.h` |
| 11.4 | Create `TiledTopology.h` | `topologies/TiledTopology.h` |
| 11.5 | Create `MosaicBus.h` | `buses/MosaicBus.h` |
| 11.6 | Smoke test: single-bus tiled topology | `examples-virtual/tiled-topology-test/main.cpp` |
| 11.7 | Smoke test: multi-bus mosaic | `examples-virtual/mosaic-bus-test/main.cpp` |

---

## 6. Migration from Original API

| Original | Rewrite | Notes |
|----------|---------|-------|
| `RgbColor::LinearBlend(a, b, p)` | `Color::linearBlend(a, b, p)` | Operates on all 5 channels |
| `RgbColor::BilinearBlend(...)` | `Color::bilinearBlend(...)` | Same |
| `RgbColor::Dim(ratio)` | `color.dim(ratio)` | Returns `Color` |
| `RgbColor::Brighten(ratio)` | `color.brighten(ratio)` | Returns `Color` |
| `RgbColor::Darken(delta)` | `color.darken(delta)` | Mutating |
| `RgbColor::Lighten(delta)` | `color.lighten(delta)` | Mutating |
| `HsbColor(const RgbColor&)` | `HsbColor(const Color&)` | Uses R,G,B channels |
| `RgbColor(const HsbColor&)` | `hsb.toColor()` | Returns `Color` |
| `HslColor::LinearBlend<NeoHueBlendShortestDistance>(...)` | `HslColor::linearBlend(a, b, p, HueBlendMode::ShortestDistance)` | Enum replaces template param |
| `HtmlColor::Parse<HtmlColorNames>(str)` | `htmlColor.parse(str, HtmlColorNameSet::Full)` | Enum replaces template param |
| `NeoTopology<T>(w, h)` | `PanelTopology(w, h, layout)` | Runtime layout param |
| `NeoMosaic<T>(pw, ph, tw, th)` | `TiledTopology({...config...})` | Config struct |
| `topo.Map(x, y)` | `topo.map(x, y)` | Same semantic |
| `topo.MapProbe(x, y)` → sentinel | `topo.mapProbe(x, y)` → `std::optional` | Safer |
| N/A (not possible) | `MosaicBus(panels, config)` | Multi-bus aggregation (new) |

---

## 7. What Changes from the Original (Summary)

| Aspect | Original | Rewrite |
|--------|----------|---------|
| Blend methods | Duplicated across 7 color classes | Single implementation on `Color` |
| HSB/HSL blend strategy | Template parameter `T_NEOHUEBLEND` | `HueBlendMode` enum parameter |
| Color model storage | Separate types used in pixel buffers | Conversion-only types, pixel buffer is always `Color` |
| HtmlColor name tables | PROGMEM arrays with `pgm_read_*` | `constexpr` arrays (RP2040 flash is memory-mapped) |
| Topology layouts | 16 template classes | `PanelLayout` enum + `mapLayout()` constexpr function |
| `NeoTopology<T>` | Template class | `PanelTopology` runtime class |
| `NeoTiles` + `NeoMosaic` | Two separate template classes | `TiledTopology` with `mosaicRotation` flag |
| Multi-bus aggregation | Not supported | `MosaicBus` implements `IPixelBus` over multiple child buses |
| Out-of-bounds detection | Sentinel value (count) | `std::optional<uint16_t>` |
| Tile preferences | Nested `typedef` on layout classes | `tilePreferredLayout()` constexpr lookup function |
