# Factory Function Design Goals

Status: final

This document defines the design intent for factory functions so implementation work stays aligned with compile-time allocation goals and clean user ergonomics.

## Current Implementation Notes

The active implementation currently provides:

1. static bus composition through `StaticBusDriverPixelBusT` (no heap bus-driver factory path),
2. `makeBus` overloads for explicit protocol config, protocol-config omitted, and timing-first one-wire wrapped forms,
3. protocol/transport compatibility gating through `src/buses/BusDriverConstraints.h`.

Direct shader arguments to `makeBus(...)` are not part of the current API surface.

## Primary Goals

- Factory functions simplify construction of bus objects for compile-time allocation-compatible code.
- Factory usage should be expressible as a single construction expression.
- Expose simplified, user-facing type names where possible.
- Prefer type inference wherever possible so users only spell template arguments when inference cannot determine them.
- Prefer template-parameter factory entry points over proliferating type-specific function names.
- Naming should be simplified through aliases where type inference can remain clear.
- One exception is color typing: if a color type is a template parameter that cannot be inferred, the user must specify it explicitly.

## Template-First Factory Naming

Factory APIs should prefer template-based composition forms, for example:

- `makeBus<Ws2812, SpiTransport>(...)`
- `makeShader<Gamma>(...)`

over type-specific function-name variants such as:

- `makeWs2812SpiTransportBus(...)`
- `makeGammaShader(...)`

Rationale:

1. Keeps API surface small and predictable.
2. Preserves consistent call shape across protocol/transport/shader combinations.
3. Avoids combinatorial growth in named helper functions.
4. Improves discoverability by centering composition on a few canonical factory names.

## Example Transport Default

For documentation and reference examples, prefer `SpiTransport` as the default transport choice.

Guidance:

1. Use `SpiTransport` in examples unless the example is explicitly demonstrating a platform-specific transport feature (for example RP2040 PIO, ESP32 RMT, DMA-specific paths, or one-wire timing wrappers).
2. Keep protocol behavior examples focused on protocol/config composition, not transport implementation details, by defaulting to `SpiTransport`.
3. When a non-`SpiTransport` transport is used, include a short note explaining why that transport is required for the example’s purpose.

## Platform-Default Transport Descriptor

The factory system provides:

- `descriptors::PlatformDefault` as a platform-selected transport descriptor alias.
- `PlatformDefaultOptions` as the matching platform-selected transport options alias.

Current mapping:

1. `ARDUINO_ARCH_ESP32` -> `descriptors::Esp32I2s`
2. `ARDUINO_ARCH_ESP8266` -> `descriptors::Esp8266DmaI2s`
3. `ARDUINO_ARCH_RP2040` -> `descriptors::RpPio`
4. `ARDUINO_ARCH_NATIVE` -> `descriptors::Nil`
5. Otherwise -> `descriptors::NeoSpi`

Options mapping follows the same platform selection:

1. `ARDUINO_ARCH_ESP32` -> `Esp32I2sOptions`
2. `ARDUINO_ARCH_ESP8266` -> `Esp8266DmaI2sOptions`
3. `ARDUINO_ARCH_RP2040` -> `RpPioOptions`
4. `ARDUINO_ARCH_NATIVE` -> `NilOptions`
5. Otherwise -> `NeoSpiOptions`

Usage example:

```cpp
auto bus = makeBus<APA102, descriptors::PlatformDefault>(
    60,
    PlatformDefaultOptions{});
```

Platform caveats must still be respected because `PlatformDefault` chooses concrete underlying transports.

Example: `Esp8266DmaI2sTransport` allows pin fields in settings/options, but data output is tied to hardware I2S pin GPIO3.

## Construction Shape

The target call shape is a single `makeBus` expression that composes:

1. Pixel count
2. Protocol config arguments struct
3. Transport config arguments struct

Reference style:

