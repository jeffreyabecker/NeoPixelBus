# Factory Function Architecture Plan

Status: final (implementation-aligned)

## Purpose

Define a factory architecture where template parameters are **factory-level descriptors**, not literal protocol/transport/settings concrete types.

This enables:

1. deterministic default initialization,
2. compile-time validation and normalization,
3. stable user-facing call shapes while internals evolve,
4. clear extension seams for aliases, wrappers, policy injection, and migration compatibility.

---

## Core Architectural Principle

Factory APIs use **parallel type parameters** that map to concrete implementation types through traits.

Example intent:

```cpp
auto bus = makeBus<APA102, SpiTransport>(
    60,
    SpiTransport{ .spi = &SPI });
```

In this model:

- `APA102` is a protocol descriptor (factory-facing type token),
- `SpiTransport` is a transport descriptor,
- descriptor-to-concrete resolution happens inside factory traits,
- default settings may be derived/normalized by factory policy.

The call-site stays stable even if concrete internals or defaulting logic change.

## Current Implementation Snapshot

The current codebase implements the following shape:

1. `makeBus` returns `StaticBusDriverPixelBusT<...>` (static ownership/composition path only).
2. `makeBus` supports four forms:
    - explicit protocol + transport config,
    - protocol-config omitted,
    - explicit protocol + timing-first wrapped transport,
    - protocol-config omitted + timing-first wrapped transport.
3. Compatibility checks at factory entry points use `BusDriverProtocolTransportCompatible` and `BusDriverProtocolSettingsConstructible` from `src/buses/BusDriverConstraints.h`.
4. There is no `makeHeapDriverPixelBus(...)` path in the active factory surface.

---

## Layered Model

### 1) Descriptor Layer (factory-facing)

Descriptor types are compile-time identifiers used by users in template arguments.

Responsibilities:

- identify protocol/transport family,
- expose the color contract,
- expose or reference default policy hooks,
- avoid forcing users to spell concrete settings and adapters in the common path.

Examples:

- protocol descriptor: `APA102`, `Ws2812`, `Ws2812x<Rgb8Color>`
- transport descriptor: `SpiTransport`, `RpPioTransport`, `OneWireWrapper<RpPioTransport>`, `OneWireWrapper<SpiTransport>`

### 2) Resolution Layer (traits)

Traits map descriptors to concrete implementation and settings types.

Conceptual shape:

```cpp
template<typename TProtocolDesc>
struct ProtocolDescriptorTraits
{
    using ProtocolType = ...;
    using SettingsType = ...;
    using ColorType = ...;

    static SettingsType defaultSettings();
    static SettingsType normalize(SettingsType in);
};

template<typename TTransportDesc>
struct TransportDescriptorTraits
{
    using TransportType = ...;
    using SettingsType = ...;

    static SettingsType defaultSettings();
    static SettingsType normalize(SettingsType in);
};
```

SPI-focused concrete illustration (descriptor token != required concrete settings literal):

```cpp
// Optional descriptor-level config type accepted by the factory API.
struct NeoSpiOptions
{
    SPIClass* spi = nullptr;
    uint32_t clockRateHz = 0;
    uint8_t bitOrder = static_cast<uint8_t>(MSBFIRST);
    uint8_t dataMode = SPI_MODE0;
    bool invert = false;
};

template<>
struct TransportDescriptorTraits<descriptors::NeoSpi>
{
    using TransportType = SpiTransport;                  // concrete transport implementation
    using SettingsType = SpiTransportSettings;           // concrete settings consumed by TransportType
    using ConfigType = NeoSpiOptions;                    // descriptor-facing config shape (optional)

    static SettingsType defaultSettings()
    {
        return SettingsType{};
    }

    static SettingsType fromConfig(const ConfigType& in)
    {
        SettingsType out{};
        out.spi = in.spi;
        out.clockRateHz = in.clockRateHz;
        out.bitOrder = in.bitOrder;
        out.dataMode = in.dataMode;
        out.invert = in.invert;
        return out;
    }

    static SettingsType normalize(SettingsType in)
    {
        if (in.clockRateHz == 0)
        {
            in.clockRateHz = SpiClockDefaultHz;
        }
        return in;
    }
};
```


