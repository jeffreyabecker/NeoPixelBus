# Bus Driver Ownership Refactor Proposal

Status: draft

## Summary

This proposal clarifies bus-driver ownership semantics by separating:

- **Static composition** (driver stored directly in bus type)
- **Heap ownership** (driver owned via `std::unique_ptr`)

The current `OwningBusDriverPixelBusT` name is misleading because it is implemented as direct composition and self-borrowing, not heap ownership.

Proposed direction:

1. Rename current `OwningBusDriverPixelBusT` to `StaticBusDriverPixelBusT`
2. Add `HeapBusDriverPixelBusT` for true heap ownership
3. Adopt borrow-first dependency flow for protocol/transport/shader under bus-driver ownership
4. Update all call sites to new canonical naming (no compatibility aliases)
5. Keep composite owning types, but remove composite free-factory helpers

---

## Motivation

### Problem

`OwningBusDriverPixelBusT` currently means "owns by containing the driver as a direct base subobject," not "owns through an owning pointer."
That causes naming friction and makes API intent harder to read.

### Design intent

The bus layer should expose ownership explicitly through type names:

- `Static*` means direct composition, allocation known at compile time
- `Heap*` means ownership through `std::unique_ptr`

This preserves embedded-friendly static allocation while enabling a pointer-owned variant when needed.

---

## Scope

### In scope

- Bus-driver type naming and factory entry points
- Clear static vs heap bus construction APIs
- Protocol/shader dependency migration from `ResourceHandle` to borrowing references/pointers where bus-driver owns lifetime
- Direct call-site updates to canonical static/heap APIs

### Out of scope

- Removing `ResourceHandle` globally in one pass
- Changing external protocol/transport contracts

---

## Ownership Policy Revision (Borrow-first)

This proposal now assumes a **borrow-first** policy for the bus-driver construction path:

- `StaticBusDriverPixelBusT` and `HeapBusDriverPixelBusT` are the primary owners of transport + protocol + embedded shader chain
- Protocol and shader objects should generally receive borrowed dependencies (`T&`, `const T&`, or nullable `T*` when optional)
- `ResourceHandle` is retained only where mixed ownership/nullability is a first-class product requirement (for example composite bus containers or optional debug chains)

Design objective:

- Remove ownership mode branching from hot-path protocol settings
- Keep object graph ownership centralized at bus-driver construction boundaries
- Make dependency lifetimes obvious from constructor signatures

Composite policy alignment:

- Composite owning types (`OwningConcatBusT`, `OwningMosaicBusT`) may remain available
- Composite free-factory helpers are removed to reduce redundant construction APIs

---

## Proposed Type Model

### 1) Core non-owning bus wrapper

Retain a core bus wrapper that drives pixel storage + show lifecycle and forwards to a driver instance.

- Holds a non-owning pointer/reference to driver
- No ownership policy logic inside this core type

Conceptually:

```cpp
template <typename TDriver>
class BusDriverPixelBusT : public IPixelBus<typename TDriver::ColorType>
{
    // _colors, _dirty, begin/show/canShow/get/set pixel behavior
    // stores TDriver* _driver (or equivalent reference wrapper)
};
```

### 2) Static composition owner

Current behavior under a clearer name:

```cpp
template <typename TTransport, typename TProtocol>
class StaticBusDriverPixelBusT
    : private ProtocolBusDriverT<TProtocol, TTransport>
    , public BusDriverPixelBusT<ProtocolBusDriverT<TProtocol, TTransport>>
{
    // same transport()/protocol() accessors as current OwningBusDriverPixelBusT
};
```

### 3) Heap ownership owner

New true pointer-owning variant:

```cpp
template <typename TTransport, typename TProtocol>
class HeapBusDriverPixelBusT
    : public BusDriverPixelBusT<ProtocolBusDriverT<TProtocol, TTransport>>
{
public:
    using DriverType = ProtocolBusDriverT<TProtocol, TTransport>;

private:
    std::unique_ptr<DriverType> _driver;
};
```

This provides an ownership path that is explicit and conventional (`std::unique_ptr`) while preserving bus behavior parity.

---

## Draft API Surface

## Type aliases

```cpp
// New canonical names
template <typename TTransport, typename TProtocol>
using StaticBusDriverPixelBusT = /* current OwningBusDriverPixelBusT implementation */;

template <typename TTransport, typename TProtocol>
class HeapBusDriverPixelBusT;
```

