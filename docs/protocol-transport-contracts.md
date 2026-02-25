# Protocol and Transport Contracts

This document defines the protocol/transport contract model used by the virtual architecture and how those contracts are enforced in compile-time tests.

## Goals

- Make protocol and transport integration predictable.
- Fail invalid combinations at compile time.
- Keep runtime behavior focused on data flow and timing, not type validation.

## Contract Layers

There are three contract layers:

1. Interface contracts (`IProtocol`, `ITransport`)
2. Concept contracts (type requirements and compatibility rules)
3. Factory/bus-driver contracts (construction and wiring rules)

---

## 1) Interface Contracts

### 1.1 Protocol interface

A protocol must satisfy the `IProtocol<TColor>` behavioral interface:

- `initialize()`
- `update(std::span<const TColor>)`
- `isReadyToUpdate() const`
- `alwaysUpdate() const`

It also provides metadata through aliases:

- `ColorType`
- `SettingsType`
- `TransportCategory`

`SettingsType` and `TransportCategory` are used by compile-time concepts to validate protocol compatibility and constructibility.

### 1.2 Transport interface

A transport must satisfy the `ITransport` behavioral interface:

- `begin()`
- `transmitBytes(std::span<const uint8_t>)`

Optional/default methods:

- `beginTransaction()`
- `endTransaction()`
- `isReadyToUpdate() const` (defaults to `true`)

---

## 2) Concept Contracts

Concepts are the source of truth for compile-time enforcement.

### 2.1 Protocol concepts

- `ProtocolType<TProtocol>`
  - Requires `TProtocol::SettingsType` and `TProtocol::TransportCategory`.

- `ProtocolPixelSettingsConstructible<TProtocol>`
  - Requires non-`void` settings.
  - Requires constructor: `(uint16_t pixelCount, SettingsType settings)`.

- `ProtocolSettingsTransportBindable<TProtocol>`
  - Requires settings support assignment of `settings.bus = ResourceHandle<ITransport>`.

- `ProtocolTransportCompatible<TProtocol, TTransport>`
  - Requires protocol type + transport type + transport category compatibility.

### 2.2 Transport concepts

All transport settings types must expose:

- `public bool invert`

This supports signal polarity inversion introduced by external hardware (for example, a level shifter or inverter stage).

- `TransportLike<TTransport>`
  - Requires `std::derived_from<TTransport, ITransport>`.
  - Requires aliases: `TransportCategory`, `TransportSettingsType`.
  - Requires `TransportSettingsType` to satisfy `TransportSettingsWithInvert`.

- `TaggedTransportLike<TTransport, TTag>`
  - `TransportLike` plus exact category tag match.

- `SettingsConstructibleTransportLike<TTransport>`
  - Requires constructor: `(TransportSettingsType)`.

- `TransportSettingsWithInvert<TTransportSettings>`
  - Requires a public member named `invert` of type `bool`.

### 2.3 Category compatibility rule

Transport categories:

- `AnyTransportTag`
- `TransportTag`
- `OneWireTransportTag`

Compatibility (`TransportCategoryCompatible<ProtocolTag, TransportTag>`) is:

- valid if protocol tag is `AnyTransportTag`, or
- valid if transport tag exactly matches protocol tag.

Implications:

- A protocol tagged `OneWireTransportTag` accepts only `OneWireTransportTag` transports.
- A protocol tagged `TransportTag` accepts only `TransportTag` transports.
- A protocol tagged `AnyTransportTag` accepts any transport category.

---

## 3) Bus Driver and Factory Contracts

### 3.1 Bus driver contracts

`ProtocolBusDriverT` enforces protocol+transport composition through:

- `BusDriverProtocolLike`
- `BusDriverProtocolSettingsConstructible`
- `BusDriverProtocolTransportCompatible`

Protocol construction path is selected with compile-time branching:

1. If protocol settings are bus-bindable, assign `settings.bus = transport` and construct `(pixelCount, settings)`.
2. Else if protocol supports constructor `(pixelCount, settings, transport&)`, use that.
3. Else construct `(pixelCount, settings)`.

This keeps protocol implementations flexible while preserving deterministic construction rules.

### 3.2 Factory contracts

Factories enforce config shape and conversion through traits and concepts:

- `ProtocolConfigTraits<TConfig>` and `FactoryProtocolConfig<TConfig>`
- `TransportConfigTraits<TConfig>` and `FactoryTransportConfig<TConfig>`

`makeBus(...)` is constrained by these concepts, so malformed config types fail at compile time.

Shader-enabled bus creation adds:

- `FactoryShaderForColor<TShaderFactory, TColor>`

which ensures shader factory compatibility with protocol color type.

---

## 4) Compile Contract Test Matrix

The suite `test/contracts/test_protocol_transport_contract_matrix_compile` is the canonical compile-time contract check.

It verifies:

- protocol type contracts for all protocol families,
- transport type contracts for selected transport families,
- positive and negative protocol/transport compatibility pairs,
- factory protocol and transport config concept compliance.

Run:

- `pio test -e native-test --filter contracts/test_protocol_transport_contract_matrix_compile`

---

## 5) Current Known Edge Case

`OneWireWrapper<TTransport>` currently inherits both `ITransport` and `TTransport`.

Because `TTransport` is also constrained to be transport-like, this can create an ambiguous base conversion for some instantiations (for example `OneWireTransport<NilTransport>`), which makes those instantiations fail `TransportLike`.

Current contract matrix behavior intentionally captures this with a negative assertion so the behavior is explicit and stable.

---

## 6) Authoring Checklist

When adding a new protocol:

- Inherit from `IProtocol<TColor>`.
- Define `using ColorType`, `using SettingsType`, `using TransportCategory`.
- Support `(uint16_t, SettingsType)` construction.
- If transport handle binding is needed, expose `settings.bus` assignment compatibility.
- Add compile assertions for the protocol in the contract matrix test.

When adding a new transport:

- Inherit from `ITransport`.
- Define `using TransportCategory`, `using TransportSettingsType`.
- Ensure `TransportSettingsType` has `public bool invert`.
- Support `(TransportSettingsType)` construction.
- Ensure category tag is correct (`TransportTag` vs `OneWireTransportTag`).
- Add compile assertions for the transport in the contract matrix test.

When adding factory config aliases:

- Ensure `ProtocolConfigTraits` or `TransportConfigTraits` resolves concrete type.
- Ensure `toSettings(...)` produces a valid settings object.
- Add concept assertions to the contract matrix test.

---

## 7) Practical Rule of Thumb

- Use protocol/transport tags to encode compatibility intent.
- Use concepts to enforce that intent at compile time.
- Use runtime tests only for timing, byte stream, and state behavior.
