# NeoPixelBus RP2040 Refactor Plan (GNU++23)

## 1) Scope and Constraints

This refactor intentionally optimizes for simplification over compatibility.

- Target platform: **RP2040 only**.
- Language/standard target: **GNU++23** (or closest practical equivalent in Arduino RP2040 toolchain).
- API compatibility: **not required**.
- STL usage: **allowed**.
- Output contract: transport/output paths rely on **RGB types only**.
- Non-RGB color types remain as **utility/conversion helpers** only.

## 2) Problem Summary

Current architecture uses a large compile-time matrix:

- `T_COLOR_FEATURE` template hierarchy for channel order and pixel shape.
- `T_METHOD` template hierarchy for chipset/platform transport.
- Large alias surfaces (`typedef`/`using`) for near-duplicate combinations.

This increases:

- compile time and symbol surface,
- maintenance overhead,
- complexity of adding/changing pixel formats,
- cognitive load for users selecting types.

## 3) High-Level Target Architecture

### 3.1 Runtime Pixel Format Model

Replace most color feature template permutations with one descriptor-driven model.

Proposed core types:

```cpp
enum class PixelOrder : uint8_t { RGB, RBG, GRB, GBR, BRG, BGR };
enum class PixelLayout : uint8_t { RGB8, RGBW8, RGBWW8, RGBWWW8 };

struct PixelFormatDescriptor {
    PixelLayout layout;
    PixelOrder order;
    uint8_t pixelSizeBytes;
    std::array<uint8_t, 6> channelToByteOffset; // max channel count support
    uint8_t channelCount;
};
```

Design goals:

- `constexpr` descriptor tables for common formats.
- `consteval`/`constexpr` validation (unique channel mapping, size/channel consistency).
- Runtime lookup by compact enum/config value.

### 3.2 RGB Output Boundary

All transport packing writes from an RGB representation.

- Internal bus write path accepts `RgbColor` (or equivalent RGB struct).
- Utility colors (`Rgbw*`, `HslColor`, `HsbColor`, etc.) convert to RGB before pack.
- Transport does not require per-feature color object templates.

### 3.3 RP2040 Method Simplification

Keep RP2040 PIO/DMA core and collapse alias explosion into thin config wrappers.

- Core sender remains high-performance and allocation-aware.
- Replace many alias typedefs with:
  - small enum/config for speed/profile,
  - optional factory/registry,
  - minimal named aliases retained only where they provide real value.

## 4) GNU++23 Features to Use

Use modern language features where they reduce complexity without harming hot-path performance.

- `enum class` for strongly-typed format/method choices.
- `std::array` for fixed descriptor data.
- `std::span<uint8_t>` for safe buffer views.
- `std::expected<T, E>` for initialization/configuration errors.
- `constexpr` tables and functions for format metadata.
- `[[nodiscard]]`, `[[likely]]`, `[[unlikely]]` where meaningful.
- `std::variant` only at configuration boundaries (not in pixel hot loops).

Avoid in hot paths:

- dynamic polymorphism (`virtual`) in per-pixel loops,
- heavy type-erasure in update/show loop.

## 5) Phased Migration Plan

### Phase 0 — Freeze and Baseline

- Lock scope to RP2040-only in top-level method includes.
- Capture baseline metrics:
  - build time,
  - binary size,
  - frame throughput and update latency,
  - memory use (buffer + DMA overhead).

Deliverable:

- Baseline report in `docs/` with test setup and numbers.

### Phase 1 — Introduce Pixel Descriptor Layer

- Add a new internal header for descriptor model (e.g., `src/internal/pixel/PixelFormat.h`).
- Implement `constexpr` descriptor table + validation.
- Add pack/unpack helpers using descriptor offsets.

Deliverable:

- Unit-level compile checks for descriptor validity.
- Adapter functions available without changing public bus API yet.

### Phase 2 — RGB-Only Render Path

- Add a central `packPixelRgb(...)` path.
- Route `SetPixelColor`/buffer writes through RGB packer.
- Keep conversion constructors/helpers for utility color types.

Deliverable:

