# Factory Bus Alias Example

This page shows how to use the `Bus<...>` helper alias so you can name the concrete bus type without writing the full return type.

## Basic Alias

```cpp
#include <LumaWave.h>

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
#include <LumaWave.h>

using ShadedBusType = Bus<Ws2812, PlatformDefault, GammaShader>;
```

This gives you an explicit bus type that includes a concrete shader type in the protocol stack.

## Notes

- `Bus<ProtocolDesc, TransportDesc>` deduces the same concrete type returned by `makeBus<ProtocolDesc, TransportDesc>(...)`.
- `Bus<ProtocolDesc, TransportDesc, TShaderTemplate>` binds shader color from the protocol descriptor automatically.
- For one-wire protocol descriptors with plain transport descriptors, `Bus<...>` resolves to the wrapped one-wire transport bus type automatically.
- `PlatformDefault` and `PlatformDefaultOptions` follow the current platform mapping in the factory traits.
