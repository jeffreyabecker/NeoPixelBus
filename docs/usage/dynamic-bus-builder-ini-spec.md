# Dynamic Bus Builder INI Spec (Draft)

This document proposes an INI-based spec system for configuring `DynamicBusBuilder` via `IniReader`.
It is designed to mirror the cases in the dynamic builder usage guide while keeping parsing simple and explicit.

## Goals

- Human-editable runtime config for bus/shader/aggregate composition
- No `std::string` parser dependency in config reader state
- Clear mapping from INI keys to `DynamicBusBuilder` calls
- Explicit pin and transport settings to avoid hidden defaults

## Pin Rules (Normative)

- Always set a `dataPin` for every real transport.
- For two-wire protocol/transport paths, always set `clockPin`.
- For one-wire protocols, do not set `clockPin`.
- For `transport=platformdefault`, set `dataPin` explicitly.

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
- `protocol:channelOrder=<token>` (optional)
- `transport:dataPin=<int>` (optional at bus scope; required for non-nil/non-print transports after preset merge)
- `transport:clockPin=<int>` (optional at bus scope; required for two-wire protocols/transports after preset merge)
- `transport:clockRateHz=<uint32>` (optional)
- `transport:invert=<bool>` (optional)
- `shader:*` (optional shader settings when `shader=` is set)

Preset reference rules:

- `@name` means "load keys from `[protocol:name]` / `[transport:name]` / `[shader:name]`".
- Node-local keys override preset keys (`protocol:*`, `transport:*`, `shader:*`).
- When `protocol=@name` and/or `transport=@name` is used, all bus-local `protocol:*` and `transport:*` keys are optional.
- Required key checks (`dataPin`, `clockPin`, one-wire/two-wire constraints) apply to the final merged configuration.
- Unknown preset names are parse errors.

Aggregate-only keys (when `kind=aggregate`):

- `children=<name|name|...>`
- `topology=<linear|tiled>`
- If `topology=tiled`, include:
  - `panelWidth`, `panelHeight`, `layout`, `tilesWide`, `tilesHigh`, `tileLayout`, `mosaicRotation`

### One-wire timing keys (bus-level)

- `protocol:timing.t0hNs`, `protocol:timing.t0lNs`, `protocol:timing.t1hNs`, `protocol:timing.t1lNs`, `protocol:timing.resetNs`
- `protocol:timing.cadence=<3step|4step>`

### Print transport keys (bus-level)

- `transport:print.output=<serial>`
- `transport:print.asciiOutput=<bool>`
- `transport:print.debugOutput=<bool>`
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

- `platformdefault`
- `nil`
- Platform-specific examples: `rppio`, `rpspi`, `rpuart`, `esp32rmtonewire`, `neoprint`
- `neoprint` aliases: `print`, `serial`, `debug`

## Shader Tokens

- `none`
- `gamma`
- `currentLimiter`
- `whiteBalance`
- Composite/hierarchical shader chain is represented as a list and resolved outside direct `DynamicBusBuilder` single-shader overload.

## Reusable Preset Sections

### Protocol preset (`[protocol:<name>]`)

```ini
[protocol:ws-default]
token=ws2812
channelOrder=grb
```

### Transport preset (`[transport:<name>]`)

```ini
[transport:strip-a]
token=platformdefault
dataPin=2
```

### Shader preset (`[shader:<name>]`)

```ini
[shader:soft-gamma]
token=gamma
gamma=2.2
enableColorGamma=true
enableBrightnessGamma=false
```

Using presets from a bus node:

```ini
[bus:front]
pixels=120
protocol=@ws-default
transport=@strip-a
shader=@soft-gamma

# Override selected preset values at bus scope:
protocol:channelOrder=rgb
transport:dataPin=7
shader:gamma=2.4
```

---

## Minimal Bus

```ini
[bus:front]
pixels=60
protocol=apa102
transport=platformdefault
transport:dataPin=2
transport:clockPin=3
```

## One-Wire Manual Timing + 4-Step Cadence + Manual Transport Clock

```ini
[bus:strip]
pixels=300
protocol=ws2812
transport=rppio
transport:dataPin=2
transport:clockRateHz=2400000

protocol:timing.t0hNs=300
protocol:timing.t0lNs=900
protocol:timing.t1hNs=900
protocol:timing.t1lNs=300
protocol:timing.resetNs=50000
protocol:timing.cadence=4step
```

