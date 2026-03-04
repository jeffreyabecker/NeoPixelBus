# Static `makeBus` Equivalents for Dynamic Bus Builder Examples

This guide mirrors the examples in `dynamic-bus-builder.md`, but uses static factory composition via `makeBus(...)`.

## Pin Rules (Important)

- Always set `dataPin` explicitly on transport options.
- For two-wire protocol/transport paths, always set `clockPin` explicitly.
- For one-wire protocols, do not set `clockPin`.
- For one-wire configurations, still set `dataPin` explicitly even when the transport can infer defaults.
- When using `PlatformDefault`, pass an explicit `PlatformDefaultOptions` object and set `dataPin`.

## Minimal Static Pattern

```cpp
#include <LumaWave/Factory.h>

PlatformDefaultOptions tx{};
tx.dataPin = 2;
tx.clockPin = 3;

auto bus = makeBus<APA102, PlatformDefault>(
    60,
    DotStarOptions{},
    tx);

bus.begin();
```

## Pre-Build Buffer Planning with `getFactory(...)`

```cpp
#include <LumaWave/Factory.h>

PlatformDefaultOptions tx{};
tx.dataPin = 2;

auto planner = getFactory<Ws2812, PlatformDefault>(
    300,
    Ws2812xOptions{},
    tx);

size_t requiredBytes = planner.getBufferSize();

auto bus = planner.make();
bus.begin();

std::vector<uint8_t> storage(requiredBytes);
auto borrowed = planner.make(storage.data(), static_cast<lw::ssize_t>(storage.size()), false);
```

If you already have a bus instance, `getFactory(bus)` also exposes `getBufferSize()`:

```cpp
auto bus = makeBus<Ws2812, PlatformDefault>(
    300,
    Ws2812xOptions{},
    tx);

auto info = getFactory(bus);
size_t bytes = info.getBufferSize();
```

## One-Wire Manual Timing + 4-Step Cadence + Manual Transport Clock

```cpp
#include <LumaWave/Factory.h>

OneWireTiming customTiming{
    300,   // t0hNs
    900,   // t0lNs
    900,   // t1hNs
    300,   // t1lNs
    50000, // resetNs
    EncodedClockDataBitPattern::FourStep
};

// RP2040 example; use your platform's transport options type.
RpPioOptions tx{};
tx.dataPin = 2;
tx.clockRateHz = 2400000; // explicit override

// One-wire static overloads are timing-first.
auto bus = makeBus<Ws2812T<Rgb8Color>, RpPio>(
    300,
    customTiming,
    tx);
```

Notes:
- If `clockRateHz` is left at `0` for one-wire flows, factory paths derive encoded rate from timing.
- Setting `clockRateHz` explicitly preserves your value.

## Specific Platform-Exclusive Interface

```cpp
#include <LumaWave/Factory.h>

#if defined(ARDUINO_ARCH_RP2040)
RpPioOptions rp{};
rp.dataPin = 6;
rp.pioIndex = 1;

auto panel = makeBus<Ws2812, RpPio>(256, rp);
#endif

#if defined(ARDUINO_ARCH_ESP32)
Esp32RmtOptions rmt{};
rmt.dataPin = 18;
rmt.channel = RMT_CHANNEL_0;

auto panel = makeBus<Ws2812, Esp32Rmt>(256, rmt);
#endif
```

## Print Transport Configuration (ASCII + Debug + Identifier)

```cpp
#include <LumaWave/Factory.h>

NeoPrintOptions printTx{};
printTx.output = &Serial;
printTx.asciiOutput = true;
printTx.debugOutput = true;
printTx.identifier = "bus-a";

using DotStarAnyTransport = DotStar<Rgb8Color, AnyTransportTag>;
auto preview = makeBus<DotStarAnyTransport, NeoPrint>(
    16,
    DotStarOptions{},
    printTx);
```

## Nil Transport Bus

```cpp
#include <LumaWave/Factory.h>

auto dryRun = makeBus<APA102, Nil>(32);
```

## Single Shader on a Bus (Static Boundary)

```cpp
#include <LumaWave/Factory.h>

PlatformDefaultOptions tx{};
tx.dataPin = 2;
tx.clockPin = 3;

auto bus = makeBus<APA102, PlatformDefault>(
    120,
    DotStarOptions{},
    tx);

GammaOptions<Rgb8Color> gamma{};
gamma.gamma = 2.2f;
auto gammaShader = makeShader<Gamma<Rgb8Color>>(gamma);
```

Notes:
- `makeBus(...)` creates buses with the default shader path.
- Build shader instances with `makeShader(...)` and compose them where your app owns composition.

## Hierarchical Shader Stack (Current Static Boundary)

```cpp
#include <LumaWave/Factory.h>

auto gamma = makeShader<Gamma<Rgb8Color>>(GammaOptions<Rgb8Color>{});
auto white = makeShader<WhiteBalance<Rgb8Color>>();
auto limiter = makeShader<CurrentLimiter<Rgb8Color>>();

auto shaderStack = makeShader(
    std::move(gamma),
    std::move(white),
    std::move(limiter));
```

