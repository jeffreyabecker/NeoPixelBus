// ...existing code...

## Settings vs Factory Config Separation

The `makeBus(...)` design preserves a strict boundary between:

1. Protocol/transport **type selection** (compile-time factory composition), and
2. Protocol/transport **settings objects** (runtime/device behavior knobs).

### Design

#### Settings Types

Each protocol and transport layer defines a dedicated settings struct with defaulted fields:

```cpp
// Protocol settings — all fields have sensible defaults
struct Ws2812Settings
{
    const char* colorOrder = "GRB";
};

struct Ws2812xSettings
{
    const char* colorOrder = "GRB";
};

// Transport settings — all fields have sensible defaults
struct DebugTransportSettings
{
    Stream* output = nullptr;
    bool invert = false;
};

struct Rp2040PioTransportSettings
{
    uint8_t pin = 0;
    bool invert = false;
};
```

Key rules for settings types:

- Every field must have a default value, so `{}` is always a valid construction.
- Transport settings must include `public bool invert` per the transport contract.
- Settings types are plain aggregates (no constructors, no inheritance, no virtual).
- Settings types carry no type-tag role; they are not used for factory overload resolution.

#### Protocol and Transport Type Tags

Protocol and transport **identity** is conveyed via the protocol/transport types themselves (e.g., `Ws2812`, `Debug`), used as template arguments. These types expose a nested `Settings` alias:

```cpp
// Protocol type (alias for common color path)
// Ws2812 = Ws2812x<Rgb8Color>
// Ws2812::Settings -> Ws2812xSettings

// Transport type
// Debug::Settings -> DebugTransportSettings
```

#### Factory Overloads

`makeBus` uses protocol and transport **types** for overload resolution, and accepts **optional** settings:

```cpp
// Full explicit settings
auto busA = makeBus<Ws2812, Debug>(
    60,
    Ws2812Settings{ .colorOrder = "GRB" },
    DebugTransportSettings{ .output = &Serial, .invert = false });

// All defaults — settings omitted entirely
auto busB = makeBus<Ws2812, Debug>(60);

// Protocol defaults, transport explicit
auto busC = makeBus<Ws2812, Debug>(
    60,
    DebugTransportSettings{ .output = &Serial });

// Transport defaults, protocol explicit
auto busD = makeBus<Ws2812, Debug>(
    60,
    Ws2812Settings{ .colorOrder = "RGB" });
```

Overload resolution rules:

1. Protocol and transport types are **always** template arguments (never inferred from settings structs).
2. Settings arguments are resolved by type, not position (after pixel count).
3. If a settings argument for a layer is absent, the factory constructs `TProtocol::Settings{}` or `TTransport::Settings{}` internally.
4. At most one protocol settings and one transport settings argument may appear; duplicates are a compile error.

#### One-Wire Timing Normalization

For one-wire protocol/transport paths, the factory provides an internal normalization step:

```cpp
// OneWireTiming is a protocol-level concept.
// When the transport is a OneWireTransport category, the factory integration
// layer may derive transport timing defaults from the protocol's OneWireTiming.
//
// This happens inside the factory — not at the call site.
```

The user never manually threads timing through transport settings. The factory's internal `normalizeSettings(protocolSettings, transportSettings, OneWireTiming)` step fills transport timing fields from the protocol's timing definition when the transport has not overridden them.

```cpp
// User writes:
auto bus = makeBus<Ws2812, Rp2040Pio>(
    60,
    Rp2040PioTransportSettings{ .pin = 2 });

// Factory internally:
// 1. Constructs Ws2812Settings{} (default, since omitted)
// 2. Takes user's Rp2040PioTransportSettings{ .pin = 2 }
// 3. Calls normalizeOneWireTransport(transportSettings, Ws2812::Timing)
//    which fills timing fields the user didn't set
// 4. Constructs protocol, transport, bus
```

#### With Shaders

Shader attachment remains positionally last and optional, unchanged from current design:

```cpp
auto bus = makeBus<Ws2812, Debug>(
    60,
    Ws2812Settings{ .colorOrder = "GRB" },
    DebugTransportSettings{ .output = nullptr },
    makeShader(
        makeShader({ .gamma = 2.6f, .enableColorGamma = true, .enableBrightnessGamma = true })));

// Or with all defaults + shader:
auto bus2 = makeBus<Ws2812, Debug>(
    60,
    makeShader(
        makeShader({ .gamma = 2.6f, .enableColorGamma = true, .enableBrightnessGamma = true })));
```

### Disambiguation Strategy

Since settings arguments are now optional and type-matched, the factory must distinguish between a settings struct and a shader argument. Rules:

1. Arguments after pixel count that satisfy `is_protocol_settings_v<T>` are protocol settings.
2. Arguments that satisfy `is_transport_settings_v<T>` are transport settings.
3. Arguments that satisfy `is_shader_v<T>` or `is_shader_factory_v<T>` are shader inputs.
4. These traits are mutually exclusive by design (settings types never model shader concepts).

### Explicit Bus Type (No `auto`)

```cpp
using MyBus = Bus<Ws2812, Debug>;

MyBus bus = makeBus<Ws2812, Debug>(60);

MyBus bus2 = makeBus<Ws2812, Debug>(
    60,
    Ws2812Settings{ .colorOrder = "RGB" },
    DebugTransportSettings{ .output = &Serial });
```

### Summary Table

| Argument | Role | Required? | Resolution |
|----------|------|-----------|------------|
| `<Ws2812, Debug>` | Protocol + Transport type selection | Yes (template) | Compile-time |
| `60` | Pixel count | Yes (positional, first) | Value |
| `Ws2812Settings{...}` | Protocol runtime knobs | No; defaults to `{}` | Type-matched |
| `DebugTransportSettings{...}` | Transport runtime knobs | No; defaults to `{}` | Type-matched |
| `shader` / `makeShader(...)` | Shader chain | No | Type-matched (last) |

### Guardrails

- Do not collapse protocol/transport settings into ambiguous "catch-all" factory config objects.
- Do not use settings types for factory overload resolution or type inference.
- Do not require namespace-qualified settings types at the example level; re-export through `NeoPixelBus.h`.
- Settings defaults must be deterministic and local to the defining protocol/transport.
- The internal normalization step (e.g., one-wire timing) must be explicit and documented, not implicit side-effect behavior.