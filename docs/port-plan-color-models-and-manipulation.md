# Port Plan: Alternative Color Models + Color Manipulation

Generated: 2026-02-25  
Scope requested:
- `HslColor` / `HsbColor`
- `LinearBlend` / `Darken` / `Lighten`

## 1) Legacy Baseline (Source of Truth)

Primary legacy implementations:
- `src/original/internal/colors/HslColor.h/.cpp`
- `src/original/internal/colors/HsbColor.h/.cpp`
- `src/original/internal/colors/RgbColor.h/.cpp`
- `src/original/internal/colors/RgbColorBase.h/.cpp`
- `src/original/internal/colors/NeoHueBlend.h`

Behavior to preserve:
1. `HslColor` and `HsbColor` are float domain (`0.0f..1.0f`) with RGB conversion constructors.
2. Hue blending supports policies (`ShortestDistance`, `LongestDistance`, `Clockwise`, `CounterClockwise`).
3. RGB color manipulation primitives:
   - `Darken(delta)` saturating subtract per channel
   - `Lighten(delta)` saturating add per channel
   - `LinearBlend(left, right, progress)` with:
     - `float progress` in `[0..1]`
     - `uint8_t progress` in `[0..255]`
4. HSL/HSB usage in examples relies on ergonomic construction patterns (hue-based effects, interpolation).

## 2) Current Virtual State

Current virtual surface:
- `src/virtual/colors/Color.h` provides `RgbBasedColor<N, TComponent>` and aliases (`Rgb8Color`, `Rgbw8Color`, `Rgbcw8Color`, `Rgb16Color`, etc.).
- No alternative color model types in virtual.
- No built-in `LinearBlend`/`Darken`/`Lighten` on virtual colors.
- Existing tests cover color domain + iterators only (`test/shaders/test_color_domain_section1`, `test/shaders/test_color_iterator_section2`).

Gap impact:
- Blocks porting examples that depend on hue-based generation and interpolation.
- Forces ad-hoc math in user code for common color operations.

## 3) Target API (Virtual)

### 3.1 New Color Model Types

Add:
- `npb::HslColor`
- `npb::HsbColor`

Placement:
- `src/virtual/colors/HslColor.h`
- `src/virtual/colors/HsbColor.h`

Design:
- Header-only, `constexpr` where practical.
- Members: `float H`, `float S`, `float L/B`.
- Constructors:
  - component constructor
  - default constructor
  - conversion constructors from RGB colors (`RgbBasedColor<3, uint8_t>` and `RgbBasedColor<3, uint16_t>`)
- Free conversion helpers (for clarity and extensibility):
  - `toRgb8(const HslColor&)`, `toRgb16(const HslColor&)`
  - `toRgb8(const HsbColor&)`, `toRgb16(const HsbColor&)`

### 3.2 Hue Blend Policies

Add:
- `src/virtual/colors/HueBlend.h`

Policies to port 1:1 from legacy semantics:
- `HueBlendShortestDistance`
- `HueBlendLongestDistance`
- `HueBlendClockwiseDirection`
- `HueBlendCounterClockwiseDirection`

### 3.3 Color Manipulation Primitives (External Algorithms)

Implement as free functions (not member methods) in a dedicated color-ops layer:
- `src/virtual/colors/ColorMath.h` (public API)
- `src/virtual/colors/detail/ColorMathBackend.h` (backend traits + dispatch)

Primary API shape:
- `template<typename TColor> void darken(TColor& color, typename TColor::ComponentType delta)`
- `template<typename TColor> void lighten(TColor& color, typename TColor::ComponentType delta)`
- `template<typename TColor> TColor linearBlend(const TColor& left, const TColor& right, float progress)`
- `template<typename TColor> TColor linearBlend(const TColor& left, const TColor& right, uint8_t progress)`

Semantics:
- Saturating arithmetic for darken/lighten.
- Preserve existing legacy integer rounding behavior for `uint8_t progress` blend path.
- Keep channel-agnostic operation for 3/4/5-channel aliases.
- Keep color storage classes (`RgbBasedColor`) as passive data types.

Acceleration model:
- Backend-selection via traits/`constexpr` dispatch (default scalar implementation).
- Optional platform-specialized backends can override blend/division/interpolation kernels.
- Public API remains stable regardless of backend in use.

### 3.4 HSL/HSB Blend APIs

Add in HSL/HSB types:
- `template<typename THueBlend> static HslColor LinearBlend(...)`
- `template<typename THueBlend> static HsbColor LinearBlend(...)`
- `template<typename THueBlend> static HslColor BilinearBlend(...)`
- `template<typename THueBlend> static HsbColor BilinearBlend(...)`

## 4) Compatibility Decisions

1. **Color channel count support**
   - Initial model conversion target: RGB (3-channel).
   - For RGBW/RGBCW pipelines, caller composes via existing expand mechanisms (or explicit channel fill policy).

2. **Float domain contract**
   - Keep H/S/L/B as normalized float range `[0..1]` without implicit clamping side effects.
   - Clamp only at RGB conversion boundaries.

