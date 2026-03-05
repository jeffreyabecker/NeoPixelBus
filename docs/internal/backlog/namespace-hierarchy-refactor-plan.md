# Namespace Hierarchy Refactor Plan

Status: planning only (no code migration executed yet).

## Goal

Reduce namespace flatness across `lw::...` so ownership is easier to reason about and later API-boundary decisions are easier to make.

Also consolidate light-driver abstractions into the transport domain so output-driver ownership is no longer split across `lights` and `transports`.

## Current Assumptions

- API stability is not a goal for this refactor.
- External compatibility is not a gating requirement for this phase.
- This phase does not decide what is "public" vs "internal".
- Namespace organization work must preserve behavior and seam contracts.
- `lights` is being retired as a source-folder/domain concept in favor of transport-centric organization.

## Constraints

- Keep C++17 compatibility.
- Prefer qualified namespace declarations when possible (for example `namespace lw::protocols::detail { ... }`).
- Do not move symbols into the C++ global namespace.
- Keep seam ownership/contract behavior intact:
- `IPixelBus`
- `IProtocol`
- `ITransport`
- `IShader`

Structural constraints for this scope:
- Move `ILightDriver` under `transports`.
- Move all concrete light-driver headers under `transports` (including platform-specific drivers).
- Delete `src/lights/Lights.h` as part of consolidation.
- Update umbrella include flow so `LumaWave/All.h` no longer depends on `LumaWave/Lights.h`.

## Namespace Principles

- Organize by domain first, then by depth (`<domain>` then `<domain>::detail`).
- Place helper traits, detection glue, and normalization internals under `...::detail`.
- Keep orchestration and user-meaningful domain symbols at domain level for now.
- Defer boundary labels; focus on clean hierarchy shape.

## Proposed `lw::` Namespace Set

Root:
- `lw`

Domain anchors:
- `lw::buses`
- `lw::colors`
- `lw::core`
- `lw::protocols`
- `lw::transports`
- `lw::factory`

Cross-domain helper layer:
- `lw::detail`

Domain detail layers (phase targets):
- `lw::factory::detail`
- `lw::protocols::detail`
- `lw::transports::detail`
- `lw::colors::detail`
- `lw::buses::detail` (candidate)

Protocol alias namespace decision for this phase:
- Do not introduce `lw::protocols::aliases` yet.
- Keep alias-family symbols in `lw::protocols`.
- Move only alias helper internals to `lw::protocols::detail` where useful.

Light-driver consolidation target:
- Light-driver abstractions live directly under transport namespaces (for example `lw::transports` and `lw::transports::rp2040`) instead of a separate `lights` domain.

Platform detail layers (later phases):
- `lw::transports::rp2040::detail`
- `lw::transports::esp32::detail`
- `lw::transports::esp8266::detail`

## Proposed Breakdown Under `lw::`

```text
lw::
	buses
		detail
	colors
		detail
		palette
			detail
	core
		detail (candidate; only if shared core internals emerge)
	factory
		detail
	protocols
		detail
	transports
		detail
		esp32
			detail
		esp8266
			detail
		rp2040
			detail
	detail (cross-domain shared helpers only)
```

Notes for this breakdown:
- This is a structure proposal, not a visibility/publicness decision.
- Domain roots (`buses`, `colors`, `core`, `factory`, `protocols`, `transports`) remain the primary organizational anchors.
- `detail` should be introduced only where there is real helper concentration, not preemptively everywhere.
- Light-driver concerns are treated as transport-adjacent and fold directly into existing transport/platform namespaces (no separate `light` sub-namespace).

## Candidate Organization Strategies

Strategy A (recommended): Domain + local detail
- Keep each subsystem under its existing domain namespace.
- Move helper-only entities to `<domain>::detail`.
- Lowest churn and easiest incremental review.

Strategy B: Two-level domain breakdown
- Introduce explicit subdomains before detail (deferred for protocols in this phase).
- Better semantic grouping in large domains.
- Higher rename overhead and more call-site changes.