## Protocol/Shader settings direction (draft)

```cpp
// before
struct Ws2812xProtocolSettings
{
    ResourceHandle<ITransport> bus;
    const char* channelOrder = ChannelOrder::GRB;
};

// after (borrowed dependency)
struct Ws2812xProtocolSettings
{
    ITransport* bus = nullptr; // nullable for default-constructibility where needed
    const char* channelOrder = ChannelOrder::GRB;
};
```

Equivalent direction applies to shader wrappers:

```cpp
// before: ResourceHandle<IShader<TColor>> shader;
// after:  IShader<TColor>* shader = nullptr;
```

For non-optional dependencies, prefer references at constructor boundaries and store pointer/reference internally.

### Debug optional-chain policy (locked)

For optional chaining scenarios (for example `DebugProtocol` wrapping an optional downstream protocol),
the project will use **nullable pointer semantics** instead of `ResourceHandle`:

- Optional dependency field type: `IProtocol<TColor>*` (default `nullptr`)
- Optional shader field type: `IShader<TColor>*` (default `nullptr`)
- Ownership remains outside protocol settings; no owning transfer through settings objects
- `nullptr` continues to represent "no chained dependency"

## Factory functions

```cpp
// Canonical static factory (rename from makeOwningDriverPixelBus)
template <typename TTransport, typename TProtocol>
StaticBusDriverPixelBusT<TTransport, TProtocol> makeStaticDriverPixelBus(
    uint16_t pixelCount,
    typename TTransport::TransportSettingsType transportSettings,
    typename TProtocol::SettingsType protocolSettings);

// Canonical heap factory
template <typename TTransport, typename TProtocol>
HeapBusDriverPixelBusT<TTransport, TProtocol> makeHeapDriverPixelBus(
    uint16_t pixelCount,
    typename TTransport::TransportSettingsType transportSettings,
    typename TProtocol::SettingsType protocolSettings);
```

## `factory::Bus` behavior

Default behavior remains static (embedded-first):

```cpp
template <typename TProtocolConfig,
          typename TTransportConfig,
          typename TShaderFactory = void>
using Bus = StaticBusDriverPixelBusT<
    typename TransportConfigTraits<remove_cvref_t<TTransportConfig>>::TransportType,
    /* protocol resolved from config + optional embedded shader */>;
```

Optional future extension (not required in first step):

```cpp
struct StaticOwnershipTag {};
struct HeapOwnershipTag {};

template <typename TProtocolConfig,
          typename TTransportConfig,
          typename TShaderFactory = void,
          typename TOwnershipTag = StaticOwnershipTag>
using Bus = /* selects StaticBusDriverPixelBusT or HeapBusDriverPixelBusT */;
```

---

## Construction Examples (Draft)

```cpp
using namespace npb::factory;

// static/direct-composition bus
auto staticBus = makeStaticDriverPixelBus<DebugTransport, Ws2812xProtocol<Rgb8Color>>(
    60,
    DebugTransportSettings{},
    Ws2812xProtocolSettings{});

// heap-owned bus
auto heapBus = makeHeapDriverPixelBus<DebugTransport, Ws2812xProtocol<Rgb8Color>>(
    60,
    DebugTransportSettings{},
    Ws2812xProtocolSettings{});

// existing high-level makeBus(...) continues to return static by default
auto bus = makeBus(60, Ws2812{}, debugSerial());
```

---

## Migration Plan

### Phase 1 (additive)

- Rename `OwningBusDriverPixelBusT` to `StaticBusDriverPixelBusT`
- Add `HeapBusDriverPixelBusT`
- Rename `makeOwningDriverPixelBus` to `makeStaticDriverPixelBus`
- Add `makeHeapDriverPixelBus`
- Update all internal/external call sites to the new names in the same change window

### Phase 2 (guided migration)

- Update docs and examples to use `Static*` naming
- Validate no remaining references to legacy names

### Phase 3 (cleanup)

- Not needed; legacy compatibility symbols are not retained

---

## Compatibility Notes

- `makeBus(...)` behavior remains static-by-default.
- Source compatibility is intentionally broken for legacy `Owning*` symbol names; call sites must be updated.
- Protocol/shader setting APIs are expected to change toward borrowing forms; call sites must be updated accordingly.

---

## Implementation Checklist (Draft)

This checklist maps the proposal to concrete edit points.

