# Virtual-Only Migration Plan (with Path Simplification)

Status: completed
Date: 2026-02-25

## Current Progress Snapshot

Completed:
- Flat include compatibility layer added under `src/{colors,protocols,transports,buses,topologies,factory,core}`.
- `VirtualNeoPixelBus.h` includes switched to flat roots.
- Internal/test includes moved off `virtual/...` roots.
- Implementations relocated into flat roots; temporary legacy forwarders have been removed.
- Top-level wrapper headers no longer include `original/*` directly.
- `src/original/**` deleted.
- Build no longer references `src/original/internal/methods/platform/rp2040/NeoRp2040PioMonoProgram.cpp`.
- Internal tests/examples switched to canonical `NeoPixelBus.h` include.
- Legacy top-level compatibility wrappers (`NeoPixelAnimator.h`, `NeoPixelBrightnessBus.h`, `NeoPixelBusLg.h`, `NeoPixelSegmentBus.h`) removed.
- Migration notes added at `docs/virtual-only-migration-notes.md`.

Previously outstanding blocker now resolved:
- `src/transports/esp32/Esp32I2sTransport.h` now inlines the required I2S/DMA implementation and no longer includes `Esp32_i2s.h`.

## Goal

Remove all legacy/original code and make virtual the only implementation surface, while simplifying include paths and module boundaries.

## Scope

In scope:
- Remove `src/original/**` (completed).
- Stop exposing legacy top-level wrappers (`src/NeoPixelBus.h`, `src/NeoPixelAnimator.h`, etc.) that only include `original/...`.
- Flatten public include paths so consumer code can use shorter, stable paths.

Out of scope for initial cut:
- Feature expansion not already present in canonical `src/{colors,protocols,transports,buses,topologies,factory,core}/` roots.
- Behavioral changes beyond parity and include-path normalization.

---

## Baseline & Cutline

Before code movement:
1. Tag/branch current known-good point (already started on `features/only-virtual-now`).
2. Record passing gates:
   - `pio test -e native-test --filter contracts/test_protocol_transport_contract_matrix_compile`
   - `pio test -e native-test --filter shaders/test_color_domain_section1`
3. Lock cutline: all new work targets virtual only.

---

## Current Dependency Findings

Observed from workspace search:
- Test and virtual code predominantly includes `virtual/...` paths.
- Top-level umbrella remains:
  - `src/NeoPixelBus.h`
- Legacy wrappers removed:
  - `src/NeoPixelBrightnessBus.h` (removed)
  - `src/NeoPixelBusLg.h` (removed)
  - `src/NeoPixelSegmentBus.h` (removed)
  - `src/NeoPixelAnimator.h` (removed)
- ESP32 I2S transport legacy include blocker is resolved by inlining the required implementation into `src/transports/esp32/Esp32I2sTransport.h`.

Remaining blockers are now limited to ongoing broad board-smoke validation.

---

## Target Simplified Layout

Target public structure under `src/`:

- `src/colors/`
- `src/protocols/`
- `src/transports/`
- `src/buses/`
- `src/topologies/`
- `src/factory/`
- `src/core/` (for shared non-domain items like `IPixelBus`, `ResourceHandle`, etc.)

Internal-only details:
- `src/**/detail/` retained where needed.

Compatibility bridge during migration (completed):
- Temporary legacy forwarding headers were used during transition and then removed.
- Canonical includes now live only under flattened folders.

---

## Compatibility Map (High-Level)

| Current Include Root | Target Include Root |
|---|---|
| `virtual/colors/*` | `colors/*` |
| `virtual/protocols/*` | `protocols/*` |
| `virtual/transports/*` | `transports/*` |
| `virtual/buses/*` | `buses/*` |
| `virtual/topologies/*` | `topologies/*` |
| `virtual/factory/*` | `factory/*` |
| `virtual/IPixelBus.h` | `core/IPixelBus.h` |
| `virtual/ResourceHandle.h` | `core/ResourceHandle.h` |

Umbrella header policy:
- Keep `src/VirtualNeoPixelBus.h` during transition.
- Final canonical umbrella should become `src/NeoPixelBus.h` (virtual implementation), not a legacy wrapper.

---

## PR-Chunk Execution Plan

## PR-1: Introduce Flat Include Surface (Completed)

- Add flat-path forwarding headers (e.g., `src/colors/Color.h` -> includes `../virtual/colors/Color.h`).
- Add `src/core/*` forwarders for shared primitives.
- Update `src/VirtualNeoPixelBus.h` to include flat paths (still transitively reaching existing virtual files).
- Keep all existing `virtual/...` includes valid.

Exit criteria:
- Existing tests pass with no behavior change.

## PR-2: Migrate Internal Includes + Tests to Flat Paths (Completed)

- Rewrite includes in source/tests/docs to flat paths.
- Keep temporary `virtual/...` forwarders only during transition.

Exit criteria:
- Native tests pass.
- No direct `#include "virtual/..."` remains.

## PR-3: Move Implementations from `virtual/` into Flat Folders (Completed)

- Physically move files from legacy virtual roots into `src/{colors,protocols,...}`.
- Use thin forwarders only as an interim compatibility step.

Exit criteria:
- All tests compile and pass.
- Canonical headers resolve correctly with no forwarders required.

## PR-4: Remove Legacy `original` Dependencies in Virtual Paths

- Replace remaining direct includes of `src/original/**` from virtual-derived code (notably ESP32 I2S internals).
- Vendor minimal required implementation into `src/transports/detail/...` or refactor transport to pure virtual-owned implementation.

Exit criteria:
- `grep` shows no `#include` references to `original/` outside legacy wrappers planned for deletion.

## PR-5: Delete Legacy Wrappers + `src/original/**`

- Remove legacy top-level headers that only forward to original.
- Delete `src/original/**`.
- Replace `src/NeoPixelBus.h` with virtual-only umbrella (or alias to `VirtualNeoPixelBus.h` then rename).

Exit criteria:
- No source references to `original/` remain.
- Native and board smoke gates pass.

## PR-6: Cleanup + Migration Notes (Completed)

- Remove temporary legacy forwarders once consumers are updated (completed).
- Add migration guide with include replacements and API notes.
- Update docs/specs to canonical path set.

Exit criteria:
- Single canonical path style in docs/tests/source.

---

## Validation Gates Per PR

Required each phase:
- `pio test -e native-test --filter contracts/test_protocol_transport_contract_matrix_compile`
- `pio test -e native-test --filter shaders/test_color_domain_section1`

Required before deleting legacy:
- `pio test -e native-test`
- Board compile smoke:
  - RP2040 representative target
  - One ESP32 representative target

---

## Deletion Checklist (Final)

Delete only when all pass:
- `src/original/**`
- Legacy wrapper headers that only include `original/...`
- Temporary legacy forwarders (completed)

---

## Risks & Mitigations

1. Include breakage during path transition
- Mitigation: staged rewrites with transitional forwarders (already retired).

2. Hidden legacy dependency in platform transports
- Mitigation: explicit grep gate for `original/` include references before deletion.

3. Consumer include path breakage
- Mitigation: document migration table and keep transitional forwarders for one release window (completed).

4. Build noise from broad move-only changes
- Mitigation: split by PR chunk and avoid behavior edits in same commits.