Current SPI descriptor token for descriptor-first usage is `descriptors::NeoSpi`.
Template-first call sites can continue using `SpiTransport` where public aliases map to the descriptor path.

Call-site examples with SPI descriptor resolution:

```cpp
// Existing call shape; transport arg may be descriptor-facing config.
auto busA = makeBus<descriptors::APA102, descriptors::NeoSpi>(
    60,
    NeoSpiOptions{ .spi = &SPI, .clockRateHz = 8000000UL });

// Existing call shape; transport arg may also be concrete settings.
auto busB = makeBus<descriptors::APA102, descriptors::NeoSpi>(
    60,
    SpiTransportSettings{ .spi = &SPI, .clockRateHz = 8000000UL });
```

Both forms map through descriptor traits into normalized `SpiTransportSettings` before constructing `SpiTransport`.

APA102 usage example (no protocol implementation code shown):

```cpp
// User-facing call: descriptor tokens in template args.
auto bus = makeBus<descriptors::APA102, descriptors::NeoSpi>(
    60,
    NeoSpiOptions{ .spi = &SPI, .clockRateHz = 10000000UL });
```

Conceptual factory flow for this call:

1. `TProtocolDesc = descriptors::APA102`, `TTransportDesc = descriptors::NeoSpi`.
2. Resolve transport via `TransportDescriptorTraits<descriptors::NeoSpi>`.
3. Convert `NeoSpiOptions` -> `SpiTransportSettings`, then `normalize(...)`.
4. Resolve protocol defaults via `ProtocolDescriptorTraits<descriptors::APA102>::defaultSettings()`.
5. Normalize protocol settings via `ProtocolDescriptorTraits<descriptors::APA102>::normalize(...)`.
6. Validate protocol/transport category compatibility.
7. Construct concrete `SpiTransport` and concrete protocol type resolved from `APA102` descriptor.
8. Return static bus object (`StaticBusDriverPixelBusT<ResolvedTransport, ResolvedProtocol>`).

Variant with omitted transport fields (defaulting path):

```cpp
auto busDefaultClock = makeBus<descriptors::APA102, descriptors::NeoSpi>(
    60,
    NeoSpiOptions{ .spi = &SPI });
```

In this variant, descriptor defaults/normalization provide the SPI clock and other unspecified transport settings.

Responsibilities:

- descriptor -> concrete type mapping,
- safe default settings construction,
- deterministic normalization,
- compile-time category metadata exposure (`TransportTag`, `OneWireTransportTag`, etc.).

### 3) Composition Layer (`makeBus`)

`makeBus` consumes descriptors + optional config inputs, resolves through traits, validates compatibility, and constructs the static bus.

Responsibilities:

- enforce compatibility constraints,
- enforce color alignment,
- apply default/normalize pipeline,
- preserve call-shape clarity and deterministic compile-time composition.

---

## Settings Pipeline (Deterministic)

For each config domain, use a fixed pipeline:

1. `defaultSettings()` from descriptor traits,
2. merge user-provided config (if present),
3. `normalize(...)` for descriptor-specific derivation,
4. construct concrete transport/protocol.

This allows missing fields or omitted config objects while preserving deterministic behavior.

### Protocol Config Omission Rule

If protocol config is omitted, factory uses descriptor defaults from `ProtocolDescriptorTraits<TProtocolDesc>::defaultSettings()`.

### Transport Config Omission Rule

If transport config is omitted (where allowed), factory uses descriptor defaults from `TransportDescriptorTraits<TTransportDesc>::defaultSettings()`.

Note: first-pass implementation can keep transport config required while still using descriptor normalization.

---

## Public API Shape (Target)

### Baseline explicit-config form

```cpp
auto bus = makeBus<ProtocolDesc, TransportDesc>(
    pixelCount,
    protocolConfig,
    transportConfig);
```

### Protocol-config omitted form

```cpp
auto bus = makeBus<ProtocolDesc, TransportDesc>(
    pixelCount,
    transportConfig);
```

### One-wire wrapped timing-first form

```cpp
auto bus = makeBus<ProtocolDesc, TransportDesc>(
    pixelCount,
    protocolConfig,
    OneWireTiming::Ws2812,
    transportConfig);
```

### One-wire wrapped protocol-config omitted form

