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
- [x] Implement compact binary decoder/encoder (`LP` format) in `src/colors/palette/PaletteCodec.h`:
  - `magic`, `lengthBytes`, `version`, `flags`, `stopCount`, `stops`, `crc16`
  - bitfield parsing for `componentBytes` + `channelCount`
- [x] Keep parse/decode path constexpr-friendly where practical (C++17 constraints, fixed-capacity outputs)
- [x] Provide utility-first public entry points in `src/colors/Colors.h` / related public surface

## Phase 1 — Tests

- [x] Add `test/shaders/test_palette_utilities_section7/`:
  - interpolation correctness
  - wrap vs clamp behavior
  - edge indices `0` and `255`
  - stop-boundary exactness
- [x] Add `test/shaders/test_palette_binary_codec_section7/`:
  - valid payload round-trip
  - invalid header/version/flags cases
  - length mismatch and checksum mismatch
  - stop ordering validation
- [x] Add compile-oriented checks ensuring palette headers do not require protocol/transport includes (`test/contracts/test_palette_first_pass_compile/`)

### Temporary status (March 2026)

- `samplePalette` public APIs were intentionally removed from `src/colors/palette/Palette.h` pending rewrite.
- `test/shaders/test_palette_utilities_section7/` is temporarily reduced to `mapPositionToPaletteIndex` coverage only.
- Full sampling behavior tests will be restored when the replacement vectorized sampling API lands.

## Phase 2 Iterate on the design
- [ ] Evaluate extending `Palette` for dynamic behaviors (random smooth / random cycle) instead of separate generator utilities
  - [ ] Confirm API/ABI impact stays acceptable for utility-first usage
  - [ ] Confirm constexpr-friendly behavior is preserved for static palettes
  - [ ] Confirm deterministic behavior can be guaranteed with explicit seed/time inputs
  - [ ] Confirm unit-test coverage can validate both static and dynamic paths clearly

## Phase 3 — Convenience + Dynamic Utilities

- [ ] Add convenience helpers for common external-call usage patterns
- [ ] Add a parser method that returns a Palette<TColor> if there is an error return a palette with a black point at 0 and 255
- [ ] Add focused tests for convenience helper behavior
- [ ] Add explicit transition helper between two palettes (duration + progress input)
- [ ] Add deterministic test coverage for generator/transition behavior

## Acceptance Criteria

- [ ] Binary codec round-trips `Palette` deterministically
- [ ] Parsing failures return stable `errorCode` values (no silent fallback)
- [ ] Utility API remains independent of shader/protocol/transport seams
- [ ] Native tests pass for all new palette test suites
