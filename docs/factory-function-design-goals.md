# Factory Function Design Goals

Status: draft

This document defines the design intent for factory functions so implementation work stays aligned with compile-time allocation goals and clean user ergonomics.

## Primary Goals

- Factory functions simplify construction of bus objects for compile-time allocation-compatible code.
- Factory usage should be expressible as a single construction expression.
- Expose simplified, user-facing type names where possible.
- Prefer type inference wherever possible so users only spell template arguments when inference cannot determine them.
- Naming should be simplified through aliases where type inference can remain clear.
- One exception is color typing: if a color type is a template parameter that cannot be inferred, the user must specify it explicitly.

## Construction Shape

The target call shape is a single `makeBus` expression that composes:

1. Pixel count
2. Protocol config arguments struct
3. Transport config arguments struct
4. Optional shader chain/aggregate
    - Provided either as a shader factory object (`make<TColor>()`) or as a direct shader instance (`IShader<TColor>`)

Reference style:

```cpp
auto bus = makeBus<Ws2812, Debug>(
    8, // Pixel count
    Ws2812{ .colorOrder = "GRB" }, // Protocol configuration arguments struct
    Debug{ .output = nullptr, .invert = false }, // Transport arguments struct
    makeShader(
        makeShader({ .gamma = 2.6f, .enableColorGamma = true, .enableBrightnessGamma = true }),
        makeShader(CurrentLimiter<Ws2812::ColorType>{
            .maxMilliamps = 5000,
            .milliampsPerChannel = ChannelMilliamps{ .R = 20, .G = 20, .B = 20 },
            .controllerMilliamps = 50,
            .standbyMilliampsPerPixel = 1,
            .rgbwDerating = true,
        })));
```

Notes:
- Shader input is optional.
- Shader-enabled calls accept either a shader factory or a direct shader instance.
- Direct shader instances should be safely copyable (copy-constructible and copy-assignable).
- The factory should keep argument roles obvious from type and position.

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

Rules:

1. Protocol and transport should be inferable from config argument struct types whenever possible.
2. Shader type should be inferable from either shader factory return types or direct shader instance type.
3. Color type may be omitted when captured by a simplified protocol alias (for example `Ws2812`).
4. Color type must be explicit when no alias is used and the template parameter cannot be inferred.

Examples:

```cpp
// Inferred protocol + transport via argument structs, color captured by alias.
auto busA = makeBus(
    60,
    Ws2812{ .colorOrder = "GRB" },
    Debug{ .output = nullptr, .invert = false });

// Same call-site shape with a shader factory variable.
auto shaderFactory = makeShader(
    makeShader({ .gamma = 2.6f, .enableColorGamma = true, .enableBrightnessGamma = true }));

auto busB = makeBus(
    60,
    Ws2812{ .colorOrder = "GRB" },
    Debug{ .output = nullptr, .invert = false },
    shaderFactory);

// Same call-site shape with a direct shader instance variable.
GammaShader<Rgb8Color> shader({ .gamma = 2.2f, .enableColorGamma = true, .enableBrightnessGamma = false });

auto busD = makeBus(
    60,
    Ws2812{ .colorOrder = "GRB" },
    Debug{ .output = nullptr, .invert = false },
    shader);

// Explicit color required because raw template form is used and color cannot be inferred.
auto busC = makeBus(
    60,
    Ws2812x<Rgb8Color>{ .colorOrder = "GRB" },
    Debug{ .output = nullptr, .invert = false });
```

### Explicit Bus Type (No `auto`)

Consumers that prefer explicit typing can use the unified `Bus` alias.
Examples below assume `using namespace npb::factory;` (or fully qualified `npb::factory::` names).

```cpp
using BusA = Bus<Ws2812, Debug>;

BusA busA = makeBus(
    60,
    Ws2812{ .colorOrder = "GRB" },
    Debug{ .output = nullptr, .invert = false });

using MyShader = Shader<Ws2812, Gamma, CurrentLimiter<Ws2812::ColorType>>;
using MyBus = Bus<Ws2812, Debug, MyShader>;

MyShader shader = makeShader(
    makeShader({ .gamma = 2.6f, .enableColorGamma = true, .enableBrightnessGamma = true }),
    makeShader(CurrentLimiter<Ws2812::ColorType>{
        .maxMilliamps = 5000,
        .milliampsPerChannel = ChannelMilliamps{ .R = 20, .G = 20, .B = 20 },
        .controllerMilliamps = 50,
        .standbyMilliampsPerPixel = 1,
        .rgbwDerating = true,
    }));

MyBus busB = makeBus(
    60,
    Ws2812{ .colorOrder = "GRB" },
    Debug{ .output = nullptr, .invert = false },
    shader);

MyBus busC = makeBus(
    60,
    Ws2812{ .colorOrder = "GRB" },
    Debug{ .output = nullptr, .invert = false },
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
