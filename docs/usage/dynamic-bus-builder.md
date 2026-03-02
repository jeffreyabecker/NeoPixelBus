# Dynamic Bus Builder Guide

This guide covers public, consumer-facing usage of `DynamicBusBuilder` with `#include <LumaWave.h>`.

## Pin Rules (Important)

- Always set `dataPin` explicitly on transport options.
- For two-wire protocols/transports, always set `clockPin` explicitly.
- For one-wire protocols, do not set `clockPin`.
- For one-wire configurations, still set `dataPin` explicitly even when the transport can infer defaults.
- When using `PlatformDefault`, pass an explicit `PlatformDefaultOptions` object and set `dataPin`.

## Minimal Builder Pattern

```cpp
#include <LumaWave.h>

DynamicBusBuilder<> builder{};

PlatformDefaultOptions tx{};
tx.dataPin = 2;
tx.clockPin = 3;

builder.addBus<APA102, PlatformDefault>(
    "front",
    60,
    DotStarOptions{},
    tx);

auto result = builder.tryBuild<Rgb8Color>("front");
if (result.ok())
{
    result.bus->begin();
}
```

## One-Wire Manual Timing + 4-Step Cadence + Manual Transport Clock

```cpp
#include <LumaWave.h>

DynamicBusBuilder<> builder{};

OneWireTiming customTiming{
    300,  // t0hNs
    900,  // t0lNs
    900,  // t1hNs
    300,  // t1lNs
    50000,// resetNs
    EncodedClockDataBitPattern::FourStep
};

// RP2040 example; use your platform's transport options type.
RpPioOptions tx{};
tx.dataPin = 2;
tx.clockRateHz = 2400000; // explicit override

builder.addBus<Ws2812T<Rgb8Color>, RpPio>(
    "strip",
    300,
    customTiming,
    tx);

auto result = builder.tryBuild<Rgb8Color>("strip");
```

Notes:
- If `clockRateHz` is left at `0` for one-wire flows, the factory path derives the encoded rate from timing.
- Setting `clockRateHz` explicitly preserves your value.
- `clockRateHz` controls encoded bit timing on the transport engine; one-wire protocols still do not use a dedicated `clockPin`.

## Specific Platform-Exclusive Interface

```cpp
#include <LumaWave.h>

DynamicBusBuilder<> builder{};

#if defined(ARDUINO_ARCH_RP2040)
RpPioOptions rp{};
rp.dataPin = 6;
rp.pioIndex = 1;

builder.addBus<Ws2812, RpPio>("panel", 256, rp);
#endif

#if defined(ARDUINO_ARCH_ESP32)
Esp32RmtOneWireOptions rmt{};
rmt.pin = 18;
rmt.channel = RMT_CHANNEL_0;

builder.addBus<Ws2812, Esp32RmtOneWire>("panel", 256, rmt);
#endif
```

## Print Transport Configuration (ASCII + Debug + Identifier)

```cpp
#include <LumaWave.h>

NeoPrintOptions printTx{};
printTx.output = &Serial;
printTx.asciiOutput = true;
printTx.debugOutput = true;
printTx.identifier = "bus-a";
```

Note:
- `NeoPrintOptions` is supported in factory transport traits.
- Protocol/transport category compatibility still applies when building a concrete bus. If your protocol requires `TransportTag`, `NeoPrint` (an `AnyTransportTag` transport) may not be a compatible direct pairing.

## Nil Transport Bus

```cpp
#include <LumaWave.h>

DynamicBusBuilder<> builder{};
builder.addBus<APA102, Nil>("dry-run", 32);

auto result = builder.tryBuild<Rgb8Color>("dry-run");
```

## Single Shader on a Bus (Builder)

```cpp
#include <LumaWave.h>

DynamicBusBuilder<> builder{};

GammaOptions<Rgb8Color> gamma{};
gamma.gamma = 2.2f;

PlatformDefaultOptions tx{};
tx.dataPin = 2;
tx.clockPin = 3;

builder.addBus<APA102, PlatformDefault, Gamma<Rgb8Color>>(
    "front",
    120,
    DotStarOptions{},
    tx,
    gamma);
```

## Hierarchical Shader Stack (Current Builder Boundary)

`DynamicBusBuilder` directly accepts one shader descriptor per bus.
For a hierarchical shader chain, build strands/buses and compose shaders with `makeShader(...)` and composite bus helpers.

## Aggregate Bus with Linear Topology

```cpp
#include <LumaWave.h>

DynamicBusBuilder<> builder{};

PlatformDefaultOptions leftTx{};
leftTx.dataPin = 2;
leftTx.clockPin = 3;

PlatformDefaultOptions rightTx{};
rightTx.dataPin = 4;
rightTx.clockPin = 5;

builder.addBus<APA102, PlatformDefault>("left", 100, DotStarOptions{}, leftTx);
builder.addBus<APA102, PlatformDefault>("right", 100, DotStarOptions{}, rightTx);
builder.addAggregate("wall", {"left", "right"});

auto result = builder.tryBuild<Rgb8Color>("wall");
```