3. **API shape**
   - Keep operation names/intent compatible with legacy (`linearBlend`, `darken`, `lighten`) to reduce example churn.
   - Keep manipulation external to color classes to allow backend swapping and hardware acceleration.
   - Prefer explicit helpers over implicit cross-type constructors to avoid surprising conversions in templated code.

4. **Backend strategy**
   - Scalar backend is normative reference behavior and always available.
   - Platform backends are optional and selected via compile-time traits/macros.
   - Backend path must be bit/parity equivalent to scalar path for covered inputs.

5. **No scope creep in this port**
   - Excludes animation (`NeoPixelAnimator`) and `NeoEase`.
   - Excludes `HtmlColor` and named-color parsing.

## 5) Implementation Phases

## Phase A — Foundations (low risk)

Deliverables:
- `HueBlend.h`
- `HslColor.h`
- `HsbColor.h`
- Public include wiring in `src/VirtualNeoPixelBus.h`

Validation:
- New compile-time and runtime tests for RGB<->HSL/HSB round-trip tolerance.

## Phase B — Manipulation Primitives + Backend Abstraction (medium risk)

Deliverables:
- Add `ColorMath.h` free-function API (`darken`, `lighten`, `linearBlend`).
- Add scalar reference backend and dispatch traits in `detail/ColorMathBackend.h`.
- Add backend override hook points for platform-accelerated implementations.

Validation:
- Unit tests for edge cases:
   - underflow/overflow saturation
   - blend endpoints (`progress = 0/1` and `0/255`)
   - midpoint rounding consistency
- Equivalence tests that compare accelerated backend results to scalar reference behavior.

## Phase C — Hue-Aware Interpolation (medium risk)

Deliverables:
- `HslColor::LinearBlend/BilinearBlend`
- `HsbColor::LinearBlend/BilinearBlend`
- Policy-driven hue interpolation using `HueBlend*` types

Validation:
- Tests around hue wrap boundaries (`~0.99 -> ~0.01`) for each policy.

## Phase D — Consumer Proving (medium risk)

Deliverables:
- Port one virtual sample sketch demonstrating:
  - hue-based generation
  - RGB blend transitions
  - darken/lighten operation

Validation:
- Native test pass + target board compile smoke (RP2040 and one ESP target).

## 6) Test Plan Additions

Test spec/document updates:
- Extend `docs/testing-spec-colors-shaders.md` with two new sections:
  - Section 5: Alternative Color Models (HSL/HSB)
  - Section 6: Color Manipulation Primitives

New test folders:
- `test/shaders/test_color_models_section5`
- `test/shaders/test_color_manipulation_section6`

Recommended test cases:
1. HSL->RGB and HSB->RGB canonical vectors (primaries, grayscale, edge hues).
2. RGB->HSL and RGB->HSB canonical vectors.
3. Round-trip tolerance checks for 8-bit and 16-bit paths.
4. Hue blend policy behavior near wrap-around.
5. `Darken` and `Lighten` saturation boundaries.
6. `LinearBlend` float and uint8 variants across 3/4/5 channels.

## 7) File Change Map

Planned new files:
- `src/virtual/colors/HueBlend.h`
- `src/virtual/colors/HslColor.h`
- `src/virtual/colors/HsbColor.h`
- `src/virtual/colors/ColorMath.h`
- `src/virtual/colors/detail/ColorMathBackend.h`
- `test/shaders/test_color_models_section5/test_main.cpp`
- `test/shaders/test_color_manipulation_section6/test_main.cpp`

Planned modified files:
- `src/VirtualNeoPixelBus.h`
- `test/shaders/README.md`
- `docs/testing-spec-colors-shaders.md`

## 8) Risks and Mitigations

1. **Behavior drift from legacy color math**
   - Mitigation: lift formulas directly; validate with vector-based parity tests.

2. **Rounding differences between float/int blend paths**
   - Mitigation: preserve legacy integer formula for uint8 progress path.

3. **Backend divergence (accelerated vs scalar)**
   - Mitigation: keep scalar as reference implementation and require parity tests for backend-specialized code paths.

4. **Template bloat / compile-time impact**
   - Mitigation: keep heavy conversion and backend logic in focused headers; avoid unnecessary overload fan-out.

5. **Ambiguity for >3 channel model conversion**
   - Mitigation: constrain initial conversion APIs to RGB targets and document policy.

## 9) Recommended Execution Order

1. Implement `HueBlend.h`, `HslColor.h`, `HsbColor.h` + tests (models only).
2. Implement external `darken`/`lighten`/`linearBlend` in `ColorMath` + scalar backend + tests.
3. Add one platform backend proof-of-concept (or stubbed trait path) and parity tests.
4. Integrate includes and docs/spec updates.
5. Port one representative hue-based example in virtual style.

## 10) Definition of Done

Done when all are true:
1. New model + manipulation tests pass under `pio test -e native-test`.
2. Existing shader/color tests remain green.
3. `VirtualNeoPixelBus.h` exports new APIs.
4. At least one virtual example demonstrates hue-based workflow without legacy headers.
5. Behavior parity is documented against legacy formulas for blend and saturation operations.
6. Backend-dispatch path is in place with scalar fallback and parity validation hooks.
