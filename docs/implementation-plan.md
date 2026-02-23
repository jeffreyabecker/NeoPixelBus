# Implementation Plan (Remaining Unimplemented Work)

This plan tracks only work that is still outstanding in the current repository.
Completed foundations (core virtual bus, core emitters, shader method set, examples) are intentionally omitted.

---

## 1) Goal

Close the gap between current code and intended end-state by implementing missing infrastructure, missing emitters/buses, and convenience APIs.

---

## 2) Current Delta Summary

Chip/protocol coverage deltas are tracked in `docs/chip-gap-analysis.md`.

### Missing files / components

- none (Phase A targets implemented)

### Missing convenience layer

1. `makeNeoPixelBus(...)` factory functions
2. expanded predefined color-order configs beyond current core constants (`ChannelOrder::RGB`, `ChannelOrder::GRB`, `ChannelOrder::BGR`)

### Behavioral gap to resolve

- In-band settings for one-wire families listed in docs (TM1814/TM1914/SM168x variants) are still deferred and not implemented as emitter-owned config encoding.

---

## 3) Remaining Phases

## Phase C — Self-Clocking Transport Abstraction

Introduce a transport abstraction for one-wire signal engines so emitter logic stays platform-independent while transport classes encapsulate all platform-specific signaling details (RP2040/ESP/nRF, peripheral choice, DMA/PIO/RMT, inversion, and timing handoff).

Status update (2026-02-23):
- Completed: one-wire signaling routes through transport implementations.
- Wrapper one-wire protocol classes have been removed in favor of direct `Ws2812xProtocol` + `ISelfClockingTransport` composition.

### C.1 Platform emitter coverage and pilot migration

Emitter/transport boundary rule:
- Emitter code is platform-independent and owns protocol payload semantics only (color transform application, framing bytes, in-band settings packing, and update policy decisions).
- Transport code is platform-specific and owns hardware/peripheral behavior only (pin/peripheral setup, waveform/clock generation, DMA/PIO/RMT/I2S mechanics, and transport readiness).
- `ColorOrderTransform` is an emitter/protocol implementation detail for parameterizing channel-order packing; it is not a transport concern and not a consumer-facing abstraction.

Documented one-wire emitters by platform (current source inventory):

| Platform | Current emitters in `src/virtual/emitters` | Pilot target for shared transport | Follow-on emitters to migrate |
|---|---|---|---|
| RP2040 | `Ws2812xProtocol` + `RpPioSelfClockingTransport` | `RpPioSelfClockingTransport` | none (single current RP2040 path) |
| ESP32 | `Ws2812xProtocol` + (`Esp32RmtSelfClockingTransport`, `Esp32I2sSelfClockingTransport`, `Esp32I2sParallelSelfClockingTransport`, `Esp32LcdParallelSelfClockingTransport`) | `Esp32RmtSelfClockingTransport` | `Esp32I2sSelfClockingTransport`, `Esp32I2sParallelSelfClockingTransport`, `Esp32LcdParallelSelfClockingTransport` |
| ESP8266 | `Ws2812xProtocol` + (`Esp8266UartSelfClockingTransport`, `Esp8266DmaSelfClockingTransport`) | `Esp8266UartSelfClockingTransport` (or `Esp8266DmaSelfClockingTransport`) | ESP8266 I2S self-clocking transport integration path + DMX512 emitter binding |
| nRF52 | `Ws2812xProtocol` + `Nrf52PwmSelfClockingTransport` | optional validation pilot after core three platforms | `Nrf52PwmSelfClockingTransport` |

#### C.1.1 Shared transport seam definition
- Introduce one common self-clocking transport contract used by one-wire protocols (initialize, submit frame, readiness/query, transport-specific timing handoff).
- Define minimal invariants for all platforms:
	- frame submission is non-blocking or bounded-blocking (documented per platform),
	- readiness semantics are consistent (`isReadyToUpdate()` parity),
	- inversion/timing ownership lives in transport config, not emitter logic.
- Keep the emitter-facing API unchanged while replacing direct platform signaling calls with transport calls.

#### C.1.2 RP2040 pilot (`Ws2812xProtocol` + `RpPioSelfClockingTransport`)
- Extract RP2040 PIO/DMA orchestration behind the shared self-clocking transport interface.
- Keep protocol responsibilities limited to payload packing + update policy.
- Validate pilot with existing RP2040 virtual examples and confirm no change in reset/latch behavior.

Pilot acceptance:
- `Ws2812xProtocol` compiles with RP2040 signaling provided only by transport-facing calls.
- Byte stream and timing behavior match pre-migration output for equivalent fixtures.

#### C.1.3 ESP32 pilot (`Ws2812xProtocol` + `Esp32RmtSelfClockingTransport`)
- Migrate RMT signaling responsibilities into an ESP32 transport implementation.
- Preserve protocol framing and channel-order behavior in emitter path.
- Keep compatibility with follow-on ESP32 I2S/LCD-parallel migrations.