```cpp
auto bus = makeBus<Ws2812, SpiTransport>(
    8, // Pixel count
    Ws2812{ .colorOrder = "GRB" }, // Protocol configuration arguments struct
    SpiTransport{ .spi = &SPI }); // Transport arguments struct
```

Notes:
- Shader factory APIs remain available as a separate composition mechanism.
- The factory should keep argument roles obvious from type and position.

## Planned Composite Construction Shape (Concat Root Ownership)

For root-owned concat composition, document the intended call shape as:

```cpp
auto concatBus = makeBus(
    std::initializer_list<uint16_t>{ 64, 32, 99 },
    makeBus<Ws2812, RpPio>(/* child 0 config */),
    makeBus<Ws2812, RpPio>(/* child 1 config */),
    makeBus<Ws2812, RpPio>(/* child 2 config */));
```

Design intent:

1. The top-most concat bus owns the authoritative pixel buffer.
2. The initializer list defines root slice lengths per child in order.
3. Child factory count must match initializer-list length.
4. Child buses are emit endpoints; hot-path pixel mutation happens through root buffer access.

## OneWireWrapper Overload Goals

Factory APIs should include overloads that make clocked transports usable through `OneWireWrapper` by accepting
`OneWireTiming` as an explicit factory argument.

Required wrapped call shape:

```cpp
auto bus = makeBus<Ws2812, SpiTransport>(
    pixelCount,
    Ws2812{ .colorOrder = "GRB" },
    OneWireTiming::Ws2812,
    SpiTransport{ .spi = &SPI });
```

Ordering requirement:

1. For wrapped one-wire construction, `OneWireTiming` appears before transport config.
2. Do not introduce timing-last variants.

When protocol aliases already capture protocol behavior/settings, protocol config should be omittable in both
wrapped and unwrapped forms:

```cpp
auto clockedBus = makeBus<APA102, SpiTransport>(
    pixelCount,
    SpiTransport{ .spi = &SPI });

auto wrappedBus = makeBus<Ws2812, SpiTransport>(
    pixelCount,
    OneWireTiming::Ws2812,
    SpiTransport{ .spi = &SPI });
```

## Shader Factory Goals

Shader factory functions (for example `makeShader(...)`) should produce concrete shader instances.

Terminology:

- Prefer the term **shader instance** for both direct variables and shader factory return values.
- Avoid the term “shader factory object” in this document unless describing internal implementation details.

Goals:

1. Factory return values are shader instances, not deferred/runtime builder handles.
2. Returned shader instances are copy-constructible and copy-assignable so they can be reused across multiple `makeBus(...)` calls.
3. Shader instance return types preserve compile-time composition and type inference behavior.
4. The call-site shape remains simple and explicit: users pass either a direct shader instance variable or the immediate result of a shader factory call.

Guardrails:

- Do not require heap allocation or runtime polymorphic wrappers for normal shader factory usage.
- Do not introduce alternate shader factory paths that weaken deterministic compile-time composition.

## WS2812x Alias Policy

For the WS2812x family, provide protocol aliases that encapsulate color type to reduce boilerplate in common cases.

Expected alias pattern:

```cpp
using Ws2812 = Ws2812x<Rgb8Color>;
using Sk6812 = Ws2812x<Rgbw8Color>;
using Ucs8904 = Ws2812x<Rgbw16Color>;
```

This alias policy is intended to keep the common path concise while preserving explicit color template selection in uncommon or ambiguous scenarios.

## Inference Rules

Use type inference by default. Require explicit template arguments only when inference cannot uniquely determine a required type.
Inference remains supported, but this document prefers template-first examples for consistency and API-shape clarity.

Rules:

1. Protocol and transport should be inferable from config argument struct types whenever possible.
2. Color type may be omitted when captured by a simplified protocol alias (for example `Ws2812`).
3. Protocol config arguments may be omitted when protocol aliases already encode required protocol behavior/settings.
4. Color type must be explicit when no alias is used and the template parameter cannot be inferred.

Examples:

