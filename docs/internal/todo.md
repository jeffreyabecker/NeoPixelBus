# Internal Backlog

## In Progress / Existing

- [ ] Add bus-level config for refresh coordination (`fullRefreshOnly` / wait for all transports to finish).
- [ ] Support non-reallocating settings alteration and expose common interfaces through composite busses (primary use-case: alter shader settings on the fly).
- [ ] Examine whether `IProtocol::bindTransport` is still required, or whether `PixelBus` should own/manage transport binding now that it has both the frame buffer and transport endpoint.
- [ ] Expose access to the factory behind static `makeBus(...)` results (for example via `getFactory(makeBus(...))`) so callers can query buffer requirements (`getBufferSize()`) and allocate external backing storage before use.

## Cleanup Backlog (Prioritized)

### P0 (quick wins)

#### Unsupported-chip backlog split (from [docs/internal/neopixelbus-unsupported-chips.md](neopixelbus-unsupported-chips.md))

- [ ] Add TM1829 descriptor alias (`Ws2812x` + `timing::Tm1829` + RGB + `invert=true`) and one compile-first contract test.
- [ ] Decide TM1829 policy: keep alias as first-class convenience descriptor vs keep timing-only/manual composition; document final rationale in usage docs.
- [ ] Add Intertek timing profile only if a reproducible device/user request appears; otherwise keep as explicit no-demand deferment.
- [ ] Track SM168x one-wire per-pixel-settings family as deferred protocol work; choose direction (`Sm168xOneWireProtocol` vs `Ws2812xProtocol` suffix extension) before implementation.
- [ ] Resolve or explicitly triage the ESP32 C++17 workaround note in [platformio/cfg/esp32.ini](../../platformio/cfg/esp32.ini) (either lock a core/toolchain path or document why flag overrides stay).

#### Notes cleanup conversion (from [docs/internal/notes.md](notes.md))

- [ ] Audit protocol aliases and verify default channel-order assumptions against current protocol definitions.

### P1 (consistency + maintainability)

- [ ] Create a dedicated TLC5947 transport-contract task from the protocol TODO in [src/protocols/Tlc5947Protocol.h](../../src/protocols/Tlc5947Protocol.h) and define done criteria.

### P2 (hygiene)

- [ ] Validate internal doc and test index links in [docs/internal/README.md](README.md) and [test/README.md](../../test/README.md).
- [ ] Run a naming consistency sweep for bus terminology (`bus`, `busses`, `buses`) and normalize where practical.