### 1) Bus-driver types and factories

File: `src/buses/BusDriver.h`

- [ ] Add `StaticBusDriverPixelBusT<TTransport, TProtocol>` as canonical name for current `OwningBusDriverPixelBusT` implementation
- [ ] Add `HeapBusDriverPixelBusT<TTransport, TProtocol>` that owns `std::unique_ptr<ProtocolBusDriverT<TProtocol, TTransport>>`
- [ ] Add `makeStaticDriverPixelBus(...)` factory
- [ ] Add `makeHeapDriverPixelBus(...)` factory
- [ ] Remove `makeOwningDriverPixelBus(...)`
- [ ] Remove `OwningBusDriverPixelBusT` symbol and replace with canonical static name
- [ ] Ensure static/heap constructors are the sole owner boundary for transport/protocol/shader graph

Symbols impacted:

- `ProtocolBusDriverT`
- `BusDriverPixelBusT`
- `OwningBusDriverPixelBusT` (rename/remove)
- `makeOwningDriverPixelBus` (rename/remove)
- `makeStaticDriverPixelBus` (new)
- `makeHeapDriverPixelBus` (new)

### 2) Factory default type wiring

File: `src/factory/MakeBus.h`

- [ ] Change `detail::BusSelector<...>::Type` from `OwningBusDriverPixelBusT<...>` to `StaticBusDriverPixelBusT<...>`
- [ ] Update `makeBus(...)` internals to call `makeStaticDriverPixelBus(...)`
- [ ] Preserve current `makeBus(...)` signatures and return behavior
- [ ] Update binding logic that currently depends on `settings.bus = ResourceHandle<ITransport>` to borrowing assignment (`&transport` or reference bind)

Symbols impacted:

- `detail::BusSelector`
- `Bus<...>` alias
- `makeBus(...)` overloads

### 3) Public include/export touchpoints

File: `src/LumaWave.h`

- [ ] Verify new type/factory names are available through the same top-level include path
- [ ] Ensure legacy `Owning*` names are no longer exported

File: `src/LumaWave.h`

- [ ] Verify virtual header exports remain consistent if bus-driver symbols are re-exported there

### 3.1) Composite free-factory removal (completed)

Files:

- `src/buses/ConcatBus.h`
- `src/buses/MosaicBus.h`

Status:

- [x] Removed `makeOwningConcatBus(...)` overloads
- [x] Removed `makeOwningMosaicBus(...)`
- [x] Verified no remaining symbol references

### 4) Documentation updates

Files:

- `docs/internal/factory-function-design-goals.md`
- `docs/internal/post-truncation-example-authoring-plan.md`
- `docs/internal/consumer-virtual-architecture.md`

Checklist:

- [ ] Replace wording that implies `OwningBusDriverPixelBusT` is canonical
- [ ] Document static default and heap alternative
- [ ] Add one example showing explicit heap factory usage

### 5) Tests and validation

Files/areas:

- `test/phase1_smoke/`
- `test/busses/`
- Any compile-only coverage for factory aliases

Checklist:

- [ ] Add/adjust compile tests for `StaticBusDriverPixelBusT`
- [ ] Add compile/runtime test path for `makeHeapDriverPixelBus(...)`
- [ ] Keep existing `makeBus(...)` behavior validation unchanged (still static default)
- [ ] Add compile coverage for borrowed settings forms (`ITransport*` / shader pointer) in major protocol families

### 6) Call-site update sweep

Files/areas:

- `src/**`
- `examples/**`
- `examples-virtual/**`
- `test/**`
- `docs/internal/**`

Checklist:

- [ ] Replace all `OwningBusDriverPixelBusT` references with `StaticBusDriverPixelBusT`
- [ ] Replace all `makeOwningDriverPixelBus(...)` calls with `makeStaticDriverPixelBus(...)`
- [ ] Replace protocol/shader `ResourceHandle` call-site usage with borrowed pointer/reference forms where ownership is now bus-driver scoped
- [ ] Confirm search returns zero references to legacy names (except migration notes if intentionally retained)

### 6.1) `ResourceHandle` usage coverage matrix (exhaustive)

All currently known `ResourceHandle` usage sites are covered below so migration does not miss stragglers.

#### A) Bus-driver + bus core ownership path (convert)

- `src/buses/BusDriver.h` (`ResourceHandle<TDriver>`)
- `src/buses/PixelBus.h` (`ResourceHandle<IProtocol<TColor>>`)

