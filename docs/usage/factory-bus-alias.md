# Factory Bus Alias Example

This page shows how to use the `Bus<...>` helper alias so you can name the concrete bus type without writing the full return type.

## Basic Alias

```cpp
#include <LumaWave/Factory.h>

using BusType = Bus<Ws2812, PlatformDefault>;

BusType makeStrip(uint16_t pixelCount)
{
    return makeBus<Ws2812, PlatformDefault>(
        pixelCount,
        PlatformDefaultOptions{});
}
```

## Alias With Shader Type

```cpp
#include <LumaWave/Factory.h>

using ShadedBusType = Bus<Ws2812, PlatformDefault, GammaShader>;
```

This gives you an explicit bus type that includes a concrete shader type in the protocol stack.

## Mixed 8/16-bit Strips With Rgbw16 Interface

```cpp
#include <LumaWave/Factory.h>

using WsRgbw16OnWire16 = descriptors::Ws2812x<
    Rgbw16Color,
    ChannelOrder::GRBW,
    &timing::Ws2812x,
    Rgbw16Color>;

using WsRgbw16OnWire8 = descriptors::Ws2812x<
    Rgbw16Color,
    ChannelOrder::GRBW,
    &timing::Ws2812x,
    Rgbw8Color>;

auto makeMixedDepthBus()
{
    auto highDepth = makeBus<WsRgbw16OnWire16, PlatformDefault>(
        120,
        OneWireTiming::Ws2812x,
        PlatformDefaultOptions{});

    auto lowDepth = makeBus<WsRgbw16OnWire8, PlatformDefault>(
        80,
        OneWireTiming::Ws2812x,
        PlatformDefaultOptions{});

    return makeBus(std::move(highDepth), std::move(lowDepth));
}
```

This keeps the root/composite bus color contract at `Rgbw16Color` while allowing each strand to serialize at its own wire depth.

## TM1829 Convenience Descriptor

```cpp
#include <LumaWave/Factory.h>

using BusType = Bus<Tm1829, PlatformDefault>;

BusType makeTm1829(uint16_t pixelCount)
{
    PlatformDefaultOptions tx{};
    tx.dataPin = 2;

    return makeBus<Tm1829, PlatformDefault>(
        pixelCount,
        Ws2812xOptions{},
        tx);
}
```

`Tm1829` is a `Ws2812x`-family convenience descriptor with TM1829 timing metadata and idle-high/invert behavior baked into the descriptor/trait path.

## Notes

- `Bus<ProtocolDesc, TransportDesc>` deduces the same concrete type returned by `makeBus<ProtocolDesc, TransportDesc>(...)`.
- `Bus<ProtocolDesc, TransportDesc, TShaderTemplate>` binds shader color from the protocol descriptor automatically.
- For one-wire protocol descriptors with plain transport descriptors, `Bus<...>` resolves to the wrapped one-wire transport bus type automatically.
- `PlatformDefault` and `PlatformDefaultOptions` follow the current platform mapping in the factory traits.