## Specific Platform-Exclusive Interface

```ini
[bus:panel-rp]
pixels=256
protocol=ws2812
transport=rppio
transport:dataPin=6
transport:pioIndex=1

[bus:panel-esp32]
pixels=256
protocol=ws2812
transport=esp32rmtonewire
transport:pin=18
transport:channel=0
```

## Print Transport to Serial (`asciiOutput`, `debugOutput`, `identifier`)

```ini
[bus:console]
pixels=16
protocol=debug
transport=serial
transport:print.output=serial
transport:print.asciiOutput=true
transport:print.debugOutput=true
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
transport=platformdefault
transport:dataPin=2
transport:clockPin=3
shader=gamma
shader:gamma=2.2
shader:enableColorGamma=true
shader:enableBrightnessGamma=false
```

## Hierarchical Shader Stack

```ini
[bus:front-stack]
pixels=120
protocol=apa102
transport=platformdefault
transport:dataPin=2
transport:clockPin=3
shaders=gamma|whiteBalance|currentLimiter

[shader:gamma]
type=gamma
gamma=2.2
enableColorGamma=true
enableBrightnessGamma=false

[shader:white]
type=whiteBalance

[shader:limiter]
type=currentLimiter
```

Implementation note: `DynamicBusBuilder` directly supports one shader descriptor; multi-shader stacks are resolved by composing shaders around built strands/buses.

## Aggregate Bus with Linear Topology

```ini
[bus:left]
pixels=100
protocol=apa102
transport=platformdefault
transport:dataPin=2
transport:clockPin=3

[bus:right]
pixels=100
protocol=apa102
transport=platformdefault
transport:dataPin=4
transport:clockPin=5

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
transport=platformdefault
transport:dataPin=2
transport:clockPin=3

[bus:right]
pixels=64
protocol=apa102
transport=platformdefault
transport:dataPin=4
transport:clockPin=5

[bus:mosaic]
kind=aggregate
children=left|right
topology=tiled
panelWidth=8
panelHeight=8
layout=columnmajor
tilesWide=2
tilesHigh=1
tileLayout=rowmajor
mosaicRotation=true
```

## Pixie Bus

```ini
[bus:pixie]
pixels=64
protocol=pixie
transport=platformdefault
transport:dataPin=8
```

## Larger Interface Color Than `TStripColor`

```ini
[bus:wide-interface]
pixels=128
protocol=ws2812x
transport=platformdefault
transport:dataPin=2
protocol:interfaceColor=rgb16
protocol:stripColor=rgb8
protocol:channelOrder=grb
```

## APA102

```ini
[bus:apa]
pixels=120
protocol=apa102
transport=platformdefault
transport:dataPin=2
transport:clockPin=3
```

## HD108

```ini
[bus:hd108]
pixels=120
protocol=hd108
transport=platformdefault
transport:dataPin=4
transport:clockPin=5
```

## Ws2812

```ini
[bus:ws2812]
pixels=120
protocol=ws2812
transport=platformdefault
transport:dataPin=6
```

## Ws2813

```ini
[bus:ws2813]
pixels=120
protocol=ws2813
transport=platformdefault
transport:dataPin=7
```

## Ucs8903

```ini
[bus:ucs8903]
pixels=90
protocol=ucs8903
transport=platformdefault
transport:dataPin=9
```

## Ucs8904

```ini
[bus:ucs8904]
pixels=90
protocol=ucs8904
transport=platformdefault
transport:dataPin=10
```

## One-Wire with Non-Default Channel Order

```ini
[bus:ordered]
pixels=150
protocol=ws2812
transport=platformdefault
transport:dataPin=2
protocol:channelOrder=rgb
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

- Reject one-wire sections that set `clockPin`.
- Reject two-wire sections missing `clockPin`.
- Reject non-nil transports missing `dataPin`.
- Validate `children` references and detect cycles before build.
- Validate aggregate tiled topology pixel count matches summed child pixels when your app requires strict dimensional consistency.

## Defaults Policy (Recommended)

- Treat missing or invalid values as parse errors for required keys.
- Keep fallback defaults only for explicitly optional keys (for example `invert=false`, no shader).
- Prefer explicit `channelOrder` for clarity when protocol family allows it.
