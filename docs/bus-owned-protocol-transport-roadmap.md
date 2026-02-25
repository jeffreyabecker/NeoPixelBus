# Bus-Owned Protocol + Transport Roadmap (Quick Note)

## Goal

Enable bus types that *fully own* their underlying protocol + transport stack, while preserving:

- Existing `IPixelBus` consumer ergonomics
- Explicit compatibility guarantees (one-wire vs clock/data)
- Clear ownership/lifetime behavior (`ResourceHandle` and static-friendly construction)

## Alpha API Policy

- This project is in alpha; API compatibility is not a blocker.
- Prefer converging quickly on the BusDriver model over preserving parallel legacy surfaces.
- Keep only the minimum temporary compatibility needed to keep active development moving.

## Status Snapshot

### Implementation Checklist

- [x] Unify protocol/factory transport compatibility rules (`AnyTransportTag` wildcard, strict concrete tag matching).
- [x] Add BusDriver seam primitives (`IBusDriver`, `ProtocolBusDriverT`, `BusDriverPixelBusT`).
- [x] Add owning BusDriver factory base (`OwningBusDriverPixelBusT` + `makeOwningBusDriverPixelBus(...)`).
- [x] Add WS2812x BusDriver factory (`makeWs2812xOwningBusDriverPixelBus(...)`).
- [x] Add WS2812x shader-enabled BusDriver factory (`makeWs2812xOwningShaderBusDriverPixelBus(...)`).
- [x] Add RP2040 WS2812x BusDriver example (`examples-virtual/rp2040-ws2812x-busdriver/main.cpp`).
- [x] Add a shader-enabled BusDriver example (dedicated sample sketch: `examples-virtual/rp2040-ws2812x-busdriver-shader/main.cpp`).
- [x] Add/expand smoke build coverage for BusDriver + shader paths.
- [x] Add BusDriver factories for additional protocol families (DotStar, WS2801, Pixie; SM/TM/TLC pending as needed).
- [ ] Publish usage guidance for the BusDriver-first construction model.

### Preserved Constraints Checklist

- [x] Decide whether protocol settings-based manual construction remains first-class or moves to secondary/advanced status. (Decision: secondary/advanced during refactor; BusDriver factories are first-class/public-default.)
- [x] Decide whether `SettingsType` with `ResourceHandle<ITransport> bus` remains public long-term or becomes transitional. (Decision: keep as an advanced public path for manual bus assembly, including config-driven construction.)
- [x] Decide whether the BusDriver abstraction remains fully public API or is mostly consumed through factories. (Decision: keep BusDriver public, but primarily consume it via chipset/protocol factories.)
- [x] Decide whether BusDriver naming stays generic or shifts to chipset-specific aliases. (Decision: keep generic naming as default; introduce dedicated chipset aliases only when justified by a major usability or technical issue.)
- [x] Remove additive-only framing and define BusDriver as primary architecture.

## Current State (Today)

- `PixelBusT` owns pixel data and delegates frame output through one `IProtocol`.
- `OwningPixelBusT<TTransport, TProtocol>` already owns both transport and protocol through `ProtocolStateT`.
- `BusDriverPixelBusT<TColor>` is the primary bus architecture path and delegates frame output through `IBusDriver<TColor>`.
- Most protocols model transport as a protocol setting (`ResourceHandle<ITransport> bus`), retained as an advanced/manual assembly path.
- Legacy transport/protocol composition surfaces still exist temporarily for migration, but BusDriver-oriented factory paths are the primary direction.

## Remaining Gap to Full BusDriver Adoption

Current ownership works, but assembly is still split across:

1. Legacy transport/protocol composition (`OwningPixelBusT` / `ProtocolStateT`)
2. BusDriver composition path (`IBusDriver` + `BusDriverPixelBusT`)
3. Protocol settings/manual wiring path (`SettingsType` with transport handles)

This split is temporary for alpha migration, but coverage across protocol families and docs is not yet complete.

### BusDriver Migration Milestones (Required)

#### Phase 1 — Core Surface Parity

- [x] Complete BusDriver factory coverage for high-use protocol families.
- [x] Complete shader-enabled BusDriver coverage for prioritized protocol families.
- [x] Define final disposition of protocol `SettingsType` + `ResourceHandle<ITransport>` (advanced public path vs transitional/internal path).

Phase gate:

- [x] Factory and shader parity reached for the Phase 1 target protocol set.

#### Phase 2 — Examples and Build Coverage

- [ ] Rewrite first-party examples to BusDriver-first implementations (in place where practical), rather than maintaining parallel equivalent variants.
- [ ] Keep only migration-critical legacy examples.
- [ ] Expand smoke/build matrix so BusDriver and shader-enabled BusDriver paths compile in every protocol-family CI lane.

Progress in this batch:

- Rewrote `examples-virtual/rp2040-neopixel/main.cpp` to BusDriver-first.
- Rewrote `examples-virtual/rp2040-ws2812x-bundle/main.cpp` to BusDriver-first.
- Rewrote `examples-virtual/pixie-stream/main.cpp` to BusDriver-first.
- Added dedicated smoke env lanes: `pico2w-virtual-onewire-busdriver`, `pico2w-virtual-onewire-busdriver-shader`.

Phase gate:

- [ ] Example parity and CI/build parity reached across supported protocol families.

#### Phase 3 — Migration Completion

- [ ] Remove or hard-deprecate legacy factory entrypoints once parity gates are complete.
- [ ] Satisfy stabilization criteria for declaring BusDriver the default model (API completeness, test coverage, example parity, BusDriver-first docs parity).

Phase gate:

- [ ] BusDriver declared default model and legacy surface reduced to intended end-state.

## Next Work (Full Refactor Workstreams)

1. Complete BusDriver factory coverage across all maintained protocol families (including SM/TM/TLC variants).
2. Complete shader-enabled BusDriver coverage for prioritized protocol families and publish canonical shader composition patterns.
3. Rewrite first-party examples to BusDriver-first examples (prefer in-place migration over parallel equivalents); keep only targeted legacy examples needed during transition.
4. Expand smoke/build matrix so BusDriver and shader-enabled BusDriver paths compile for every supported protocol family.
5. Remove/hard-deprecate legacy factory entrypoints as parity gates are met and complete BusDriver-first documentation cleanup.

## Refactor Scope

This effort is a full architecture refactor toward a BusDriver-primary model, not an additive feature track.

- Primary target: make BusDriver the default construction model across protocol families.
- Transitional support: keep legacy composition surfaces only long enough to migrate examples/tests/docs.
- Exit criteria:
  - BusDriver factory parity across target protocol families
  - BusDriver shader-path parity for priority protocols
  - CI/build parity and example parity
  - BusDriver-first documentation parity and removal/deprecation plan executed

## Non-Goals

- Redesigning `SegmentBus`, `ConcatBus`, or `MosaicBus`
- Reworking shader architecture
- Changing consumer-facing color model semantics

## Migration Strategy

- Refactor toward BusDriver as the single primary model across construction and documentation.
- Migrate protocol families in batches with explicit parity gates (factory coverage, shader coverage, BusDriver-first example rewrites, build coverage).
- Remove or hard-deprecate legacy helpers as each batch reaches parity; avoid indefinite dual-surface maintenance.

## Open Questions

- None currently.