Strategy C: File-structure first, namespace second
- First split files into domain helper headers, then apply namespace moves.
- Reduces single-PR complexity in dense headers.
- Slower path to visible namespace hierarchy improvements.

Strategy D: Domain consolidation (`lights` -> `transports`) (required for this scope)
- Move light-driver interface and implementations into transport hierarchy.
- Remove separate lights umbrella/header layer.
- Simplifies ownership model by grouping all physical-output drivers under transport-focused organization.
- Keep driver type names directly under their transport domain (for example `lw::transports::rp2040::RpPwmLightDriver`).

Recommendation: Apply Strategy D with Strategy A mechanics (incremental, domain + local detail), then selectively apply Strategy B in dense areas.

Phase decision:
- Keep protocol alias organization at `lw::protocols` + `lw::protocols::detail` only.

## Phase 1 Scope

Files:
- `src/buses/MakePixelBus.h`
- `src/protocols/ProtocolAliases.h`
- `src/lights/ILightDriver.h` -> `src/transports/ILightDriver.h`
- `src/lights/NilLightDriver.h` -> `src/transports/NilLightDriver.h`
- `src/lights/PrintLightDriver.h` -> `src/transports/PrintLightDriver.h`
- `src/lights/AnalogPwmLightDriver.h` -> `src/transports/AnalogPwmLightDriver.h`
- `src/lights/esp32/*` -> `src/transports/esp32/*`
- `src/lights/rp2040/*` -> `src/transports/rp2040/*`
- Delete `src/lights/Lights.h`
- Remove `src/LumaWave/Lights.h` (or replace with transitional include only if needed for migration)
- Update `src/LumaWave/All.h` to stop including `LumaWave/Lights.h`

Planned moves:
- `MakePixelBus.h`
- Move helper traits/functions to `lw::factory::detail`:
- `PixelBusProtocolSettingsHasTiming`
- `assignPixelBusProtocolTimingIfPresent`
- `DirectMakeBusCompatible`
- `DirectMakeBusShaderCompatible`
- `IsWs2812xProtocolAlias`
- Keep `makePixelBus(...)` overload surface in `lw::factory`.

- `ProtocolAliases.h`
- Move helper traits to `lw::protocols::detail`:
- `ResolveProtocolType` (with temporary forwarding alias if needed)
- `WrappedSpecHasNormalizeSettings`
- Keep alias family in `lw::protocols` for this phase.

- Light-driver consolidation plan:
- Move `ILightDriver` contract to the transport folder/namespace organization.
- Move all concrete light drivers to transport folder/namespace organization.
- Update include sites (for example `buses/LightBus.h`) to include from `transports/...`.
- Remove lights umbrella headers from aggregate include flow.

## Detailed Move Inventory

### Phase 1 namespace-helper moves

| Member | Current file | Current namespace | Target file | Target namespace |
| --- | --- | --- | --- | --- |
| `PixelBusProtocolSettingsHasTiming` | `src/buses/MakePixelBus.h` | `lw::factory` | `src/buses/MakePixelBus.h` | `lw::factory::detail` |
| `assignPixelBusProtocolTimingIfPresent` | `src/buses/MakePixelBus.h` | `lw::factory` | `src/buses/MakePixelBus.h` | `lw::factory::detail` |
| `DirectMakeBusCompatible` | `src/buses/MakePixelBus.h` | `lw::factory` | `src/buses/MakePixelBus.h` | `lw::factory::detail` |
| `DirectMakeBusShaderCompatible` | `src/buses/MakePixelBus.h` | `lw::factory` | `src/buses/MakePixelBus.h` | `lw::factory::detail` |
| `IsWs2812xProtocolAlias` | `src/buses/MakePixelBus.h` | `lw::factory` | `src/buses/MakePixelBus.h` | `lw::factory::detail` |
| `ResolveProtocolType` | `src/protocols/ProtocolAliases.h` | `lw::protocols` | `src/protocols/ProtocolAliases.h` | `lw::protocols::detail` |
| `Debug<TWrappedProtocolSpec>::WrappedSpecHasNormalizeSettings` | `src/protocols/ProtocolAliases.h` | `lw::protocols::Debug<...> (nested private)` | `src/protocols/ProtocolAliases.h` | `lw::protocols::detail` |

