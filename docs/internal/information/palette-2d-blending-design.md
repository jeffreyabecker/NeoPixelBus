# Palette 2D Blending Extensions (Design Draft)

Status: proposed  
Scope: additive API design for 2D palette sampling/compositing on top of existing 1D palette utilities  
Reference source: `C:\ode\WLED` (`wled00/FX.h`, `wled00/FX_2Dfcn.cpp`, `wled00/FX.cpp`, `wled00/colorTools.hpp`)

---

## 1) Intent

Define a small, C++17-friendly API extension that makes 2D palette use first-class without breaking current 1D sampling patterns.

Goals:

- Keep protocol/transport seams unchanged.
- Reuse existing palette blend/wrap/sampling strategy model.
- Prefer index iterator/sentinel composition over new sampling entry points.
- Support matrix/grid callers and arbitrary XY-position callers.
- Keep runtime overhead low and deterministic.

Non-goals (phase 1):

- No new protocol features.
- No mandatory 2D palette asset format.
- No dynamic allocation requirement.

---

## 2) WLED observations to carry forward

WLED’s 2D effects generally compose color in three stages:

1. Build a palette index from spatial context (`x`, `y`, radial/noise/time).
2. Sample a 1D palette (`ColorFromPalette(...)`).
3. Blend in pixel space (`blendPixelColorXY`, additive writes, blur passes, Wu/subpixel distribution).

Key takeaway:

- Practical 2D behavior is mostly **index-mapping + spatial compositing**.
- This maps well to LumaWave’s current utility-first palette architecture.

---

## 3) Proposed API surface

All names below are draft names to validate call ergonomics before implementation.

### 3.1 Position mapping (XY -> palette index stream)

Prefer iterator/sentinel adapters as the public 2D mapping surface so callers can feed existing `samplePalette(...)` APIs directly.

Keep a scalar map helper as an internal primitive and test seam.

```cpp
namespace lw
{
    enum class Palette2DIndexMode
    {
        AxisX,
        AxisY,
        Diagonal,
        Radial,
        Custom
    };

    struct Palette2DMapOptions
    {
        Palette2DIndexMode mode{Palette2DIndexMode::AxisX};
        uint8_t offset{0};
        uint8_t scale{255};
    };

    constexpr uint8_t mapXYToPaletteIndex(size_t x,
                                          size_t y,
                                          size_t width,
                                          size_t height,
                                          Palette2DMapOptions options = {});

    class Palette2DIndexIterator;
    struct Palette2DIndexSentinel;

    constexpr Palette2DIndexIterator makePalette2DIndexBegin(size_t width,
                                                             size_t height,
                                                             Palette2DMapOptions options = {});

    constexpr Palette2DIndexSentinel makePalette2DIndexEnd();
}
```

### 3.2 Optional thin helper on index iterators

Primary path is direct `samplePalette(...)` invocation with 2D index iterators.

If a dedicated 2D-named helper is desired for readability, it should accept index iterator/sentinel pairs (not width/height) and forward to `samplePalette(...)`.

```cpp
namespace lw
{
    template <typename TBlend = BlendLinearContiguous<>,
              typename TPaletteLike,
              typename TIndexIt,
              typename TIndexSentinel,
              typename TOutputRange,
              typename = std::enable_if_t<
                  IsPaletteLike<TPaletteLike>::value &&
                  IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
    constexpr size_t samplePalette2D(const TPaletteLike &palette,
                                     TIndexIt indexBegin,
                                     TIndexSentinel indexEnd,
                                     TOutputRange &&output,
                                     PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {});
}
```

### 3.3 Explicit XY input iterator adapter

For topology-aware callers with custom coordinate streams, provide an XY->index iterator adapter.

```cpp
namespace lw
{
    struct XYu16
    {
        uint16_t x{0};
        uint16_t y{0};
    };

    template <typename TXYIt>
    class Palette2DIndexFromXYIterator;

    template <typename TXYIt>
    constexpr Palette2DIndexFromXYIterator<TXYIt> makePalette2DIndexFromXYBegin(TXYIt xyBegin,
                                                                                 TXYIt xyEnd,
                                                                                 size_t width,
                                                                                 size_t height,
                                                                                 Palette2DMapOptions options = {});

    constexpr Palette2DIndexSentinel makePalette2DIndexFromXYEnd();
}
```

