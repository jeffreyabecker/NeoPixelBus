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
- [ ] Expose access to the factory behind static `makeBus(...)` results (for example via `getFactory(makeBus(...))`) so callers can query buffer requirements (`getBufferSize()`) and allocate external backing storage before use.
- [ ] Add a bus path that is compile-time allocatable (no runtime heap requirement) for fixed-size/static-storage deployments.
