# Dynamic Bus Builder INI Spec (Draft)

This document proposes an INI-based spec system for configuring `DynamicBusBuilder` via `IniReader`.
It is designed to mirror the cases in the dynamic builder usage guide while keeping parsing simple and explicit.

## Goals

- Human-editable runtime config for bus/shader/aggregate composition
- No `std::string` parser dependency in config reader state
- Clear mapping from INI keys to `DynamicBusBuilder` calls
- Explicit pin and transport settings to avoid hidden defaults

## Pin Rules (Normative)

- Always set a `data-pin` for every real transport.
- For two-wire protocol/transport paths, always set `clock-pin`.
- For one-wire protocols, do not set `clock-pin`.
- For `transport=platform-default`, set `data-pin` explicitly.

## Top-Level Shape

- Node sections: `[bus:<name>]`
- Optional reusable presets: `[protocol:<name>]`, `[transport:<name>]`, `[shader:<name>]`, `[topology:<name>]`

Each `bus:<name>` is a node. A node can be either:

- a strand-producing bus node (`kind=bus`, default)
- a composition/aggregate node (`kind=aggregate`)

## Core Keys

### Node keys (`[bus:<name>]`)

- `kind=<bus|aggregate>` (optional; default `bus`)

- `pixels=<uint16>`
- `protocol=<token|@name>`
- `transport=<token|@name>`
- `shader=<token|@name>` (optional, single shader)
- `protocol:channel-order=<token>` (optional)
- `transport:data-pin=<int>` (optional at bus scope; required for non-nil/non-print transports after preset merge)
- `transport:clock-pin=<int>` (optional at bus scope; required for two-wire protocols/transports after preset merge)
- `transport:clock-rate-hz=<uint32>` (optional)
- `transport:invert=<bool>` (optional)
- `shader:*` (optional shader settings when `shader=` is set)

Preset reference rules:

- `@name` means "load keys from `[protocol:name]` / `[transport:name]` / `[shader:name]`".
- Node-local keys override preset keys (`protocol:*`, `transport:*`, `shader:*`).
- When `protocol=@name` and/or `transport=@name` is used, all bus-local `protocol:*` and `transport:*` keys are optional.
- Required key checks (`data-pin`, `clock-pin`, one-wire/two-wire constraints) apply to the final merged configuration.
- Unknown preset names are parse errors.

Aggregate-only keys (when `kind=aggregate`):

- `children=<name|name|...>`
- `topology=<linear|tiled>`
- If `topology=tiled`, include:
  - `panel-width`, `panel-height`, `layout`, `tiles-wide`, `tiles-high`, `tile-layout`, `mosaic-rotation`
  - Optional layout modifiers:
    - `layout-mode=<progressive|serpentine>` (default `progressive`)
    - `layout-rotation=<0|90|180|270>` (default `0`)
    - `tile-layout-mode=<progressive|serpentine>` (default `progressive`)
    - `tile-layout-rotation=<0|90|180|270>` (default `0`)
  - `layout` and `tile-layout` base tokens use kebab-case: `row-major` or `column-major`

### One-wire timing keys (bus-level)

- `protocol:timing.t0h-ns`, `protocol:timing.t0l-ns`, `protocol:timing.t1h-ns`, `protocol:timing.t1l-ns`, `protocol:timing.reset-ns`
- `protocol:timing.cadence=<3step|4step>`

### Print transport keys (bus-level)

- `transport:print.output=<serial>`
- `transport:print.ascii-output=<bool>`
- `transport:print.debug-output=<bool>`
- `transport:print.identifier=<text>`

## Descriptor/Traits Token Export Contract

To keep INI token parsing in sync with public factory capabilities, each protocol/transport/shader descriptor trait should expose canonical tokens and aliases.

Proposed trait contract shape:

- `static constexpr const char *PrimaryToken`
- `static constexpr const char *const Tokens[]`
- Optional helper: `static constexpr span<const char *const> tokenList()`