### 3.4 Spatial compositing helpers (optional phase 1, required phase 2)

```cpp
namespace lw
{
    enum class Palette2DBlendOp
    {
        Replace,
        Alpha,
        Add,
        Max
    };

    template <typename TColor, typename TRangeDst, typename TRangeSrc>
    constexpr size_t composite2D(TRangeDst &&dst,
                                 const TRangeSrc &src,
                                 Palette2DBlendOp op,
                                 uint8_t amount = 255);
}
```

---

## 4) Example call sites

### Example A: Fill a matrix by X-gradient palette (iterator-first)

```cpp
#include <array>
#include <LumaWave.h>

using namespace lw;

std::array<Rgb8Color, 16 * 16> frame{};
Palette<Rgb8Color> pal(makeSunsetStops());

const auto idxBegin = makePalette2DIndexBegin(16,
                                              16,
                                              Palette2DMapOptions{Palette2DIndexMode::AxisX, 0, 255});
const auto idxEnd = makePalette2DIndexEnd();

samplePalette(pal,
              idxBegin,
              idxEnd,
              frame);
```

### Example B: Radial map, smooth blend, circular wrap

```cpp
PaletteSampleOptions<Rgb8Color> sampleOpts;
sampleOpts.wrapMode = PaletteWrapMode::Circular;

const auto idxBegin = makePalette2DIndexBegin(32,
                                              8,
                                              Palette2DMapOptions{Palette2DIndexMode::Radial, 24, 220});
const auto idxEnd = makePalette2DIndexEnd();

samplePalette<BlendSmoothstepContiguous<WrapCircular>>(pal,
                                                       idxBegin,
                                                       idxEnd,
                                                       frame,
                                                       sampleOpts);
```

### Example C: Topology-driven XY list

```cpp
std::array<XYu16, 64> coords = makePanelCoords();
std::array<Rgb8Color, 64> sampled{};

const auto idxBegin = makePalette2DIndexFromXYBegin(coords.begin(),
                                                    coords.end(),
                                                    panelWidth,
                                                    panelHeight,
                                                    Palette2DMapOptions{Palette2DIndexMode::Diagonal, 0, 255});
const auto idxEnd = makePalette2DIndexFromXYEnd();

samplePalette(pal,
              idxBegin,
              idxEnd,
              sampled);
```

### Example D: Sample + composite over existing frame

```cpp
std::array<Rgb8Color, 256> layer{};
std::array<Rgb8Color, 256> base = previousFrame();

const auto idxBegin = makePalette2DIndexBegin(16,
                                              16,
                                              Palette2DMapOptions{Palette2DIndexMode::AxisY, 0, 255});
const auto idxEnd = makePalette2DIndexEnd();

samplePalette(pal,
              idxBegin,
              idxEnd,
              layer);
composite2D<Rgb8Color>(base, layer, Palette2DBlendOp::Add, 160);
```

---

## 5) Compatibility with current API

- Existing `samplePalette(...)` overloads remain unchanged and are the primary execution path.
- 2D support is additive via index iterator adapters (XY -> index stream).
- `samplePalette2D(...)` remains optional sugar and should stay iterator/sentinel-based.
- Existing `PaletteBlendMode` and `PaletteWrapMode` semantics are reused where possible.

---

## 6) Phase plan

Phase 1 (minimal and useful):

- `Palette2DMapOptions` + `mapXYToPaletteIndex(...)` primitive.
- grid index iterator/sentinel adapter.
- XY-source index iterator/sentinel adapter.
- optional iterator-based `samplePalette2D(...)` forwarding helper.
- focused native tests for mapper correctness and output count behavior.

Phase 2 (WLED-like compositing ergonomics):

- `composite2D(...)` blend ops.
- optional row/col blur helpers in palette utility area (if needed by effects).

Phase 3 (if needed):

- true 2D palette fields (corner palette bilinear interpolation).
- time-aware mapper helpers (`x,y,t`).

---

## 7) Open decisions

- Whether `XYu16` should be library-provided or caller-provided (templated accessor approach).
- Whether compositing helpers live in `colors/palette/` or in a separate `colors/composite/` utility header.
- Whether to expose a callable custom mapper in phase 1 or keep enum-only and add callable mappers later.