```cpp
// Template-first style: explicit protocol + transport at the factory call.
auto busA = makeBus<Ws2812, SpiTransport>(
    60,
    Ws2812{ .colorOrder = "GRB" },
    SpiTransport{ .spi = &SPI });

// Alias-encoded protocol settings can allow protocol config omission.
auto busA2 = makeBus<APA102, SpiTransport>(
    60,
    SpiTransport{ .spi = &SPI });

// Explicit color required because raw template form is used and color cannot be inferred.
auto busC = makeBus<Ws2812x<Rgb8Color>, SpiTransport>(
    60,
    Ws2812x<Rgb8Color>{ .colorOrder = "GRB" },
    SpiTransport{ .spi = &SPI });

// Wrapped one-wire form can also omit protocol config when alias settings are sufficient.
auto busE = makeBus<Ws2812, SpiTransport>(
    60,
    OneWireTiming::Ws2812,
    SpiTransport{ .spi = &SPI });
```

## First-Pass Design Contract (Design-Only Gate)

This section defines the exact first implementation pass that must be validated before coding.

### Goal Call Shape (required)

```cpp
auto busA2 = makeBus<APA102, SpiTransport>(
    60,
    SpiTransport{ .spi = &SPI });
```

### First-Pass Scope

1. Add a protocol-config-omitting `makeBus` form for template-first calls when protocol settings are default-constructible.
2. Preserve existing explicit protocol-config call shape:

```cpp
auto busA = makeBus<APA102, SpiTransport>(
    60,
    APA102{},
    SpiTransport{ .spi = &SPI });
```

3. Keep static/compile-time bus construction path; no runtime container or heap-only API path changes.
4. Keep shader behavior unchanged in first pass (no new shader overloads introduced by this pass).

### First-Pass Constraints

1. Omitted protocol-config overload participates only when `TProtocol::SettingsType` is default-constructible.
2. Omitted protocol-config overload must not weaken protocol/transport compatibility constraints.
3. Argument role clarity is preserved: pixel count first, transport config last for unwrapped call shape.
4. One-wire wrapped ordering remains timing-first and unchanged (`..., OneWireTiming, transportConfig`).

### First-Pass Non-Goals

1. No concatenation factory overloads in this pass.
2. No additional type-erased/dynamic factory path.
3. No inference from pixel count, shader config, or unrelated arguments.
4. No timing-last one-wire overload variants.

### Acceptance Criteria Before Merge

1. The goal call shape compiles for `APA102 + SpiTransport`.
2. Existing explicit protocol-config call shape remains valid.
3. Contract compile checks remain green for protocol/transport compatibility.
4. No ambiguity regressions are introduced in overload resolution for existing call shapes.

### Explicit Bus Type (No `auto`)

Consumers that prefer explicit typing can use the unified `Bus` alias.
Examples below assume `using namespace npb::factory;` (or fully qualified `npb::factory::` names).

```cpp
using BusA = Bus<Ws2812, SpiTransport>;

BusA busA = makeBus<Ws2812, SpiTransport>(
    60,
    Ws2812{ .colorOrder = "GRB" },
    SpiTransport{ .spi = &SPI });

using MyShader = Shader<Ws2812, Gamma, CurrentLimiter<Ws2812::ColorType>>;
using MyBus = Bus<Ws2812, SpiTransport, MyShader>;

MyShader shader = makeShader(
    makeShader<Gamma>({ .gamma = 2.6f, .enableColorGamma = true, .enableBrightnessGamma = true }),
    makeShader<CurrentLimiter<Ws2812::ColorType>>(CurrentLimiter<Ws2812::ColorType>{
        .maxMilliamps = 5000,
        .milliampsPerChannel = ChannelMilliamps{ .R = 20, .G = 20, .B = 20 },
        .controllerMilliamps = 50,
        .standbyMilliampsPerPixel = 1,
        .rgbwDerating = true,
    }));

MyBus busB = makeBus<Ws2812, SpiTransport>(
    60,
    Ws2812{ .colorOrder = "GRB" },
    SpiTransport{ .spi = &SPI },
    shader);

MyBus busC = makeBus<Ws2812, SpiTransport>(
    60,
    Ws2812{ .colorOrder = "GRB" },
    SpiTransport{ .spi = &SPI },
    shader);
```