`Tokens[]` must include `PrimaryToken` first.

`Ini` token registries should be generated from these exports (not duplicated manually).

## INI Token Registry Generation

- Protocol token registry is built by aggregating protocol-descriptor token exports.
- Transport token registry is built by aggregating transport-descriptor token exports.
- Shader token registry is built by aggregating shader-descriptor token exports.
- Parsing is case-insensitive and matches any token in the exported list.

## Protocol Tokens

Derived from descriptor trait token exports (no hard-coded duplicate list in parser logic).

Canonical examples:

- `apa102`
- `hd108`
- `ws2812`
- `ws2813`
- `ucs8903`
- `ucs8904`
- `pixie`
- Generic: `ws2812x`

## Transport Tokens

- `platform-default`
- `nil`
- Platform-specific examples: `rp-pio`, `rp-spi`, `rp-uart`, `esp32-rmt-onewire`, `neoprint`
- `neoprint` aliases: `print`, `serial`, `debug`

## Shader Tokens

- `none`
- `gamma`
- `current-limiter`
- `white-balance`
- Composite/hierarchical shader chain is represented as a list and resolved outside direct `DynamicBusBuilder` single-shader overload.

## Reusable Preset Sections

### Protocol preset (`[protocol:<name>]`)

```ini
[protocol:ws-default]
token=ws2812
channel-order=grb
```

### Transport preset (`[transport:<name>]`)

```ini
[transport:strip-a]
token=platform-default
data-pin=2
```

### Shader preset (`[shader:<name>]`)

```ini
[shader:soft-gamma]
token=gamma
gamma=2.2
enable-color-gamma=true
enable-brightness-gamma=false
```

Using presets from a bus node:

```ini
[bus:front]
pixels=120
protocol=@ws-default
transport=@strip-a
shader=@soft-gamma

# Override selected preset values at bus scope:
protocol:channel-order=rgb
transport:data-pin=7
shader:gamma=2.4
```

---

## Minimal Bus

```ini
[bus:front]
pixels=60
protocol=apa102
transport=platform-default
transport:data-pin=2
transport:clock-pin=3
```

## One-Wire Manual Timing + 4-Step Cadence + Manual Transport Clock

```ini
[bus:strip]
pixels=300
protocol=ws2812
transport=rp-pio
transport:data-pin=2
transport:clock-rate-hz=2400000

protocol:timing.t0h-ns=300
protocol:timing.t0l-ns=900
protocol:timing.t1h-ns=900
protocol:timing.t1l-ns=300
protocol:timing.reset-ns=50000
protocol:timing.cadence=4step
```

## Specific Platform-Exclusive Interface

```ini
[bus:panel-rp]
pixels=256
protocol=ws2812
transport=rp-pio
transport:data-pin=6
transport:pio-index=1

[bus:panel-esp32]
pixels=256
protocol=ws2812
transport=esp32-rmt-onewire
transport:pin=18
transport:channel=0
```

## Print Transport to Serial (`ascii-output`, `debug-output`, `identifier`)

```ini
[bus:console]
pixels=16
protocol=debug
transport=serial
transport:print.output=serial
transport:print.ascii-output=true
transport:print.debug-output=true
transport:print.identifier=bus-a
```

## Nil Transport Bus

```ini
[bus:dry-run]
pixels=32
protocol=apa102
transport=nil
```

## Single Shader Bus

```ini
[bus:front-shaded]
pixels=120
protocol=apa102
transport=platform-default
transport:data-pin=2
transport:clock-pin=3
shader=gamma
shader:gamma=2.2
shader:enable-color-gamma=true
shader:enable-brightness-gamma=false
```

## Hierarchical Shader Stack

```ini
[bus:front-stack]
pixels=120
protocol=apa102
transport=platform-default
transport:data-pin=2
transport:clock-pin=3
shaders=gamma|white-balance|current-limiter

[shader:gamma]
type=gamma
gamma=2.2
enable-color-gamma=true
enable-brightness-gamma=false

[shader:white]
type=white-balance

[shader:limiter]
type=current-limiter
```

