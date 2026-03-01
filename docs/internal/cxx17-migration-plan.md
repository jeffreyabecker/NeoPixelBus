# C++17 Migration Plan

Status: in progress  
Date: 2026-02-25

## Goal

Make C++17 the target language standard across the virtual-first codebase while preserving behavior and test coverage, and progressively isolate Arduino runtime dependencies behind narrow platform seams.

Out of scope for this plan phase:
- Platform-specific build validation (`esp32-smoke`, `esp8266-smoke`, board flashes).
- Feature additions unrelated to compatibility.

## Progress Snapshot (2026-02-25)

Completed in active code paths:
- C++17 compatibility layer introduced (`lw::span`, `lw::remove_cvref_t`).
- Core concept/requires removal started and applied through transport/protocol foundations.
- Bus/factory tranche converted (`BusDriver`, `ConcatBus`, `MosaicBus`, factory traits/make/shader factories).
- Protocol/transport/shader `std::span` surfaces migrated to `lw::span` aliases.
- Remaining active `concept`/`requires` usage removed from virtual-first headers.

Latest validation:
- `pio test -e native-test` -> PASS (`165/165`)
- `pio test -e native-test --filter contracts/test_protocol_transport_contract_matrix_compile` -> PASS
- `pio test -e native-test --filter shaders/test_color_domain_section1` -> PASS
- `pio run -e esp32-smoke` -> PASS
- `pio run -e esp8266-smoke` -> PASS

## Why this is non-trivial

A `span` shim solves only one category of incompatibility. Current sources also rely heavily on C++20 language features:
- Concepts and `requires` clauses in core/public template APIs.
- C++20 type traits usage patterns (`std::remove_cvref_t`).
- C++20-only test constructs (`consteval`, designated initializers in a few tests).

## Compatibility Findings (High-Level)

Primary C++20 blockers are concentrated in foundational headers:
- `src/transports/ITransport.h`
- `src/protocols/IProtocol.h`
- `src/buses/BusDriver.h`
- `src/factory/Traits.h`
- `src/factory/MakeBus.h`
- `src/colors/Color.h`

`std::span` appears across most protocol/transport/shader surfaces:
- `src/core/IPixelBus.h`
- `src/protocols/*`
- `src/transports/*`
- `src/colors/*`

Current state:
- Active virtual-first headers now use `lw::span` (compat alias) instead of direct `std::span`.

Arduino coupling is still present across virtual-first surfaces and should be reduced during the migration:
- Widespread `#include <Arduino.h>` in protocol/transport/factory headers.
- Direct runtime calls (`micros()`, `yield()`, `pinMode()`, `digitalWrite()`) in protocol and transport implementations.
- Output dependency on `Print` and `Serial` in debug/print transports and factory helpers.

## Arduino Dependency Abstraction Strategy

Target architecture:
- Keep Arduino APIs available at compatibility edges, but avoid requiring `Arduino.h` in core virtual-first contracts.
- Route runtime behavior through small adapter seams in `core/` so host-native tests and non-Arduino builds can supply replacements.
- Preserve zero/low-overhead call paths (inline wrappers or policy-based templates; no required heap/runtime polymorphism).

Proposed seam groups:
1. Runtime/time seam
  - Wrap `micros()`, `millis()`, `yield()` behind a small runtime adapter API.
2. Pin control seam
  - Wrap `pinMode()`/`digitalWrite()` for transports that manage line state directly.
3. Output/writer seam
  - Decouple `Print` from core transport/protocol templates by using a minimal writable contract and Arduino adapter types.
4. Include-boundary policy
  - Restrict `#include <Arduino.h>` to platform-specific transport edges and adapter headers.

Migration guardrails:
- Do not break existing Arduino-first consumer API in this phase.
- Keep convenience helpers (`printSerial()`, etc.) as adapter-backed wrappers.
- Validate native tests after each seam extraction chunk.

## Migration Strategy

### Phase 1 — Introduce Arduino abstraction seams (no behavior change)

1. Add a compact runtime abstraction header (time/yield + optional pin operations) under `src/core/` or `src/core/platform/`.
2. Add Arduino-backed default adapters in platform-facing headers.
3. Move direct Arduino calls in virtual-first protocol/transport internals to the seam API.
4. Keep Arduino includes only where adapters are implemented or where platform transports require direct HAL access.

Exit criteria:
- Core virtual-first interfaces no longer require direct Arduino runtime calls.
- Public API compatibility retained for Arduino sketches.
- Native tests remain green.

### Phase 2 — Add C++17 compatibility layer