Notes:
- `makePixelBus(...)` overloads remain in `lw::factory`.
- Protocol alias family (`DotStar`, `Ws2812x`, `Tm1814`, etc.) remains in `lw::protocols` for this phase.

### Light-driver domain consolidation moves

| Member | Current file | Current namespace | Target file | Target namespace |
| --- | --- | --- | --- | --- |
| `ILightDriver<TColor>` | `src/lights/ILightDriver.h` | `lw` | `src/transports/ILightDriver.h` | `lw::transports` |
| `LightDriverSettingsBase` | `src/lights/ILightDriver.h` | `lw` | `src/transports/ILightDriver.h` | `lw::transports` |
| `LightDriverLikeImpl<TDriver,...>` | `src/lights/ILightDriver.h` | `lw` | `src/transports/ILightDriver.h` | `lw::transports` |
| `LightDriverLike<TDriver>` | `src/lights/ILightDriver.h` | `lw` | `src/transports/ILightDriver.h` | `lw::transports` |
| `SettingsConstructibleLightDriverLike<TDriver>` | `src/lights/ILightDriver.h` | `lw` | `src/transports/ILightDriver.h` | `lw::transports` |
| `NilLightDriverSettings` | `src/lights/NilLightDriver.h` | `lw` | `src/transports/NilLightDriver.h` | `lw::transports` |
| `NilLightDriver<TColor>` | `src/lights/NilLightDriver.h` | `lw` | `src/transports/NilLightDriver.h` | `lw::transports` |
| `PrintLightDriverSettingsT<TWritable>` | `src/lights/PrintLightDriver.h` | `lw` | `src/transports/PrintLightDriver.h` | `lw::transports` |
| `PrintLightDriverT<TColor,TWritable>` | `src/lights/PrintLightDriver.h` | `lw` | `src/transports/PrintLightDriver.h` | `lw::transports` |
| `PrintLightDriverSettings` | `src/lights/PrintLightDriver.h` | `lw` | `src/transports/PrintLightDriver.h` | `lw::transports` |
| `PrintLightDriver<TColor>` | `src/lights/PrintLightDriver.h` | `lw` | `src/transports/PrintLightDriver.h` | `lw::transports` |
| `AnalogPwmLightDriverSettings` | `src/lights/AnalogPwmLightDriver.h` | `lw` | `src/transports/esp8266/AnalogPwmLightDriver.h` | `lw::transports::esp8266` |
| `AnalogPwmLightDriver<TColor>` | `src/lights/AnalogPwmLightDriver.h` | `lw` | `src/transports/esp8266/AnalogPwmLightDriver.h` | `lw::transports::esp8266` |
| `Esp32LedcLightDriverSettings` | `src/lights/esp32/Esp32LedcLightDriver.h` | `lw` | `src/transports/esp32/Esp32LedcLightDriver.h` | `lw::transports::esp32` |
| `Esp32LedcLightDriver<TColor>` | `src/lights/esp32/Esp32LedcLightDriver.h` | `lw` | `src/transports/esp32/Esp32LedcLightDriver.h` | `lw::transports::esp32` |
| `Esp8266LedcLightDriverSettings` | `src/lights/esp32/Esp32LedcLightDriver.h` | `lw` | `src/transports/esp8266/Esp8266LedcLightDriver.h` | `lw::transports::esp8266` |
| `Esp8266LedcLightDriver<TColor>` | `src/lights/esp32/Esp32LedcLightDriver.h` | `lw` | `src/transports/esp8266/Esp8266LedcLightDriver.h` | `lw::transports::esp8266` |
| `Esp32SigmaDeltaLightDriverSettings` | `src/lights/esp32/Esp32SigmaDeltaLightDriver.h` | `lw` | `src/transports/esp32/Esp32SigmaDeltaLightDriver.h` | `lw::transports::esp32` |
| `Esp32SigmaDeltaLightDriver<TColor>` | `src/lights/esp32/Esp32SigmaDeltaLightDriver.h` | `lw` | `src/transports/esp32/Esp32SigmaDeltaLightDriver.h` | `lw::transports::esp32` |
| `RpPwmLightDriverSettings` | `src/lights/rp2040/RpPwmLightDriver.h` | `lw` | `src/transports/rp2040/RpPwmLightDriver.h` | `lw::transports::rp2040` |
| `RpPwmLightDriver<TColor>` | `src/lights/rp2040/RpPwmLightDriver.h` | `lw` | `src/transports/rp2040/RpPwmLightDriver.h` | `lw::transports::rp2040` |

