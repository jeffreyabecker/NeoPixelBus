# API Gap Analysis: Original NeoPixelBus vs. Rewrite Architecture

Surveyed the full public API surface of `src/original/` and compared against
`docs/architecture-for-rewrite.md`. Items below are present in the original
codebase but absent or only partially covered in the rewrite architecture doc.

---

## Already Documented (no action needed)

- Core bus: `Begin`, `Show`, `CanShow`, `SetPixelColor`, `GetPixelColor`,
  `ClearTo`, `Pixels`, `PixelCount`, `PixelSize`, `PixelsSize`
- `NeoPixelBusLg` luminance/gamma shimming, `LuminanceShader`
- Color types (all 7 classes + their layout)
- Feature hierarchy (all byte/word feature variants, settings mixins)
- Method hierarchy (all one-wire + two-wire + serial)
- T_TWOWIRE / `IClockDataBus`
- Settings classes (TM1814, SM168x, TLC59711, `NeoNoSettings`)
- Gamma methods (`NeoGammaEquationMethod`, `CieLab`, `Table`, `Null`)
- Buffer ownership model
- `CalcTotalMilliAmpere` → `CurrentLimiterShader`

---

## Not Documented — Needs Revisiting

| # | Feature Area | Key Types / Methods | Notes |
|---|-------------|-------------------|-------|
| 1 | **Dirty flag protocol** | `IsDirty()`, `Dirty()`, `ResetDirty()` on bus, shaders, and DIBs | Architecture doc shows `_dirty` on PixelBus but doesn't formalize it as an interface contract shared with shaders/buffers |
| 2 | **Buffer manipulation ops** | `RotateLeft/Right(count, first, last)`, `ShiftLeft/Right(count, first, last)`, `SwapPixelColor(a, b)` | Entirely absent from the rewrite doc — these operate on the raw byte buffer in the original |
| 3 | **`NeoPixelBrightnessBus`** | Deprecated brightness-scaling subclass | Doc mentions `NeoPixelBusLg` replaces it, but doesn't explicitly note the deprecation or migration path |
| 4 | **`NeoPixelSegmentBus`** | `SetString(indexDigit, str, brightness)` for 7-segment displays | Out of rewrite scope per §Scope, but not called out in the exclusions list |
| 5 | **`NeoBuffer<T>` / `NeoBufferContext<T>`** | 2D RAM/PROGMEM pixel buffer, `Blt()`, `Render()` with shaders, `LayoutMapCallback` | Not mentioned at all in the rewrite — related to topologies/bitmaps |
| 6 | **`NeoDib<T>`** | Device-independent bitmap with typed color storage, `Render()` through shaders | Not mentioned — this is the original's equivalent of "color buffer + shader render" |
| 7 | **`NeoBitmapFile<T_FEATURE, T_FILE>`** | Reads .bmp files from filesystem, `Render()` with shader | Not mentioned |
| 8 | **`NeoVerticalSpriteSheet<T>`** | Sprite sheet, `Blt()` per sprite index | Not mentioned |
| 9 | **Shader protocol (original)** | `NeoShaderBase`, `NeoShaderNop<T>`, duck-typed `Apply(index, color)` | Architecture doc defines `ITransformColors` replacement but doesn't cross-reference the original shader protocol |
| 10 | **Animation system** | `NeoPixelAnimator`, `AnimUpdateCallback`, `AnimationParam`, `AnimationState`, `NeoEase` (25+ easing functions), time scaling | Out of scope per §Scope ("excludes animations"), but not explicitly listed |
| 11 | **HSL / HSB color models** | `HslColor`, `HsbColor`, hue-blend strategies (`NeoHueBlendShortestDistance` etc.) | Out of scope per §Scope ("excludes HSB/HSL"), but the new `Color` class has no conversion path noted |
| 12 | **`HtmlColor`** | `Parse()` / `ToString()` for named/hex CSS colors, `HtmlColorNames` / `HtmlShortColorNames` | Not mentioned at all |
| 13 | **`Rgb16Color`** (565 packed) | 16-bit 565 display color | Not mentioned — may be irrelevant for LED strips |
| 14 | **`RgbwwwColor`** (6-channel) | 6×uint8 — three independent white channels | Doc §7 says "6-byte/word features: Dropped" but doesn't mention the color type itself |
| 15 | **`SevenSegDigit`** | 9-segment color type for 7-seg displays | Listed as out of scope but not explicitly excluded |
| 16 | **Color utility methods** | `Dim()`, `Brighten()`, `Darken()`, `Lighten()`, `LinearBlend()`, `BilinearBlend()`, `CalculateBrightness()`, `CompareTo()`, `IsMonotone()`, `IsColorLess()` | The new `Color` class has **none** of these — only construction + indexed access |
| 17 | **Cross-type `GetPixelColor<T>()`** | Template version that reads from bus buffer in one color type and returns as another | Dropped implicitly (read from Color vector now), but worth noting |
| 18 | **`ClearTo(color, first, last)`** | Range-based fill | Only single-pixel and span-based access on new `IPixelBus` |
| 19 | **`SetPixelSettings` / `SetMethodSettings`** | Runtime settings change after construction | Doc says settings are constructor params — runtime mutation not addressed |
| 20 | **`NeoGammaDynamicTableMethod`** | Runtime-generated gamma table from callback | Not mentioned in gamma/shader coverage |
| 21 | **`NeoGammaInvertMethod<T>`** | Inverted gamma wrapper | Not mentioned |
| 22 | **`NeoBufferContext` implicit cast** | `operator NeoBufferContext<T_COLOR_FEATURE>()` on bus — enables Blt interop | Not relevant if buffers are dropped, but worth noting |
| 23 | **Topology system** | `NeoTopology`, `NeoRingTopology`, `NeoTiles`, `NeoMosaic`, 16 layout classes, `NeoTopologyHint` | Out of scope per §Scope, but not in exclusions list |
| 24 | **`NeoUtil`** | `Reverse8Bits()`, `RoundUp()`, `PrintBin()` | Utility functions not mentioned |
| 25 | **Parallel output (`X4`/`X8`/`X16`)** | Multi-channel parallel methods | Doc mentions them under methods but doesn't address how `IEmitPixels` handles multi-channel |
| 26 | **Quad/Octal SPI `Begin()` overloads** | `Begin(sck, dat0..dat3, ss)`, `Begin(sck, dat0..dat7, ss)` | Not addressed in `IEmitPixels::initialize()` |

