# Pallets Implementation Backlog

Source design: [../information/pallets-design.md](../information/pallets-design.md)

## Phase 1 — Core MVP (utility-first)

- [x] Add core palette value types in `src/colors/`:
  - `PaletteStop<TColor>`
  - `Palette<TColor>`
  - `PaletteSampleOptions`
  - blend/wrap strategy types
- [x] Add sampling utilities:
  - `mapPositionToPaletteIndex<TWrap>(pixelIndex, pixelCount)`
  - `samplePalette(...)` with `Nearest` + `Linear` behavior
- [x] Provide utility-first public entry points in `src/colors/Colors.h` / related public surface

Serialization note:

- Palette serialization/deserialization is intentionally consumer-owned and out of scope for core palette utilities.

## Phase 1 — Tests

- [x] Add `test/shaders/test_palette_utilities_section7/`:
  - interpolation correctness
  - wrap vs clamp behavior
  - edge indices `0` and `255`
  - stop-boundary exactness
- [x] Add compile-oriented checks ensuring palette headers do not require protocol/transport includes (`test/contracts/test_palette_first_pass_compile/`)

### Temporary status (March 2026)

- `samplePalette` public APIs were intentionally removed from `src/colors/palette/Palette.h` pending rewrite.
- `test/shaders/test_palette_utilities_section7/` is temporarily reduced to `mapPositionToPaletteIndex` coverage only.
- Full sampling behavior tests will be restored when the replacement vectorized sampling API lands.


## Phase 2 — Convenience + Dynamic Utilities

- [x] Add convenience helpers for common external-call usage patterns
- [x] Add focused tests for convenience helper behavior
- [x] Add explicit transition helper between two palettes (duration + progress input)
- [x] Add deterministic test coverage for generator/transition behavior

## Acceptance Criteria

- [ ] Utility API remains independent of shader/protocol/transport seams
- [ ] Native tests pass for all new palette test suites