Pilot acceptance:
- `Ws2812xProtocol` no longer owns ESP32 platform signaling details directly.
- Existing RMT-targeted examples build with no public API changes.

#### C.1.4 ESP8266 pilot (`Ws2812xProtocol` + ESP8266 self-clocking transport)
- Select one ESP8266 transport path and migrate signaling lifecycle into shared transport abstraction.
- Preserve current readiness and update cadence behavior to avoid frame pacing regressions.
- Keep second ESP8266 one-wire path as follow-on in C.2.

Pilot acceptance:
- Selected ESP8266 pilot transport compiles and runs through transport abstraction.
- Emitted data stream remains behavior-equivalent for baseline test patterns.

#### C.1.5 Cross-platform convergence checks
- Enforce identical emitter-side structure across RP2040/ESP32/ESP8266 pilots (no platform `#if` branching in emitter update loops).
- Confirm shader/color-order flow remains emitter-owned and shared.
- Confirm `alwaysUpdate()` behavior is preserved when transport requires continuous refresh.

#### C.1.6 Exit gate for C.1
- Complete at least one pilot migration each for RP2040, ESP32, and ESP8266 before broad rollout.
- Record per-platform migration notes (transport type, readiness semantics, inversion/timing ownership).
- Promote unresolved platform-specific exceptions to explicit C.2 work items.

### C.2 Expand to remaining one-wire emitters
- Migrate all remaining listed emitters per platform matrix in C.1.
- For ESP32 I2S/LCD parallel variants, preserve explicit shared-context semantics.
- Preserve the same platform-independent emitter surface across all transports; do not add platform branches inside emitters.
- Ensure multi-channel/parallel buses correctly enforce `alwaysUpdate` when required.

### C.3 Validate no regressions
- Confirm waveform timings, inversion behavior, and reset/latch timing parity with current emitters
- Verify mixed platform examples still compile and run

### C.4 Consolidate color-order packing into shared protocol
- After all one-wire transports are migrated and stable, roll `ColorOrderTransform` functionality into `Ws2812xProtocol` as protocol-private packing logic.
- Preserve current channel-count and channel-order configurability while removing the standalone helper type.
- Keep this refactor transport-agnostic and behavior-equivalent (byte ordering and frame sizing unchanged).

Exit criteria:
- RP2040 + ESP8266 have at least one emitter using the shared one-wire bus abstraction.
- ESP32 one-wire coverage is complete across `Esp32RmtSelfClockingTransport`, `Esp32I2sSelfClockingTransport`, `Esp32I2sParallelSelfClockingTransport`, and `Esp32LcdParallelSelfClockingTransport`.
- Existing one-wire examples continue to pass with no behavior regressions.
- `Ws2812xProtocol` owns color-order packing directly with no standalone `ColorOrderTransform` class.

---

## Phase B — One-Wire In-Band Settings (Deferred Gap Closure)

Implement emitter-owned config + inline encoding for deferred one-wire families:

1. TM1814 current settings encoding
2. TM1914 mode settings encoding
3. SM168x gain packing variants (3-channel / 4-channel / 5-channel)

Rules:
- settings are owned by emitter config structs
- encoding is performed inside emitter `update()`
- no separate global settings shuttle types

Exit criteria:
- Settings bytes appear in expected stream positions in `examples-virtual/in-band-settings/main.cpp` scenarios.

---

## Phase D — Convenience API + Migration Surface

### D.1 Factory functions
- Add `makeNeoPixelBus(...)` family for common chip/platform combinations
- Default selection should hide platform timing/inversion details where possible

### D.2 Predefined color-order configs
- Completed baseline constants: `ChannelOrder::RGB`, `ChannelOrder::GRB`, `ChannelOrder::BGR` and constexpr lengths in `Color.h`
- Expand shared constants for additional layouts (`GRBW`, `RGBW`, etc.)
- Keep definitions centralized and include-safe

Exit criteria:
- Public, ergonomic construction paths exist without manual emitter wiring for common use cases.

---

## 4) Validation Checklist

For each phase completion:

1. Build succeeds for current primary environment(s)
2. Relevant `examples-virtual/*` target compiles
3. Byte-stream output matches expected framing/ordering for known fixture pixels
4. Existing implemented emitters/shaders are not behavior-regressed

---

## 5) Non-Goals (for this remaining plan)

- Reworking already functional core classes solely to match old naming in prior drafts
- Broad architecture rewrites unrelated to missing deliverables
- Introducing new chip families not already listed above

---

## 6) Suggested Execution Order

1. Phase C (one-wire signal bus abstraction)
2. Phase B (one-wire in-band settings)
3. Phase D (factories + config constants)

This order minimizes rework by establishing the shared one-wire transport layer before adding chip-specific one-wire settings encoding, and lets convenience APIs bind to finalized emitter/bus coverage.
