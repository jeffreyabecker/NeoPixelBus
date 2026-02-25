# Bus-Owned Protocol + Transport Roadmap (Quick Note)

## Goal

Enable bus types that *fully own* their underlying protocol + transport stack, while preserving:

- Existing `IPixelBus` consumer ergonomics
- Explicit compatibility guarantees (one-wire vs clock/data)
- Clear ownership/lifetime behavior (`ResourceHandle` and static-friendly construction)

## Current State (Today)

- `PixelBusT` owns pixel data and delegates frame output through one `IProtocol`.
- `OwningPixelBusT<TTransport, TProtocol>` already owns both transport and protocol through `ProtocolStateT`.
- Most protocols still model transport as a protocol setting (`ResourceHandle<ITransport> bus`).
- Factories are transport/protocol-composition oriented (for example WS2812x), not yet “single bundled driver” oriented.

## Gap to True “Bus Owns Stack”

Current ownership works, but assembly is still split across:

1. Bus/factory composition (`OwningPixelBusT` / `ProtocolStateT`)
2. Protocol settings (embedded transport handles)

To make the bus *fully own* the stack as one unit, we need a first-class bundled stack abstraction.

## Recommended Next Work

### 1) Introduce a bundled stack seam

Add a concrete stack object (name example: `ProtocolStackT` or `BusDriverT`) that encapsulates:

- Transport instance
- Protocol instance bound to that transport
- Protocol-facing update lifecycle (`initialize`, `update`, readiness)

Then allow `PixelBusT` (or a sibling bus type) to depend on this seam directly.

### 2) Normalize constructor conventions

Standardize protocol constructors so bundled factories can construct all protocols in a predictable way:

- Consistent `SettingsType` semantics
- Consistent `pixelCount` + settings entrypoint
- Clear owned vs borrowed transport choices

### 3) Add bundled factories

Create chipset-focused factory helpers (MVP: WS2812x first):

- `makeWs2812xBundledBus(...)`
- One call site with one configuration model
- No manual protocol/transport wiring in user code

### 4) Align compatibility logic

Unify category compatibility rules across protocol and factory constraints:

- `AnyTransportTag` wildcard behavior
- `TransportTag` and `OneWireTransportTag` strict matching

This avoids subtle differences between protocol-level and bus-factory-level checks.

### 5) Verify with focused examples/tests

Add or update virtual examples to validate:

- Static allocation path (primary)
- Owned stack lifecycle/teardown
- Timing/readiness parity with existing composition path

## Suggested MVP Scope

Start with one end-to-end path:

- Target: WS2812x on RP2040 PIO one-wire
- Deliverable: one bundled bus type + one factory + one example migration
- Success criteria:
  - Existing animation code compiles with minimal changes
  - No regression in `begin`/`show`/`canShow` behavior
  - No loss of transport/protocol category safety

## Non-Goals (for this MVP)

- Redesigning `SegmentBus`, `ConcatBus`, or `MosaicBus`
- Reworking shader architecture
- Changing consumer-facing color model semantics

## Migration Strategy

- Keep current `OwningPixelBusT` APIs intact.
- Add bundled APIs alongside existing ones.
- Migrate examples incrementally.
- Deprecate old composition helpers only after coverage exists for all major protocol families.

## Open Questions

- Should bundled stack abstraction be public API or internal-only behind factory returns?
- Do we keep protocol-level transport handle settings for advanced users, or hide them for bundled paths?
- Should “bundled” naming be generic (`BundledPixelBusT`) or chipset-specific only?
