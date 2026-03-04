# Palette Design (WLED-Informed)

Status: proposed  
Scope: internal design for upcoming palette/pallet support in NpbNext  
Reference source: `C:\ode\WLED` (`wled00/FX.h`, `wled00/FX_fcn.cpp`, `wled00/json.cpp`, `wled00/palettes.h`, `default-palettes.json`)

---

## 1) Intent

Design a palette system ("pallets" in user wording) that captures WLED-level usefulness while fitting NpbNext virtual-first architecture and C++17 constraints.

Primary goals:

- Keep palette logic independent from protocol/transport seams.
- Preserve deterministic behavior and low overhead on MCUs.
- Support built-in, generated, and user/custom palettes.
- Keep initial scope focused on runtime palette behavior (no import/export or serialization requirement).

---

## 2) WLED behaviors to carry forward

Observed in WLED runtime:

- Palette is selected by `uint8_t` id (`seg.palette` / JSON `"pal"`).
- Mixed palette categories share one id space:
  - Reserved/builtin behaviors (`Default`, `Random`, `Color 1`, `Color 1&2`, etc.).
  - FastLED fixed palettes (`Party`, `Cloud`, `Lava`, ...).
  - PROGMEM gradients (`gGradientPalettes[...]`).
  - Custom palettes (high id range in WLED).
- Sampling behavior includes:
  - Mapping index from pixel position to `[0..255]`.
  - Optional wrap/non-wrap behavior.
  - Blend mode (`LINEARBLEND` or `NOBLEND`, globally controlled in WLED).
- Dynamic/random palettes are first-class and support time-based transitions.
- API exposure includes names list and expanded palette samples (`/json/pal`, `/json/palx`).

Useful lessons:

- Keep a stable, compact palette id.
- Separate **selection** from **sampling**.
- Treat dynamic/random palettes as generators, not static data blobs.

---

## 3) NpbNext architectural fit

### 3.1 Current seam alignment

- `IShader<TColor>` remains the transform seam (`apply(span<TColor>)`).
- `IPixelBus`, `IProtocol`, and `ITransport` stay palette-agnostic.
- Palette support belongs in `colors/` utility code (not protocol/transport).

### 3.2 Proposed new non-virtual core types

Add value types (no per-pixel virtual dispatch):

- `PaletteStop<TColor>`
  - `uint8_t index` (`0..255`)
  - `TColor color`
- `Palette<TColor>`
  - sorted stops (`lw::span<const PaletteStop<TColor>>` view in hot paths)
- `PaletteBlendMode`
  - `Linear`, `Nearest`
- wrap strategy type (for example clamp vs circular)
  - `Wrap`, `Clamp`
- `PaletteSampleOptions`
  - blend mode, wrap mode, brightness scale

Core helper API (constexpr/inline where possible):

- `samplePalette(const Palette<TColor>&, uint8_t index, PaletteSampleOptions)`
- `mapPositionToPaletteIndex<TWrap>(pixelIndex, pixelCount)`

### 3.3 Scope clarification: no registry layer

Current direction is utility-first without a palette registry in LumaWave.

- Keep palette data as caller-owned inputs to utility functions.
- Keep identifiers/lookup concerns external to LumaWave core palette utilities.
- Revisit a registry only if a concrete runtime need emerges.

---

## 4) Usage model

### 4.1 Keep usage explicit

Do not hide position/time mapping inside protocol or transport.

Use one of these integration paths:

1. Effect/topology code computes palette index per pixel and writes colors.
2. External callers use palette utility helpers directly to map/sample colors.

### 4.2 Utility-first integration

Treat palettes as standalone utility methods in `colors/`:

- Callers provide index/mapping context.
- Utilities return sampled colors from gradient definitions.
- Blending policy remains caller-controlled via `PaletteSampleOptions`.

This keeps palette logic reusable across effects, buses, and external tooling without forcing shader integration.

### 4.3 Dynamic palette architecture options

For dynamic behavior (random smooth / random cycle), two implementation directions were considered.

#### Option A: virtual accessors on `Palette`

Example direction:

- make `Palette<TColor>` polymorphic,
- add virtual sampling/access methods,
- implement static/dynamic palette subclasses.

Pros:

- Single runtime abstraction for callers.
- Straightforward type-erased ownership patterns.

Cons:

