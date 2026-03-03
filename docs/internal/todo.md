# Internal Backlog

## In Progress / Existing

- [ ] Add bus-level config for refresh coordination (`fullRefreshOnly` / wait for all transports to finish).
- [ ] Support non-reallocating settings alteration and expose common interfaces through composite busses (primary use-case: alter shader settings on the fly).
- [ ] Remove `IProtocol::setBuffer` now that protocol buffers are passed explicitly into the show/update path.
- [ ] Examine whether `IProtocol::bindTransport` is still required, or whether `PixelBus` should own/manage transport binding now that it has both the frame buffer and transport endpoint.

## Cleanup Backlog (Prioritized)

### P0 (quick wins)

- [x] Consolidate scattered TODOs into this file from [docs/internal/notes.md](notes.md), [src/protocols/Tlc5947Protocol.h](../../src/protocols/Tlc5947Protocol.h), and [platformio/cfg/esp32.ini](../../platformio/cfg/esp32.ini) by decomposing them into explicit checklist items below and in P1.

#### Unsupported-chip backlog split (from [docs/internal/neopixelbus-unsupported-chips.md](neopixelbus-unsupported-chips.md))

- [ ] Add TM1829 descriptor alias (`Ws2812x` + `timing::Tm1829` + RGB + `invert=true`) and one compile-first contract test.
- [ ] Decide TM1829 policy: keep alias as first-class convenience descriptor vs keep timing-only/manual composition; document final rationale in usage docs.
- [ ] Add Intertek timing profile only if a reproducible device/user request appears; otherwise keep as explicit no-demand deferment.
- [ ] Track SM168x one-wire per-pixel-settings family as deferred protocol work; choose direction (`Sm168xOneWireProtocol` vs `Ws2812xProtocol` suffix extension) before implementation.


#### Notes cleanup conversion (from [docs/internal/notes.md](notes.md))

- [ ] Evaluate fixed-point math approach (`fpm`) as an alternative to float in constrained/hot paths; define criteria before adoption.
- [ ] Add a small example-animation plan using `ColorIterator` patterns and map candidate examples under `examples/`.
- [ ] Audit protocol aliases and verify default channel-order assumptions against current protocol definitions.
- [ ] Create a focused task to evaluate externalizing one-wire encoding from `Esp32RmtTransport` without breaking protocol/transport seams.
- [ ] Add a DotStar white-only support investigation task (format assumptions, brightness-path behavior, and required color model decisions).

### P1 (consistency + maintainability)

- [ ] Resolve or explicitly triage the ESP32 C++17 workaround note in [platformio/cfg/esp32.ini](../../platformio/cfg/esp32.ini) (either lock a core/toolchain path or document why flag overrides stay).
- [ ] Create a dedicated TLC5947 transport-contract task from the protocol TODO in [src/protocols/Tlc5947Protocol.h](../../src/protocols/Tlc5947Protocol.h) and define done criteria.
- [ ] Run a docs consistency pass for wording/typos/terminology drift in internal docs (focus on [docs/internal/notes.md](notes.md) and [docs/internal/comparison-lumawave-vs-neopixelbus.md](comparison-lumawave-vs-neopixelbus.md)).

### P2 (hygiene)

- [ ] Validate internal doc and test index links in [docs/internal/README.md](README.md) and [test/README.md](../../test/README.md).
- [ ] Run a naming consistency sweep for bus terminology (`bus`, `busses`, `buses`) and normalize where practical.