---

## Recommended Triage

### Must Decide Before Implementation

- **#16 (Color utility methods)** — Does the new `Color` need `Dim`,
  `Brighten`, `LinearBlend`, etc., or are those shader-only concerns now?
- **#2 (Buffer manipulation)** — `RotateLeft/Right`, `ShiftLeft/Right`,
  `SwapPixelColor` are popular for animations. Do they move to `IPixelBus` or
  stay as external free functions?
- **#18 (`ClearTo` range)** — The span-based `setPixelColor(offset, span)` can
  do this, but it's less ergonomic than `ClearTo(color, first, last)`.
- **#25 (Parallel output)** — Does `IEmitPixels` need multi-channel awareness
  or is that a separate `IEmitPixelsParallel` interface?

### Explicitly Exclude (document the exclusion in §Scope)

- **#4** — `NeoPixelSegmentBus` / `SevenSegDigit`
- **#10** — Animation system (`NeoPixelAnimator`, `NeoEase`)
- **#15** — `SevenSegDigit` color type
- **#23** — Topology system (`NeoTopology`, `NeoTiles`, `NeoMosaic`,
  `NeoRingTopology`, layout classes)

### Defer to Later Phases

- **#5–#8** (Buffer/DIB/Bitmap/Sprite) — worth a future phase after core is
  stable
- **#11–#12** (HSL/HSB/HtmlColor) — conversion utilities, not critical for LED
  output
- **#13** (`Rgb16Color`) — niche; only relevant if driving 565 displays
- **#14** (`RgbwwwColor`) — 6-channel type; architecture already drops 6-byte
  features
- **#19** (Runtime settings mutation) — can add setter methods later if needed
- **#20–#21** (Dynamic gamma table, inverted gamma) — specialize later in
  shader work

### Likely Obsoleted by New Design

- **#3** (`NeoPixelBrightnessBus`) — replaced by `BrightnessShader`
- **#9** (Original shader protocol) — replaced by `ITransformColors`
- **#17** (Cross-type `GetPixelColor<T>()`) — unnecessary with unified `Color`
- **#22** (`NeoBufferContext` cast) — unnecessary if buffer system is redesigned
- **#24** (`NeoUtil`) — utility functions can be reimplemented as needed

---

## Original API Reference (Full Catalog)

### Core Bus Classes

