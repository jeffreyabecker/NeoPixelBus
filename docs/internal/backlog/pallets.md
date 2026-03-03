# Pallets Implementation Backlog

Source design: [../information/pallets-design.md](../information/pallets-design.md)

## Phase 1 — Core MVP (utility-first)

- [x] Add core palette value types in `src/colors/`:
  - `PaletteStop<TColor>`
  - `Palette<TColor>`
  - `PaletteSampleOptions`
  - `PaletteBlendMode` / `PaletteWrapMode`
- [x] Add sampling utilities:
  - `mapPositionToPaletteIndex(pixelIndex, pixelCount, wrapMode)`
  - `samplePalette(...)` with `Nearest` + `Linear` behavior
- [x] Implement compact binary decoder/encoder (`LP` format) in `src/colors/PaletteCodec.h`:
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

## Phase 2 — Convenience + Dynamic Utilities

- [ ] Add convenience helpers for common external-call usage patterns
- [ ] Add a parser method that returns a Palette<TColor> if there is an error return a palette with a black point at 0 and 255
- [ ] Add focused tests for convenience helper behavior
- [ ] Add dynamic palette generator utilities (random smooth / random cycle)
- [ ] Add explicit transition helper between two palettes (duration + progress input)
- [ ] Add deterministic test coverage for generator/transition behavior

## Acceptance Criteria

- [ ] Binary codec round-trips `Palette` deterministically
- [ ] Parsing failures return stable `errorCode` values (no silent fallback)
- [ ] Utility API remains independent of shader/protocol/transport seams
- [ ] Native tests pass for all new palette test suites
