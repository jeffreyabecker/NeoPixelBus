# Pallets Implementation Backlog

Source design: [../information/pallets-design.md](../information/pallets-design.md)

## Phase 1 — Core MVP (utility-first)

- [ ] Add core palette value types in `src/colors/`:
  - `PaletteStop<TColor>`
  - `GradientPalette<TColor>`
  - `PaletteSampleOptions`
  - `PaletteBlendMode` / `PaletteWrapMode`
- [ ] Add sampling utilities:
  - `mapPositionToPaletteIndex(pixelIndex, pixelCount, wrapMode)`
  - `samplePalette(...)` with `Nearest` + `Linear` behavior
- [ ] Implement compact binary decoder/encoder (`LP` format) in `src/colors/detail/`:
  - `magic`, `lengthBytes`, `version`, `flags`, `stopCount`, `stops`, `crc16`
  - bitfield parsing for `componentBytes` + `channelCount`
- [ ] Keep parse/decode path constexpr-friendly where practical (C++17 constraints, fixed-capacity outputs)
- [ ] Provide utility-first public entry points in `src/colors/Colors.h` / related public surface

## Phase 1 — Tests

- [ ] Add `test/colors/test_palette_sampling/`:
  - interpolation correctness
  - wrap vs clamp behavior
  - edge indices `0` and `255`
  - stop-boundary exactness
- [ ] Add `test/colors/test_palette_binary_codec/`:
  - valid payload round-trip
  - invalid header/version/flags cases
  - length mismatch and checksum mismatch
  - stop ordering validation
- [ ] Add compile-oriented checks ensuring palette headers do not require protocol/transport includes

## Phase 2 — Registry + Convenience

- [ ] Add `PaletteId` + `PaletteDescriptor` + `PaletteRegistry<TColor>`
- [ ] Add built-in gradient palette table and descriptor enumeration
- [ ] Add convenience helpers for common external-call usage patterns
- [ ] Add `test/colors/test_palette_registry/` for id/name lookup + enumeration + duplicate handling

## Phase 3 — Dynamic Palette Utilities

- [ ] Add dynamic palette generator utilities (random smooth / random cycle)
- [ ] Add explicit transition helper between two palettes (duration + progress input)
- [ ] Add deterministic test coverage for generator/transition behavior

## Acceptance Criteria

- [ ] Binary codec round-trips `GradientPalette` deterministically
- [ ] Parsing failures return stable `errorCode` values (no silent fallback)
- [ ] Utility API remains independent of shader/protocol/transport seams
- [ ] Native tests pass for all new palette test suites