#### `NeoPixelBus<T_COLOR_FEATURE, T_METHOD>`

**Constructors:**
- `(uint16_t countPixels, uint8_t pin)` — single-wire
- `(uint16_t countPixels, uint8_t pin, NeoBusChannel channel)` — with channel
- `(uint16_t countPixels, uint8_t pinClock, uint8_t pinData)` — two-wire
- `(uint16_t countPixels, uint8_t pinClock, uint8_t pinData, uint8_t pinLatch, uint8_t pinOutputEnable)` — four-wire
- `(uint16_t countPixels)` — no pin
- `(uint16_t countPixels, Stream* pixieStream)` — serial stream

**Lifecycle:**
- `void Begin()` / `Begin(sck, miso, mosi, ss)` / `Begin(sck, dat0..3, ss)` / `Begin(sck, dat0..7, ss)`
- `void Show(bool maintainBufferConsistency = true)`
- `bool CanShow() const`

**Dirty state:**
- `bool IsDirty() const` / `void Dirty()` / `void ResetDirty()`

**Pixel access:**
- `void SetPixelColor(uint16_t index, ColorObject color)`
- `ColorObject GetPixelColor(uint16_t index) const`
- `template<T> T GetPixelColor(uint16_t index) const` — cross-type
- `void ClearTo(ColorObject color)` / `ClearTo(color, first, last)`
- `void SwapPixelColor(uint16_t a, uint16_t b)`

**Buffer manipulation:**
- `RotateLeft(count)` / `RotateLeft(count, first, last)`
- `RotateRight(count)` / `RotateRight(count, first, last)`
- `ShiftLeft(count)` / `ShiftLeft(count, first, last)`
- `ShiftRight(count)` / `ShiftRight(count, first, last)`

**Info / buffer:**
- `uint8_t* Pixels()` / `size_t PixelsSize()` / `size_t PixelSize()` / `uint16_t PixelCount()`

**Settings:**
- `void SetPixelSettings(const SettingsObject&)`
- `void SetMethodSettings(const MethodSettingsObject&)`

**Power:**
- `uint32_t CalcTotalMilliAmpere(const CurrentSettings&)`

**Cast:**
- `operator NeoBufferContext<T_COLOR_FEATURE>()`

#### `NeoPixelBusLg<T_COLOR_FEATURE, T_METHOD, T_GAMMA>`

- `LuminanceShader Shader` — public member
- `void SetLuminance(uint8_t)` / `uint8_t GetLuminance()`
- Overrides: `SetPixelColor`, `ClearTo` (apply luminance + gamma)
- `void ApplyPostAdjustments()`

#### `NeoPixelBrightnessBus<T_COLOR_FEATURE, T_METHOD>` (DEPRECATED)

- `void SetBrightness(uint8_t)` / `uint8_t GetBrightness()`
- Overrides: `SetPixelColor`, `GetPixelColor`, `ClearTo`

#### `NeoPixelSegmentBus<T_COLOR_FEATURE, T_METHOD>`

- `void SetString(uint16_t indexDigit, const char* str, uint8_t brightness, uint8_t defaultBrightness = 0)`

### Color Types

All share: `operator[]`, `operator==/!=`, `CalculateBrightness()`, `Dim()`,
`Brighten()`, `Darken()`, `Lighten()`, `LinearBlend()`, `BilinearBlend()`,
`CompareTo()` / `Compare()`, `CalcTotalTenthMilliAmpere()`.

| Type | Channels | Element | Members |
|------|----------|---------|---------|
| `RgbColor` | 3 | uint8 | R, G, B |
| `Rgb16Color` | 3 | packed 565 | Color565 |
| `Rgb48Color` | 3 | uint16 | R, G, B |
| `RgbwColor` | 4 | uint8 | R, G, B, W, `IsMonotone()`, `IsColorLess()` |
| `Rgbw64Color` | 4 | uint16 | R, G, B, W |
| `RgbwwColor` | 5 | uint8 | R, G, B, WW, CW |
| `Rgbww80Color` | 5 | uint16 | R, G, B, WW, CW |
| `RgbwwwColor` | 6 | uint8 | R, G, B, W1, W2, W3 |
| `HslColor` | 3 | float | H, S, L |
| `HsbColor` | 3 | float | H, S, B |
| `HtmlColor` | 1 | uint32 | Color, `Parse()`, `ToString()` |
| `SevenSegDigit` | 9 | uint8 | A–G, Decimal, Custom |