Action:

- [ ] Convert to borrow-first API (`T*` / reference) with ownership centralized in static/heap bus-driver construction

#### B) Protocol transport settings (convert)

Files:

- `src/protocols/DotStarProtocol.h`
- `src/protocols/Hd108Protocol.h`
- `src/protocols/Lpd6803Protocol.h`
- `src/protocols/Lpd8806Protocol.h`
- `src/protocols/NilProtocol.h`
- `src/protocols/P9813Protocol.h`
- `src/protocols/PixieProtocol.h`
- `src/protocols/Sm16716Protocol.h`
- `src/protocols/Sm168xProtocol.h`
- `src/protocols/Tlc5947Protocol.h`
- `src/protocols/Tlc59711Protocol.h`
- `src/protocols/Tm1814Protocol.h`
- `src/protocols/Tm1914Protocol.h`
- `src/protocols/Ws2801Protocol.h`
- `src/protocols/Ws2812xProtocol.h`

Action:

- [ ] Replace `ResourceHandle<ITransport>` fields/params with borrowed pointer/reference forms

#### C) Protocol chaining / shader wrappers (convert)

Files:

- `src/protocols/DebugProtocol.h` (`IProtocol<TColor>*` locked as nullable policy)
- `src/protocols/WithShaderProtocol.h` (`IShader<TColor>*` nullable)
- `src/factory/ProtocolConfigs.h` (debug config entry points currently taking `ResourceHandle<IProtocol<TColor>>`)

Action:

- [ ] Convert optional chain/shader dependencies to nullable pointer semantics (`nullptr` = disabled)

#### D) Composite buses + aggregate shader (retain or selectively convert)

Files:

- `src/buses/ConcatBus.h`
- `src/buses/MosaicBus.h`
- `src/colors/AggregateShader.h`

Action:

- [ ] Decide keep-vs-convert per site; document rationale for any retained `ResourceHandle` mixed-ownership APIs

#### E) Header/include cleanup after migration (audit)

Files:

- `src/protocols/IProtocol.h`
- `src/factory/TransportConfigs.h`
- `src/LumaWave.h`

Action:

- [ ] Remove stale `#include "core/ResourceHandle.h"` where no longer required

#### F) Tests and docs touchpoints (update)

Files/areas:

- `test/busses/test_bus_spec_section1/test_main.cpp`
- `docs/internal/consumer-virtual-architecture.md`
- `docs/internal/protocol-transport-contracts.md`
- `docs/internal/testing-plan-native-unity-arduinofake.md`

Action:

- [ ] Update tests/docs to match borrow-first APIs and any retained composite mixed-ownership behavior

### 6.2) Coverage validation snapshot

Validation run date: 2026-02-25

- Source files containing `ResourceHandle<...>`: **23**
- Files not mapped by this plan: **0**
- Coverage status: **23/23 mapped**

Validation method:

- Enumerate all `src/**` files containing `ResourceHandle<...>`
- Normalize to workspace-relative lowercase paths
- Verify each path appears in this proposal coverage matrix/checklist

### 7) Residual `ResourceHandle` policy

Files/areas:

- `src/buses/ConcatBus.h`
- `src/buses/MosaicBus.h`
- `src/buses/SegmentBus.h` (verify no ownership usage)

Checklist:

- [ ] Decide per remaining `ResourceHandle<T>` site: keep (true mixed ownership/optional chain) vs convert (borrow-only)
- [ ] Document retained `ResourceHandle<T>` sites and rationale
- [ ] Ensure retained usage is outside hot-path protocol update loops where possible
- [ ] Convert `DebugProtocol` optional chaining/settings to nullable pointers (`nullptr` means no chain)

---

## Risks and Trade-offs

- Additional type names increase API surface area slightly.
- Heap variant introduces dynamic allocation path that may be undesirable for constrained targets; this is why static remains default.
- Borrow-first conversion can increase migration churn because settings/constructor signatures change across many protocols.
- Borrowed dependencies require stricter lifetime discipline and clearer construction ordering.

---

## Open Questions

1. Should `HeapBusDriverPixelBusT` be move-only (recommended: yes)?
2. Should `factory::Bus` ever select heap ownership via a policy tag, or should heap remain explicit via dedicated factory?
3. Should this rename land in one atomic PR, or as sequenced PRs where each remains buildable? -- one pr