## Non-goals

- Do not add overloads that attempt to infer color type from unrelated arguments.
- Do not infer color type from transport config, shader config, or pixel count.
- Do not expand the factory surface with ambiguous overloads solely to avoid explicit color in edge cases.
- Do not add ambiguous shader overloads; factory and shader-instance paths must remain disambiguated by constraints.
- Keep explicit color selection as the required fallback when protocol aliasing or direct inference is unavailable.

## Guardrails

- Do not optimize for runtime/dynamic allocation convenience at the expense of compile-time allocation compatibility.
- Prefer APIs that preserve type safety and make ownership/lifetime intent obvious.
- Keep the factory surface small and predictable; avoid adding overloads that obscure the common single-expression use case.

## Bus Concatenation Goals (`concatBus` / `makeBus` overload)

Support composing multiple already-constructed buses into one logical bus view.

Target usage:

```cpp
auto shader = makeShader(
    makeShader<Gamma>({ .gamma = 2.6f, .enableColorGamma = true, .enableBrightnessGamma = true }),
    makeShader<CurrentLimiter<Ws2812::ColorType>>(CurrentLimiter<Ws2812::ColorType>{
        .maxMilliamps = 5000,
        .milliampsPerChannel = ChannelMilliamps{ .R = 20, .G = 20, .B = 20 },
        .controllerMilliamps = 50,
        .standbyMilliampsPerPixel = 1,
        .rgbwDerating = true,
    }));

auto busA = makeBus<Ws2812, SpiTransport>(
    8,
    Ws2812{ .colorOrder = "GRB" },
    SpiTransport{ .spi = &SPI },
    shader);

auto busB = makeBus<Ws2812, SpiTransport>(
    8,
    Ws2812{ .colorOrder = "GRB" },
    SpiTransport{ .spi = &SPI },
    shader);

auto busC = makeBus<Ws2812, SpiTransport>(
    8,
    Ws2812{ .colorOrder = "GRB" },
    SpiTransport{ .spi = &SPI },
    shader);

auto singleBus = makeBus(busA, busB, busC); // or concatBus(busA, busB, busC)
```

Rules:

1. All input buses must use the same color type (`TColor`).
2. Concatenated indexing is contiguous across segments.
3. Frame lifecycle calls are forwarded to each segment in order.
4. Concatenation layer is composition-only; no extra shader is applied at concat level.
5. Segment buses keep their own ownership/lifetime; concat object must not outlive them.
6. Keep this API compile-time/static in shape; do not add runtime container-based concat paths.

Naming:

- Prefer `makeBus(busA, busB, ...)` if overload resolution remains unambiguous.
- If ambiguity appears, use explicit `concatBus(...)` as the public name.

## Settings vs Factory Config Separation

The `makeBus(...)` design must preserve a strict boundary between:

1. Protocol/transport **settings objects** (runtime/device behavior knobs), and
2. Factory **configuration inputs** (construction-time composition inputs).

Core intent:

- Users should not be forced to provide every settings field on each call.
- Settings types should support sensible defaults for common paths.
- The factory framework should provide a clear internal participation point for protocol/transport layers to finalize defaults.
- Defaulting logic should be explicit, deterministic, and local to the relevant protocol/transport integration path.
- Keep argument roles clear at the call site: pixel count + protocol config + transport config + optional shader.

Example direction:

- For one-wire paths, integration layers (for example wrapper/adapters) may derive sensible transport defaults from `OneWireTiming`.
- This derived-default behavior should occur in a defined internal normalization step, not by blurring protocol/transport settings with factory composition roles.

Guardrail:

- Do not collapse protocol/transport settings into ambiguous “catch-all” factory config objects; role clarity is a primary API goal.