```cpp
auto bus = makeBus<ProtocolDesc, TransportDesc>(
    pixelCount,
    OneWireTiming::Ws2812,
    transportConfig);
```

Ordering guardrail: timing-first only (`..., OneWireTiming, transportConfig`), never timing-last.

---

## Constraints and Concepts/Traits Contract

Factory entry points must validate:

1. descriptor resolvability (`ProtocolDescriptorTraits`, `TransportDescriptorTraits` specialization exists),
2. protocol/transport category compatibility,
3. color contract consistency,
4. constructibility of resolved concrete types,
5. settings merge/normalize availability for provided config form.

Failure mode should be compile-time with targeted diagnostics near factory entry points.

---

## Descriptor vs Concrete Type Separation Rules

1. Template parameters in `makeBus<...>` are **descriptor tokens**, not required to equal concrete protocol/transport types.
2. Concrete protocol/transport types are an internal output of descriptor resolution.
3. User-provided config objects may be descriptor config types, concrete settings types, or sanctioned adapters, but conversion must be explicit in traits.
4. No reliance on unrelated argument inference for color/settings.

---

## Namespace and Symbol Exposure Plan

Recommended public surface:

- `lw::factory::makeBus`
- descriptor aliases re-exported via public headers for unqualified consumer use where intended
- trait specializations in `lw::factory::detail` or equivalent internal namespace

Rationale:

- keeps factory mechanics isolated,
- avoids polluting core protocol/transport namespaces,
- makes future API growth (`makeShader`, `concatBus`) consistent.

---

## Migration Plan (Incremental)

### Phase A — Descriptor traits scaffolding

- add protocol/transport descriptor trait contracts,
- map initial descriptors (`APA102`, `Ws2812`, `SpiTransport`),
- preserve existing lower-level constructor behavior.

### Phase B — First-pass `makeBus` path

- implement baseline + protocol-omitted forms,
- keep shader and concat overloads unchanged,
- keep static bus construction path (`StaticBusDriverPixelBusT`).

### Phase C — One-wire timing integration

- route timing-first overloads through descriptor normalization,
- avoid timing-last overloads.

### Phase D — Shader and concat alignment

- align shader descriptor/factory semantics to same descriptor model,
- align concat construction naming decision (`makeBus(...)` vs `concatBus(...)`) without ambiguity regressions.

---

## Validation Plan

### Compile-contract coverage

Add compile assertions for:

1. descriptor resolvability,
2. compatibility pass/fail matrix,
3. call-shape validity for omitted protocol config,
4. no ambiguity between explicit and omitted overloads,
5. one-wire timing-first acceptance and timing-last rejection.

### Runtime smoke coverage

- basic construction + pixel count sanity for representative descriptor pairs,
- no behavior regression in protocol update lifecycle.

Target gates:

- `pio test -e native-test`
- `pio test -e native-test --filter contracts/test_protocol_transport_contract_matrix_compile`

---

## Non-Goals (for initial architecture rollout)

1. runtime type-erased factory registry,
2. heap-only construction paths,
3. inferred color typing from unrelated arguments,
4. broad overload expansion beyond defined call-shape set.

---

## Risks and Mitigations

### Risk: overload ambiguity

Mitigation:

- strict SFINAE/trait gates,
- explicit timing position,
- compile tests that validate negative cases.

### Risk: hidden default behavior surprises

Mitigation:

- defaults centralized in descriptor traits,
- normalization rules documented per descriptor,
- deterministic pipeline order documented and tested.

### Risk: descriptor/concrete mismatch confusion

Mitigation:

- explicit terminology in docs: descriptor vs concrete vs settings,
- naming convention for traits and aliases,
- compile-time diagnostics that mention descriptor names.

---

## First Implementation Slice (recommended)

Deliver only:

1. descriptor trait scaffolding for `APA102` + `SpiTransport`,
2. `makeBus<ProtocolDesc, TransportDesc>(pixelCount, transportConfig)` (protocol omitted),
3. `makeBus<ProtocolDesc, TransportDesc>(pixelCount, protocolConfig, transportConfig)` (explicit),
4. compile tests for these two forms and compatibility checks.

This keeps scope narrow while proving the core descriptor architecture.