- Existing RGB workflows run through descriptor path.
- Utility color types still usable via conversion to RGB.

### Phase 3 — Replace Feature Template Matrix

- Deprecate/remove `Neo*Feature` class explosion where redundant.
- Replace with descriptor selected at construction/config time.
- Keep only minimal compile-time constructs needed for performance-critical internals.

Deliverable:

- Drastic reduction in `src/internal/features/*` surface.
- Equivalent pixel order support via descriptors.

### Phase 4 — RP2040 Method API Cleanup

- Refactor RP2040 sender setup around compact config:
  - speed profile,
  - PIO instance/channel choice,
  - invert flag.
- Replace long alias lists with table-driven/profile-driven creation.

Deliverable:

- RP2040 method configuration is explicit and compact.
- Alias count significantly reduced.

### Phase 5 — Remove Non-RP2040 Codepaths

- Strip non-RP2040 includes/branches in method aggregators.
- Remove dead platform-specific files from active build paths.

Deliverable:

- RP2040-only build graph.
- Smaller include and symbol surface.

### Phase 6 — Simplify `NeoPixelBus` Core

- Reduce template arguments in `NeoPixelBus` where possible.
- Move from feature type dependency to descriptor/config dependency.
- Keep API surface minimal and practical for RP2040 use.

Deliverable:

- Cleaner bus interface with fewer template parameters.

### Phase 7 — Example and Documentation Migration

- Update examples to new API/config model.
- Add migration notes and rationale to `ReadMe.md`.

Deliverable:

- Working RP2040 examples using new descriptor/config model.

### Phase 8 — Validation and Performance Gate

- Verify output correctness (channel order, brightness behavior).
- Re-run baseline metrics and compare.
- Confirm no regressions in target workloads.

Deliverable:

- Final validation report in `docs/`.

## 6) Proposed File/Module Changes

Potential new modules:

- `src/internal/pixel/PixelFormat.h`
- `src/internal/pixel/PixelPacker.h`
- `src/internal/pixel/PixelConversion.h`

Likely high-impact edits:

- `src/NeoPixelBus.h`
- `src/internal/NeoColorFeatures.h`
- `src/internal/NeoMethods.h`
- `src/internal/methods/platform/rp2040/NeoRp2040x4Method.h`
- `src/internal/XMethods.h`

Likely reduced/deleted surfaces:

- large subsets of `src/internal/features/*`
- non-RP2040 platform method includes/aliases

## 7) Performance and Safety Notes

Hot path principles:

- Precompute descriptor offsets once per strip/config.
- Use direct indexed stores for channel writes.
- Avoid branching per channel where possible.

Safety principles:

- Validate descriptor data at compile time when possible.
- Validate runtime-selected format and return explicit errors.
- Keep DMA/buffer ownership and swap semantics deterministic.

## 8) Risks and Mitigations

Risk: runtime descriptor indirection slows pixel writes.

- Mitigation: cache resolved offsets and specialized function pointers per configured format.

Risk: broad refactor may break examples.

- Mitigation: migrate examples incrementally and keep a small compatibility shim only during transition.

Risk: toolchain support for full GNU++23 varies.

- Mitigation: gate advanced features behind feature-test macros and provide fallback (`expected` alternative) where required.

## 9) Acceptance Criteria

Functional:

- RP2040-only library builds and runs examples.
- RGB output path is the only transport boundary.
- Utility color types remain available through conversion.

Structural:

- Significant reduction in feature/method alias surface.
- Simplified `NeoPixelBus` configuration model.

Performance:

- Throughput/latency within target (equal or better than baseline in core workloads).
- No unacceptable RAM increase.

## 10) Suggested Implementation Order

1. Phase 0 baseline
2. Phase 1 descriptor model
3. Phase 2 RGB-only pack path
4. Phase 5 remove non-RP2040 includes/branches (early simplification)
5. Phase 4 RP2040 method cleanup
6. Phase 3 feature matrix collapse
7. Phases 6–8 finalize, migrate examples, validate

---

This document is the implementation blueprint for the refactor and should be updated as design decisions are finalized during execution.
