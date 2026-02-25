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
3. Keep `ResourceHandle` for protocols/shaders/composites where mixed owning/borrowing is still valuable
4. Update all call sites to new canonical naming (no compatibility aliases)

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
- Direct call-site updates to canonical static/heap APIs

### Out of scope

- Removing `ResourceHandle` globally
- Reworking protocol settings ownership model (`ResourceHandle<ITransport>`, `ResourceHandle<IShader<...>>`)
- Changing external protocol/transport contracts

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
- No protocol/transport setting ownership behavior changes are required.

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

Symbols impacted:

- `detail::BusSelector`
- `Bus<...>` alias
- `makeBus(...)` overloads

### 3) Public include/export touchpoints

File: `src/NeoPixelBus.h`

- [ ] Verify new type/factory names are available through the same top-level include path
- [ ] Ensure legacy `Owning*` names are no longer exported

File: `src/VirtualNeoPixelBus.h`

- [ ] Verify virtual header exports remain consistent if bus-driver symbols are re-exported there

### 4) Documentation updates

Files:

- `docs/factory-function-design-goals.md`
- `docs/post-truncation-example-authoring-plan.md`
- `docs/consumer-virtual-architecture.md`

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

### 6) Call-site update sweep

Files/areas:

- `src/**`
- `examples/**`
- `examples-virtual/**`
- `test/**`
- `docs/**`

Checklist:

- [ ] Replace all `OwningBusDriverPixelBusT` references with `StaticBusDriverPixelBusT`
- [ ] Replace all `makeOwningDriverPixelBus(...)` calls with `makeStaticDriverPixelBus(...)`
- [ ] Confirm search returns zero references to legacy names (except migration notes if intentionally retained)

---

## Risks and Trade-offs

- Additional type names increase API surface area slightly.
- Heap variant introduces dynamic allocation path that may be undesirable for constrained targets; this is why static remains default.
- Delaying deprecation removal avoids churn but prolongs dual naming.

---

## Open Questions

1. Should `HeapBusDriverPixelBusT` be move-only (recommended: yes)?
2. Should `factory::Bus` ever select heap ownership via a policy tag, or should heap remain explicit via dedicated factory?
3. Should this rename land in one atomic PR, or as sequenced PRs where each remains buildable?
