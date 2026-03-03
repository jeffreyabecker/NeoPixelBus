# Internal Backlog

## In Progress / Existing

- [ ] Add bus-level config for refresh coordination (`fullRefreshOnly` / wait for all transports to finish).
- [ ] Support non-reallocating settings alteration and expose common interfaces through composite buses (primary use-case: alter shader settings on the fly).
- [ ] Rename initialization methods consistently to `begin` across protocol/transport/bus-facing interfaces and implementations.
- [ ] Remove legacy `src/factory/DynamicBusConfigParser.h` after call sites fully migrate to current dynamic-bus config parsing flow.
- [ ] Add a compile flag to isolate INI/spec parsing from the rest of factory (`BuildDynamicBusBuilderFromIni`, parser/reader), while preserving dynamic builder support (INI/spec path depends on `DynamicBusBuilder`).
- [ ] Add a compile flag to isolate static factories (`makeBus`, static descriptor/trait path) from the rest of factory for static-only consumer builds.
- [ ] Add a compile flag to isolate `DynamicBusBuilder` from the rest of factory for runtime-builder-only consumers that do not use INI/spec parsing.
- [ ] Define and document a consistent factory compile-flag naming scheme before implementation (proposed pattern: `LW_FACTORY_ENABLE_<SUBSYSTEM>` with explicit defaults and dependency notes).
- [x] Separate protocol and transport responsibilities so `PixelBus` owns transport readiness/transaction orchestration while protocols remain encode-focused; completed with the protocol/transport decoupling refactor.
- [ ] Expose access to the factory behind static `makeBus(...)` results (for example via `getFactory(makeBus(...))`) so callers can query buffer requirements (`getBufferSize()`) and allocate external backing storage before use.
- [ ] Add a bus path that is compile-time allocatable (no runtime heap requirement) for fixed-size/static-storage deployments.

## Cleanup Backlog (Prioritized)

### P0 (quick wins)

#### Unsupported-chip backlog split (from [docs/internal/neopixelbus-unsupported-chips.md](neopixelbus-unsupported-chips.md))

- [x] Add TM1829 descriptor alias (`Ws2812x` + `timing::Tm1829` + RGB + `invert=true`) and one compile-first contract test.
- [x] Decide TM1829 policy: keep alias as first-class convenience descriptor vs keep timing-only/manual composition; document final rationale in usage docs. -- keep the descriptor because its there and works
- [x] Add Intertek timing profile only if a reproducible device/user request appears; otherwise keep as explicit no-demand deferment.
- [x] Resolve or explicitly triage the ESP32 C++17 workaround note in [platformio/cfg/esp32.ini](../../platformio/cfg/esp32.ini) (either lock a core/toolchain path or document why flag overrides stay).

#### Notes cleanup conversion (from [docs/internal/notes.md](notes.md))

- [x] Audit protocol descriptor aliases and verify default channel-order assumptions against current protocol/trait definitions; document any mismatches and follow-up actions. -- completed 2026-03-02, see `docs/internal/notes.md`

### P1 (consistency + maintainability)

- [x] Keep TLC5947 documented as unsupported until a latch/OE-aware transport contract is defined and validated (then re-open support work).

### P2 (hygiene)

- [x] Validate internal doc and test index links in [docs/internal/README.md](README.md) and [test/README.md](../../test/README.md).
- [x] Run a naming consistency sweep for bus terminology (`bus`, `busses`, `buses`) and normalize where practical (kept `busses` where it is part of stable file paths/identifiers).