**Hue blend strategies:** `NeoHueBlendShortestDistance`,
`NeoHueBlendLongestDistance`, `NeoHueBlendClockwiseDirection`,
`NeoHueBlendCounterClockwiseDirection`

### Gamma Correction

- `NeoGamma<T_METHOD>` — `static ColorObject Correct(const ColorObject&)`
- Methods: `NeoGammaEquationMethod`, `NeoGammaCieLabEquationMethod`,
  `NeoGammaTableMethod`, `NeoGammaDynamicTableMethod`, `NeoGammaNullMethod`,
  `NeoGammaInvertMethod<T>`

### Animation System

- `NeoPixelAnimator(countAnimations, timeScale)`
- `StartAnimation(index, duration, callback)` / `StopAnimation(index)` / `StopAll()`
- `IsAnimating()` / `IsAnimationActive(index)` / `NextAvailableAnimation(&index)`
- `RestartAnimation(index)` / `ChangeAnimationDuration(index, newDuration)`
- `UpdateAnimations()` — call in `loop()`
- `IsPaused()` / `Pause()` / `Resume()` / `getTimeScale()` / `setTimeScale()`
- `AnimationParam`: `float progress`, `uint16_t index`, `AnimationState state`
- `AnimationState`: `Started`, `Progress`, `Completed`
- Time scales: `NEO_MILLISECONDS` (1), `NEO_CENTISECONDS` (10),
  `NEO_DECISECONDS` (100), `NEO_SECONDS` (1000), `NEO_DECASECONDS` (10000)

**`NeoEase`** — 25+ easing functions (all `static float fn(float)`):
Linear, Quadratic{In,Out,InOut,Center}, Cubic{...}, Quartic{...},
Quintic{...}, Sinusoidal{...}, Exponential{...}, Circular{...}, Gamma, GammaCieLab

### Buffers & Image Types

- `NeoBufferContext<T_FEATURE>` — lightweight buffer reference (`Pixels`, `SizePixels`, `PixelCount()`)
- `NeoBuffer<T_BUFFER_METHOD>` — 2D pixel buffer, `SetPixelColor(x,y)`, `GetPixelColor(x,y)`, `Blt()`, `Render(shader)`
  - `NeoBufferMethod<T>` (RAM), `NeoBufferProgmemMethod<T>` (PROGMEM)
- `NeoDib<T_COLOR_OBJECT>` — device-independent bitmap, `Render(dest, shader)`
- `NeoBitmapFile<T_FEATURE, T_FILE>` — reads .bmp, `Render()` through shader
- `NeoVerticalSpriteSheet<T>` — sprite sheet, `Blt()` per sprite
- Shaders: `NeoShaderBase`, `NeoShaderNop<T>`, duck-typed `Apply(index, color)`
- `LayoutMapCallback` — `std::function<uint16_t(int16_t x, int16_t y)>`

### Topology System

- `NeoTopology<T_LAYOUT>` — `Map(x,y)`, `MapProbe(x,y)`, `getWidth()`, `getHeight()`
- `NeoRingTopology<T_LAYOUT>` — `Map(ring, pixel)`, `RingPixelShift()`, `RingPixelRotate()`
- `NeoTiles<T_MATRIX, T_TILE>` — `Map()`, `MapProbe()`, `TopologyHint()`
- `NeoMosaic<T_LAYOUT>` — mosaic tiling with auto-rotation
- 16 layout classes: `{Row,Column}Major{,Alternating}{,90,180,270}Layout`
- `NeoTopologyHint` enum: `FirstOnPanel`, `InPanel`, `LastOnPanel`, `OutOfBounds`

### Settings

- `NeoRgb[w[w[w]]]CurrentSettings` — per-channel current (1/10th mA)
- `NeoSevenSegCurrentSettings`
- `NeoTm1814Settings` — extends `NeoRgbwCurrentSettings` with `MinCurrent`/`MaxCurrent`/`LimitCurrent()`
- `NeoNoSettings`
- `NeoBusChannel` enum: `NeoBusChannel_0` through `NeoBusChannel_7`

### Utilities

- `NeoUtil::Reverse8Bits(uint8_t)`, `RoundUp(size_t, size_t)`, `PrintBin<T>(T)`
- `NEO_DIRTY = 0x80`, `PixelIndex_OutOfBounds = 0xFFFF`