Implementation note: `DynamicBusBuilder` directly supports one shader descriptor; multi-shader stacks are resolved by composing shaders around built strands/buses.

## Aggregate Bus with Linear Topology

```ini
[bus:left]
pixels=100
protocol=apa102
transport=platform-default
transport:data-pin=2
transport:clock-pin=3

[bus:right]
pixels=100
protocol=apa102
transport=platform-default
transport:data-pin=4
transport:clock-pin=5

[bus:wall]
kind=aggregate
children=left|right
topology=linear
```

## Aggregate Bus with Tiled Topology

```ini
[bus:left]
pixels=64
protocol=apa102
transport=platform-default
transport:data-pin=2
transport:clock-pin=3

[bus:right]
pixels=64
protocol=apa102
transport=platform-default
transport:data-pin=4
transport:clock-pin=5

[bus:mosaic]
kind=aggregate
children=left|right
topology=tiled
panel-width=8
panel-height=8
layout=column-major
tiles-wide=2
tiles-high=1
tile-layout=row-major
mosaic-rotation=true
```

## Pixie Bus

```ini
[bus:pixie]
pixels=64
protocol=pixie
transport=platform-default
transport:data-pin=8
```

## Larger Interface Color Than `TStripColor`

```ini
[bus:wide-interface]
pixels=128
protocol=ws2812x
transport=platform-default
transport:data-pin=2
protocol:interface-color=rgb16
protocol:strip-color=rgb8
protocol:channel-order=grb
```

## APA102

```ini
[bus:apa]
pixels=120
protocol=apa102
transport=platform-default
transport:data-pin=2
transport:clock-pin=3
```

## HD108

```ini
[bus:hd108]
pixels=120
protocol=hd108
transport=platform-default
transport:data-pin=4
transport:clock-pin=5
```

## Ws2812

```ini
[bus:ws2812]
pixels=120
protocol=ws2812
transport=platform-default
transport:data-pin=6
```

## Ws2813

```ini
[bus:ws2813]
pixels=120
protocol=ws2813
transport=platform-default
transport:data-pin=7
```

## Ucs8903

```ini
[bus:ucs8903]
pixels=90
protocol=ucs8903
transport=platform-default
transport:data-pin=9
```

## Ucs8904

```ini
[bus:ucs8904]
pixels=90
protocol=ucs8904
transport=platform-default
transport:data-pin=10
```

## One-Wire with Non-Default Channel Order

```ini
[bus:ordered]
pixels=150
protocol=ws2812
transport=platform-default
transport:data-pin=2
protocol:channel-order=rgb
```

---

## Parse + Build Flow (Reference)

```cpp
#include <LumaWave.h>

void buildFromIni(char *text, size_t length)
{
    auto reader = IniReader::parse(span<char>{text, length});

    DynamicBusBuilder<> builder{};

    // 1) Parse [bus:*] sections
    // 2) Build token registries from descriptor/trait token exports
    // 3) Resolve @preset references from [protocol:*], [transport:*], [shader:*]
    // 4) For kind=bus, call addBus<...>(...)
    // 5) For kind=aggregate, call addAggregate(...)
    //    - Use topology overload when topology=tiled
    // 6) Select root node and call tryBuild<TColor>(name)
}
```

## Validation Recommendations

- Reject one-wire sections that set `clock-pin`.
- Reject two-wire sections missing `clock-pin`.
- Reject non-nil transports missing `data-pin`.
- Validate `children` references and detect cycles before build.
- Validate aggregate tiled topology pixel count matches summed child pixels when your app requires strict dimensional consistency.

## Defaults Policy (Recommended)

- Treat missing or invalid values as parse errors for required keys.
- Keep fallback defaults only for explicitly optional keys (for example `invert=false`, no shader).
- Prefer explicit `channel-order` for clarity when protocol family allows it.