- Introduces vtable and runtime dispatch into a currently value/utility-first path.
- Weakens constexpr friendliness and compile-time evaluation potential.
- Adds ABI/surface complexity to a core type currently intended as a lightweight view over stops.
- Encourages heap/lifetime coupling for polymorphic instances in embedded contexts.

#### Option B: templated helpers on `TPaletteLike`

Example direction:

- keep `Palette<TColor>` as current stop-based value type,
- add helper templates accepting any `TPaletteLike` with a small required API,
- provide adapters for static palette views and dynamic palette state objects.

Pros:

- Preserves zero-overhead path for static palettes.
- Keeps constexpr-friendly design for compile-time-capable call sites.
- Avoids forcing one inheritance hierarchy on callers.
- Works well with utility-first architecture and explicit caller-managed state.

Cons:

- Requires documenting a small concept-like contract for `TPaletteLike`.
- Template errors can be less friendly without careful static assertions.

#### Recommendation

Prefer **Option B** (`TPaletteLike` helper templates) and keep `Palette<TColor>` non-virtual.

Rationale:

- aligns with LumaWave utility-first goals,
- avoids unnecessary virtual overhead in hot paths,
- keeps static and dynamic palette usage composable without introducing a registry or polymorphic base requirement.

#### WLED usage alignment

WLED usage is close to the target usage here:

- Effects sample colors from a current palette using indexed lookup paths.
- Dynamic behavior (random smooth/cycle, transitions) is implemented by updating palette state over time, not by introducing polymorphic palette object families.
- Runtime control remains effect/segment/state driven, while palette sampling stays a tight utility path.

Implication for LumaWave:

- Keep `Palette<TColor>` as a lightweight stop-view data type.
- Model dynamic behavior as caller-managed state/adapters consumed by templated helper functions.
- Avoid virtual accessors on `Palette<TColor>` unless a future proven need requires runtime polymorphism.

---

## 5) Data model boundary

### 5.1 Internal canonical format

Use a compact gradient format:

- Variable stop count (`>=2`).
- Monotonic stop indices (`0..255`).
- Color stored in `TColor`.

### 5.2 Runtime ownership boundary

Serialization is dropped from this design scope and remains a consumer/application concern.

Core palette responsibilities remain:

- representing palette stops,
- validating monotonic stop order where required by sampling utilities,
- sampling by index and strategy policies.

If a consumer needs wire/storage formats, those should be designed in a separate consumer-owned module.

### 5.3 Validation and failure behavior (core palette module)

Sampling/path utilities must reject/guard invalid palette data when any of these holds:

- invalid stop ordering (equal/decreasing index)

Implementation note:

- Keep palette sampling helpers constexpr-friendly where practical under C++17 constraints.

---

## 6) Compatibility policy with WLED

### 6.1 What to match

- Indexed gradient sampling behavior.
- Reserved semantic palette behaviors (`Default`, random family, primary/secondary/tertiary convenience palettes).

### 6.2 What not to copy directly

- WLED global singleton state patterns.
- WLED custom-id encoding tricks (`255 - i + palettesCount`).
- Hard coupling to FastLED-specific palette container types in public API.

---

## 7) Proposed phase plan

### Phase 1 (MVP)

- Add core value types + sampler.
- Add built-in gradient table (small initial set).
- Add utility entry points for external callers/effects to map + sample palettes.
- Add native tests for:
  - interpolation correctness,
  - wrap/clamp behavior,
  - edge indices (`0`, `255`),
  - stop-boundary exactness.

### Phase 2

- Add lightweight convenience helpers for common external-call patterns.
- Add caller-oriented metadata helper patterns (without introducing a core registry abstraction).

### Phase 3

- Add dynamic palette generators (random smooth / random cycle / rainbow).
- Add transition blending between palettes with explicit duration.
- Add optional docs/examples showing WLED palette migration.

---

## 8) Test strategy

Add focused suites under `test/colors/` (native first):

- `test_palette_sampling`
- `test_palette_registry`

Contract checks (compile-first) should verify:

- palette types compile without Arduino.h inclusion,
- templated `TPaletteLike` helpers accept user-defined palette adapters without requiring inheritance or virtual methods.

---

## 10) Open decisions

- Dynamic palette generator modeling as strategy objects vs enum + settings is obviated by the `TPaletteLike` approach.
- Palette blend mode scope (global vs per-shader/per-segment) is out of scope for this document revision.