### Header-level consolidation actions (non-member)

| Item | Current file | Action |
| --- | --- | --- |
| Lights umbrella header | `src/lights/Lights.h` | Delete |
| LumaWave lights umbrella | `src/LumaWave/Lights.h` | Remove or replace with transitional include (if needed) |
| Top-level aggregate include dependency | `src/LumaWave/All.h` | Remove `#include "LumaWave/Lights.h"` and rely on transport umbrella |

## Migration Workflow

1. Namespace extraction
- Move helper entities into target `...::detail` namespaces.
- Update all internal references with explicit qualification.

2. Syntax normalization
- Replace nested namespace block style with qualified namespace declarations where reasonable.

3. Contract verification
- Run compile/test gates and verify no seam regressions.

4. Hierarchy review
- Check whether resulting hierarchy makes ownership and call flow easier to audit.

5. Expand incrementally
- Apply same pattern to next candidate files in small PR-sized batches.

6. Consolidation cleanup
- Remove now-unused `lights/` includes and folder-level umbrella dependencies.
- Verify no residual references to `lights/` paths remain.

## Validation Gates

Required per phase:
- `pio test -e native-test`
- `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile`

Additional checks:
- grep for namespace drift outside targeted files.
- verify no type-trait or overload resolution regressions.
- inspect consistency of `<domain>` vs `<domain>::detail` placement.
- grep for stale include paths and references:
- `lights/`
- `LumaWave/Lights.h`
- `lights/Lights.h`

## Risks and Mitigations

Risk: wrong hierarchy placement during mechanical edits.
Mitigation: targeted symbol grep + small scoped PRs.

Risk: ADL/template lookup changes from namespace relocation.
Mitigation: explicit qualification at call sites and trait references.

Risk: oversized edits reduce review quality.
Mitigation: file-bounded phases with explicit before/after namespace inventory.

Risk: include-graph breakage after deleting `Lights.h`.
Mitigation: update umbrella includes in the same change set and run required compile/test gates immediately.

## Deliverables Checklist

- [ ] Apply Phase 1 namespace moves in `MakePixelBus.h`.
- [ ] Apply Phase 1 namespace moves in `ProtocolAliases.h`.
- [ ] Move `ILightDriver` and concrete light-driver headers from `lights/` to `transports/`.
- [ ] Delete `src/lights/Lights.h`.
- [ ] Remove/replace `src/LumaWave/Lights.h` and update `src/LumaWave/All.h` include chain.
- [ ] Run required test gates.
- [ ] Produce namespace diff summary for moved symbols.
- [ ] Propose Phase 2 file set based on Phase 1 results.

## Exit Criteria for This Planning Phase

- Namespace targets are explicit per domain.
- First migration slice is clearly bounded.
- Validation gates are defined and reproducible.
- Plan supports future API-boundary decisions without deciding them now.
