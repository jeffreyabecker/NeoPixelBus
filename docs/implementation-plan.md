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
2. predefined color-order configs (`GrbConfig`, `GrbwConfig`, `RgbConfig`, `RgbwConfig`, etc.)

### Behavioral gap to resolve

- In-band settings for one-wire families listed in docs (TM1814/TM1914/SM168x variants) are still deferred and not implemented as emitter-owned config encoding.

---

## 3) Remaining Phases

## Phase C — Self-Clocking Transport Abstraction

Introduce a transport abstraction for one-wire signal engines so emitter logic stays platform-independent while transport classes encapsulate all platform-specific signaling details (RP2040/ESP/nRF, peripheral choice, DMA/PIO/RMT, inversion, and timing handoff).

### C.1 Platform emitter coverage and pilot migration

Emitter/transport boundary rule:
- Emitter code is platform-independent and owns protocol payload semantics only (color transform application, framing bytes, in-band settings packing, and update policy decisions).
- Transport code is platform-specific and owns hardware/peripheral behavior only (pin/peripheral setup, waveform/clock generation, DMA/PIO/RMT/I2S mechanics, and transport readiness).

Documented one-wire emitters by platform (current source inventory):

| Platform | Current emitters in `src/virtual/emitters` | Pilot target for shared transport | Follow-on emitters to migrate |
|---|---|---|---|
| RP2040 | `RpPioOneWireProtocol` | `RpPioOneWireProtocol` | none (single current RP2040 path) |
| ESP32 | `Esp32RmtOneWireProtocol`, `Esp32I2sOneWireProtocol`, `Esp32I2sParallelOneWireProtocol`, `Esp32LcdParallelOneWireProtocol` | `Esp32RmtOneWireProtocol` | `Esp32I2sOneWireProtocol`, `Esp32I2sParallelOneWireProtocol`, `Esp32LcdParallelOneWireProtocol` |
| ESP8266 | `Esp8266UartOneWireProtocol`, `Esp8266DmaOneWireProtocol` | `Esp8266UartOneWireProtocol` (or `Esp8266DmaOneWireProtocol`) | ESP8266 I2S self-clocking transport integration path + DMX512 emitter binding |
| nRF52 | `Nrf52PwmOneWireProtocol` | optional validation pilot after core three platforms | `Nrf52PwmOneWireProtocol` |

- Keep color transform and shader logic in emitter-level shared flow while moving all platform-specific signaling to transport bus classes.
- Complete at least one pilot migration each for RP2040, ESP32, and ESP8266 before broad rollout.

### C.2 Expand to remaining one-wire emitters
- Migrate all remaining listed emitters per platform matrix in C.1.
- For ESP32 I2S/LCD parallel variants, preserve explicit shared-context semantics.
- Preserve the same platform-independent emitter surface across all transports; do not add platform branches inside emitters.
- Ensure multi-channel/parallel buses correctly enforce `alwaysUpdate` when required.

### C.3 Validate no regressions
- Confirm waveform timings, inversion behavior, and reset/latch timing parity with current emitters
- Verify mixed platform examples still compile and run

Exit criteria:
- RP2040 + ESP8266 have at least one emitter using the shared one-wire bus abstraction.
- ESP32 one-wire coverage is complete across `Esp32RmtOneWireProtocol`, `Esp32I2sOneWireProtocol`, `Esp32I2sParallelOneWireProtocol`, and `Esp32LcdParallelOneWireProtocol`.
- Existing one-wire examples continue to pass with no behavior regressions.

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
- Add shared constants for common layouts (`GRB`, `GRBW`, `RGB`, `RGBW`, etc.)
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