1. Add a single compatibility header, e.g. `src/core/Compat.h`, that provides:
   - `lw::span` aliasing to:
     - `std::span` when available, otherwise
     - `tcb::span` (from `tcbrindle/span`).
   - `lw::remove_cvref_t` backport alias.
2. Add a vendored header path for `tcbrindle/span` (single-header copy, e.g. `src/third_party/tcb/span.hpp`).
3. Start replacing direct `<span>` usage in core interfaces with compat include + `lw::span`.

Exit criteria:
- Core interface headers compile under C++23 unchanged behavior.
- No immediate API behavior regressions.

### Phase 3 — Replace Concepts with C++17 traits/SFINAE

Convert constraints in priority order:
1. `src/transports/ITransport.h`
2. `src/protocols/IProtocol.h`
3. `src/buses/BusDriver.h` ✅
4. `src/factory/Traits.h` ✅
5. `src/factory/MakeBus.h` ✅
6. `src/colors/Color.h` and related shader/factory constraints

Replacement pattern:
- `concept` definitions -> boolean trait structs / `_v` constants.
- `requires` clauses -> `std::enable_if_t<...>* = nullptr` or dummy template parameter.
- Keep diagnostics quality with targeted `static_assert` messages near entry points.

Exit criteria:
- No `concept`/`requires` remains in active virtual-first headers.
- Native tests still pass under current toolchain mode.

### Phase 4 — Remove remaining C++20 syntax from tests

1. Replace `consteval` test helpers with `constexpr` or plain compile-time `static_assert` wrappers.
2. Replace designated initializer usages in tests with explicit struct assignment/builders where required.

Exit criteria:
- Test sources compile under C++17 mode.

### Phase 5 — Flip project standard to C++17

1. Update `platformio.ini` primary build/test flags from `-std=gnu++23` to `-std=gnu++17` for active migration environments (`pico2w`, `pico2w-virtual`, `native-test`).
2. Keep platform-specific smoke env work deferred.

Exit criteria:
- Core build/test environments use C++17 only.

### Phase 6 — Native-only validation gates

Run and require green:
- `pio test -e native-test`
- `pio test -e native-test --filter contracts/test_protocol_transport_contract_matrix_compile`
- `pio test -e native-test --filter shaders/test_color_domain_section1`

Exit criteria:
- All native tests pass with C++17.

## Suggested Implementation Order (Small PRs)

1. PR-A: Introduce runtime/pin/output seams and wire 2–3 representative protocol/transport headers through adapters.
2. PR-B: Introduce `Compat.h` + span shim wiring in 2–3 core headers. ✅
3. PR-C: Transport/protocol concept removal. (in progress)
4. PR-D: Bus/factory concept removal. ✅
5. PR-E: Color/shader concept removal + trait cleanup. ✅ (active headers)
6. PR-F: Test syntax downgrades (`consteval`, designated init). ✅
7. PR-G: Flip `platformio.ini` to C++17 + run native gates. ✅

## Risks and Mitigations

1. Template diagnostics regress and become harder to read.
- Mitigation: keep explicit `static_assert` guardrails with actionable messages.

2. Behavior drift while replacing constrained overloads.
- Mitigation: preserve signatures and run native gates after each PR chunk.

3. Span API mismatch between `std::span` and shim namespace.
- Mitigation: centralize through `lw::span`; avoid direct `std::span` in project headers.

4. Arduino abstraction adds accidental overhead or lifecycle complexity.
- Mitigation: prefer inline/policy-based adapters and keep ownership rules explicit in config types.

5. Hidden C++20 dependence remains after concept removal.
- Mitigation: one final grep pass for `concept|requires|consteval|<=>|remove_cvref_t|std::span` before flipping flags.

6. Hidden direct Arduino dependency remains in virtual-first headers.
- Mitigation: final grep pass for `#include <Arduino.h>|micros\(|millis\(|yield\(|pinMode\(|digitalWrite\(|\bPrint\b` across `src/core`, `src/protocols`, `src/buses`, `src/factory`, and `src/colors`.

## Readiness Checklist Before Flag Flip

- [x] `lw::span` compatibility layer in place.
- [ ] Runtime/pin/output seam adapters in place for virtual-first internals.
- [ ] Arduino includes limited to adapter/platform edge headers where feasible.
- [x] No active virtual-first headers include `<concepts>`.
- [x] No active virtual-first headers use `concept`/`requires`.
- [x] No tests use `consteval`.
- [x] Native test suite green in current mode.
- [x] Build flags switched to `-std=gnu++17` for migration environments.
- [x] Native gates re-run and green under C++17.