## Aggregate Bus with Linear Topology

```cpp
#include <LumaWave/Factory.h>

PlatformDefaultOptions leftTx{};
leftTx.dataPin = 2;
leftTx.clockPin = 3;

PlatformDefaultOptions rightTx{};
rightTx.dataPin = 4;
rightTx.clockPin = 5;

auto left = makeBus<APA102, PlatformDefault>(100, DotStarOptions{}, leftTx);
auto right = makeBus<APA102, PlatformDefault>(100, DotStarOptions{}, rightTx);

auto wall = makeBus(std::move(left), std::move(right));
```

## Aggregate with Tiled Topology

```cpp
#include <LumaWave/Factory.h>

PlatformDefaultOptions leftTx{};
leftTx.dataPin = 2;
leftTx.clockPin = 3;

PlatformDefaultOptions rightTx{};
rightTx.dataPin = 4;
rightTx.clockPin = 5;

auto left = makeBus<APA102, PlatformDefault>(64, DotStarOptions{}, leftTx);
auto right = makeBus<APA102, PlatformDefault>(64, DotStarOptions{}, rightTx);

TopologySettings topo{};
topo.panelWidth = 8;
topo.panelHeight = 8;
topo.layout = PanelLayout::ColumnMajor;
topo.tilesWide = 2;
topo.tilesHigh = 1;
topo.tileLayout = PanelLayout::RowMajor;
topo.mosaicRotation = true;

auto tiled = makeBus(std::move(topo), std::move(left), std::move(right));
```

## Protocol Configuration Recipes

```cpp
#include <LumaWave/Factory.h>

PlatformDefaultOptions apaTx{};
apaTx.dataPin = 2;
apaTx.clockPin = 3;
auto apa = makeBus<APA102, PlatformDefault>(120, DotStarOptions{}, apaTx);

PlatformDefaultOptions hd108Tx{};
hd108Tx.dataPin = 4;
hd108Tx.clockPin = 5;
auto hd108 = makeBus<HD108, PlatformDefault>(120, DotStarOptions{}, hd108Tx);

PlatformDefaultOptions ws2812Tx{};
ws2812Tx.dataPin = 6;
auto ws2812 = makeBus<Ws2812, PlatformDefault>(120, Ws2812xOptions{}, ws2812Tx);

PlatformDefaultOptions ws2813Tx{};
ws2813Tx.dataPin = 7;
auto ws2813 = makeBus<Ws2813, PlatformDefault>(120, Ws2812xOptions{}, ws2813Tx);

PlatformDefaultOptions tm1829Tx{};
tm1829Tx.dataPin = 11;
auto tm1829 = makeBus<Tm1829, PlatformDefault>(120, Ws2812xOptions{}, tm1829Tx);

PlatformDefaultOptions pixieTx{};
pixieTx.dataPin = 8;
auto pixie = makeBus<PixieProtocol, PlatformDefault>(64, PixieProtocol::SettingsType{}, pixieTx);

PlatformDefaultOptions ucs8903Tx{};
ucs8903Tx.dataPin = 9;
auto ucs8903 = makeBus<Ucs8903, PlatformDefault>(90, Ws2812xOptions{}, ucs8903Tx);

PlatformDefaultOptions ucs8904Tx{};
ucs8904Tx.dataPin = 10;
auto ucs8904 = makeBus<Ucs8904, PlatformDefault>(90, Ws2812xOptions{}, ucs8904Tx);
```

## Larger Interface Color Than `TStripColor`

```cpp
#include <LumaWave/Factory.h>

using WsWire8Interface16 = Ws2812x<
    Rgb16Color,
    ChannelOrder::GRB,
    &timing::Ws2812x,
    Rgb8Color>;

PlatformDefaultOptions tx{};
tx.dataPin = 2;

auto wide = makeBus<WsWire8Interface16, PlatformDefault>(
    128,
    Ws2812xOptions{},
    tx);
```

## One-Wire with Non-Default Channel Order

```cpp
#include <LumaWave/Factory.h>

Ws2812xOptions ws{};
ws.channelOrder = ChannelOrder::RGB::value;

PlatformDefaultOptions tx{};
tx.dataPin = 2;

auto ordered = makeBus<Ws2812T<Rgb8Color>, PlatformDefault>(
    150,
    ws,
    tx);
```

## Final Checklist

- Set `dataPin` on every transport options struct you provide.
- Set `clockPin` whenever your transport/protocol path is two-wire.
- Prefer explicit timing and clock-rate configuration for reproducible output.
- Use `makeBus(std::move(a), std::move(b), ...)` for linear aggregation.
- Use `makeBus(TopologySettings, std::move(a), std::move(b), ...)` for tiled aggregation.
