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

1. `src/virtual/buses/Esp32DmaSpiClockDataTransport.h`
2. `src/virtual/buses/Esp32I2SClockDataTransport.h`
3. `src/virtual/buses/Esp8266I2SClockDataTransport.h`
4. `src/virtual/buses/Esp8266I2SSelfClockingTransport.h`
5. `src/virtual/emitters/Dmx512Emitter.h`

### Missing convenience layer

1. `makeNeoPixelBus(...)` factory functions
2. predefined color-order configs (`GrbConfig`, `GrbwConfig`, `RgbConfig`, `RgbwConfig`, etc.)

### Behavioral gap to resolve

- In-band settings for one-wire families listed in docs (TM1814/TM1914/SM168x variants) are still deferred and not implemented as emitter-owned config encoding.

---

## 3) Remaining Phases

## Phase A — Missing Chip/Bus Targets

### A.1 Add ESP32 DMA SPI clock/data bus
- Implement `src/virtual/buses/Esp32DmaSpiClockDataTransport.h`
- Conform to `IClockDataTransport` contract and existing SPI behavior expectations

### A.2 Add ESP32 I2S clock/data transport
- Implement `src/virtual/buses/Esp32I2SClockDataTransport.h`
- Treat this as the SPI-like-over-I2S transport path for ESP32 when hardware SPI pins/peripherals are constrained
- Support transport-level framing/throughput modes appropriate for clocked protocols that can be emitted over I2S on ESP32
- Keep protocol-specific packing in emitter/transform layer; transport stays byte-stream oriented
- Include validation notes for byte order, effective clock rate bounds, and platform guards (ESP32 variants where I2S TX mode is viable)

### A.2.1 ESP32 transport selection guidance (DMA SPI vs I2S)
- Prefer `Esp32DmaSpiClockDataTransport` when native SPI pin routing is available and lowest integration risk is preferred
- Prefer `Esp32I2SClockDataTransport` when SPI peripherals/pins are constrained or when I2S routing better matches board-level wiring
- Keep emitter-facing semantics identical (`IClockDataTransport`) so transport choice is swappable without emitter logic changes
- Validate both against the same byte-stream fixtures and timing envelopes to prevent transport-specific protocol drift

| Decision factor | Prefer DMA SPI transport | Prefer I2S transport |
|---|---|---|
| Pin/peripheral availability | SPI pins and controller are available without conflicts | SPI pins/controller are constrained or reserved |
| Board routing fit | Existing board design already centered on SPI nets | Existing board design already centered on I2S-capable routing |
| Integration risk | Lower risk: conventional SPI-like implementation path | Acceptable risk for alternative transport path |
| Throughput validation | Meets target with SPI clock envelope | Meets target with I2S clock/framing envelope |
| Portability across ESP32 variants | SPI support is straightforward for target variants | I2S TX capability is confirmed on target variants |

### A.3 Add ESP8266 I2S clock/data transport
- Implement `src/virtual/buses/Esp8266I2SClockDataTransport.h`
- Conform to `IClockDataTransport` for ESP8266 I2S-backed, SPI-like byte-stream scenarios

### A.4 Add ESP8266 I2S self-clocking transport
- Implement `src/virtual/buses/Esp8266I2SSelfClockingTransport.h`
- Model timing/encoding semantics needed by ESP8266 I2S-driven self-clocking protocols (including DMX512-style framing)

### A.5 Add DMX512 emitter (separate from transport)
- Implement `src/virtual/emitters/Dmx512Emitter.h`
- Keep DMX512 protocol framing and slot semantics emitter-owned; transport remains hardware-signal focused

Exit criteria:
- All five files compile and each has at least one smoke/integration example path.

---

## Phase C — Self-Clocking Transport Abstraction

Introduce a transport abstraction for one-wire signal engines so emitter logic can share the same high-level flow across RP2040 and ESP platforms.

### C.1 Platform emitter coverage and pilot migration

Documented one-wire emitters by platform (current source inventory):

| Platform | Current emitters in `src/virtual/emitters` | Pilot target for shared transport | Follow-on emitters to migrate |
|---|---|---|---|
| RP2040 | `RpPioOneWireEmitter` | `RpPioOneWireEmitter` | none (single current RP2040 path) |
| ESP32 | `Esp32RmtOneWireEmitter`, `Esp32I2sOneWireEmitter`, `Esp32I2sParallelOneWireEmitter`, `Esp32LcdParallelOneWireEmitter` | `Esp32RmtOneWireEmitter` | `Esp32I2sOneWireEmitter`, `Esp32I2sParallelOneWireEmitter`, `Esp32LcdParallelOneWireEmitter` |
| ESP8266 | `Esp8266UartOneWireEmitter`, `Esp8266DmaOneWireEmitter` | `Esp8266UartOneWireEmitter` (or `Esp8266DmaOneWireEmitter`) | ESP8266 I2S self-clocking transport integration path + DMX512 emitter binding |
| nRF52 | `Nrf52PwmOneWireEmitter` | optional validation pilot after core three platforms | `Nrf52PwmOneWireEmitter` |

- Keep color transform and shader logic in emitter-level shared flow while moving hardware signaling to transport bus classes.
- Complete at least one pilot migration each for RP2040, ESP32, and ESP8266 before broad rollout.

### C.2 Expand to remaining one-wire emitters
- Migrate all remaining listed emitters per platform matrix in C.1.
- For ESP32 I2S/LCD parallel variants, preserve explicit shared-context semantics.
- Ensure multi-channel/parallel buses correctly enforce `alwaysUpdate` when required.

### C.3 Validate no regressions
- Confirm waveform timings, inversion behavior, and reset/latch timing parity with current emitters
- Verify mixed platform examples still compile and run

Exit criteria:
- RP2040 + ESP8266 have at least one emitter using the shared one-wire bus abstraction.
- ESP32 one-wire coverage is complete across `Esp32RmtOneWireEmitter`, `Esp32I2sOneWireEmitter`, `Esp32I2sParallelOneWireEmitter`, and `Esp32LcdParallelOneWireEmitter`.
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

1. Phase A (missing chip/bus targets)
2. Phase C (one-wire signal bus abstraction)
3. Phase B (one-wire in-band settings)
4. Phase D (factories + config constants)

This order minimizes rework by establishing the shared one-wire transport layer before adding chip-specific one-wire settings encoding, and lets convenience APIs bind to finalized emitter/bus coverage.