Builder aggregates use linear topology by default.

## Aggregate with Tiled Topology

For tiled aggregate layout, compose built buses with `TopologySettings` using composite bus factories:

```cpp
#include <LumaWave.h>

DynamicBusBuilder<> builder{};

PlatformDefaultOptions leftTx{};
leftTx.dataPin = 2;
leftTx.clockPin = 3;

PlatformDefaultOptions rightTx{};
rightTx.dataPin = 4;
rightTx.clockPin = 5;

builder.addBus<APA102, PlatformDefault>("left", 64, DotStarOptions{}, leftTx);
builder.addBus<APA102, PlatformDefault>("right", 64, DotStarOptions{}, rightTx);

auto left = builder.tryBuild<Rgb8Color>("left");
auto right = builder.tryBuild<Rgb8Color>("right");

TopologySettings topo{};
topo.panelWidth = 8;
topo.panelHeight = 8;
topo.layout = PanelLayout::ColumnMajor;
topo.tilesWide = 2;
topo.tilesHigh = 1;
topo.tileLayout = PanelLayout::RowMajor;
topo.mosaicRotation = true;

auto tiled = makeBus(std::move(topo), std::move(left.bus), std::move(right.bus));
```

## Protocol Configuration Recipes

```cpp
#include <LumaWave.h>

DynamicBusBuilder<> builder{};

PlatformDefaultOptions apaTx{};
apaTx.dataPin = 2;
apaTx.clockPin = 3;

// APA102
builder.addBus<APA102, PlatformDefault>("apa", 120, DotStarOptions{}, apaTx);

PlatformDefaultOptions hd108Tx{};
hd108Tx.dataPin = 4;
hd108Tx.clockPin = 5;

// HD108 (16-bit strip protocol)
builder.addBus<HD108, PlatformDefault>("hd108", 120, DotStarOptions{}, hd108Tx);

PlatformDefaultOptions ws2812Tx{};
ws2812Tx.dataPin = 6;

// Ws2812
builder.addBus<Ws2812, PlatformDefault>("ws2812", 120, Ws2812xOptions{}, ws2812Tx);

PlatformDefaultOptions ws2813Tx{};
ws2813Tx.dataPin = 7;

// Ws2813
builder.addBus<Ws2813, PlatformDefault>("ws2813", 120, Ws2812xOptions{}, ws2813Tx);

// Pixie protocol
PlatformDefaultOptions pixieTx{};
pixieTx.dataPin = 8;
builder.addBus<PixieProtocol, PlatformDefault>("pixie", 64, PixieProtocol::SettingsType{}, pixieTx);

PlatformDefaultOptions ucs8903Tx{};
ucs8903Tx.dataPin = 9;

// Ucs8903 / Ucs8904 descriptors
builder.addBus<Ucs8903, PlatformDefault>("ucs8903", 90, Ws2812xOptions{}, ucs8903Tx);

PlatformDefaultOptions ucs8904Tx{};
ucs8904Tx.dataPin = 10;
builder.addBus<Ucs8904, PlatformDefault>("ucs8904", 90, Ws2812xOptions{}, ucs8904Tx);
```

## Larger Interface Color Than `TStripColor`

```cpp
#include <LumaWave.h>

using WsWire8Interface16 = Ws2812x<
    Rgb16Color,
    ChannelOrder::GRB,
    &timing::Ws2812x,
    Rgb8Color>;

DynamicBusBuilder<> builder{};

PlatformDefaultOptions tx{};
tx.dataPin = 2;

builder.addBus<WsWire8Interface16, PlatformDefault>("wide-interface", 128, Ws2812xOptions{}, tx);

auto result = builder.tryBuild<Rgb16Color>("wide-interface");
```

## One-Wire with Non-Default Channel Order

```cpp
#include <LumaWave.h>

DynamicBusBuilder<> builder{};

Ws2812xOptions ws{};
ws.channelOrder = ChannelOrder::RGB::value;

PlatformDefaultOptions tx{};
tx.dataPin = 2;

builder.addBus<Ws2812T<Rgb8Color>, PlatformDefault>(
    "ordered",
    150,
    ws,
    tx);
```

## Final Checklist

- Set `dataPin` on every transport options struct you provide.
- Set `clockPin` whenever your transport/protocol path is two-wire.
- Prefer explicit timing and clock-rate configuration for reproducible output.
- Use linear aggregate in builder directly; use composite bus topology APIs for tiled aggregate mapping.
