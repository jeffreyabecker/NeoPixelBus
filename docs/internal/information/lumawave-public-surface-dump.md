# LumaWave Public Surface Dump

This is a quick inventory of symbols a consumer gets from:

```cpp
#include <LumaWave.h>
```

Scope notes:
- Symbols are exposed in `lw` (and nested namespaces like `lw::protocols`), not the C++ global namespace.
- Platform-specific symbols are conditionally available based on target defines.

## Include Chain
- `LumaWave.h`
- `LumaWave/All.h`
- `LumaWave/Core.h`
- `LumaWave/Colors.h`
- `LumaWave/Lights.h`
- `LumaWave/Transports.h`
- `LumaWave/Protocols.h`
- `LumaWave/Buses.h`

## Core (`lw`)
- `PixelCount`
- `ssize_t`
- `remove_cvref_t`
- `dynamic_extent`
- `span`
- `IndexSentinel`
- `IndexIterator`
- `IndexRange`
- `IPixelBus<TColor>`
- `PixelView<TColor>`
- `GridMapping`
- `TopologySettings`
- `Topology`

## Color Models and Traits (`lw`)
- `RgbBasedColor<NChannels, TComponent, InternalSize>`
- `Rgb8Color`
- `Rgbw8Color`
- `Rgbcw8Color`
- `Rgb16Color`
- `Rgbw16Color`
- `Rgbcw16Color`
- `DefaultColorType`
- `Color`
- `LargerColorType<TLeftColor, TRightColor>`
- `LargerColorTypeT<TLeftColor, TRightColor>`
- `ColorType<TColor>`
- `ColorChannelsExactly<TColor, NChannels>`
- `ColorChannelsAtLeast<TColor, MinChannels>`
- `ColorChannelsAtMost<TColor, MaxChannels>`
- `ColorChannelsInRange<TColor, MinChannels, MaxChannels>`
- `ColorComponentTypeIs<TColor, TComponent>`
- `ColorComponentBitDepth<TColor, BitDepth>`
- `ColorComponentAtLeastAsLarge<TLeftColor, TRightColor>`
- `ColorChannelAtLeastAsLarge<TLeftColor, TRightColor>`
- `ColorAtLeastAsLarge<TLeftColor, TRightColor>`
- `RequireColorChannelsExactly<TColor, NChannels>`
- `RequireColorChannelsInRange<TColor, MinChannels, MaxChannels>`
- `RequireColorComponentBitDepth<TColor, BitDepth>`

## Channel and Color Utilities (`lw`)
- `ChannelMap<TColor, TValue>`
- `ChannelSource<TColor, ...>`
- `ColorChannelIndexIterator<NChannels>`
- `ColorChannelIndexRange<NChannels>`
- `ColorHexCodec`
- `ColorIteratorT<TColor>`
- `SolidColorSourceT<TColor>`
- `SpanColorSourceT<TColor>`
- `FillColorSourceT<TColor>`
- `HsbColor`
- `HslColor`
- `HueBlendBase`
- `HueBlendShortestDistance`
- `HueBlendLongestDistance`
- `HueBlendClockwiseDirection`
- `HueBlendCounterClockwiseDirection`
- `ChannelOrder` namespace tags:
- `RGB`
- `GRB`
- `BGR`
- `RGBW`
- `GRBW`
- `BGRW`
- `WRGB`
- `W`
- `CW`
- `RGBCW`
- `GRBCW`
- `BGRCW`

## Shaders (`lw`)
- `IShader<TColor>`
- `NilShader<TColor>`
- `AggregateShaderSettings<TColor>`
- `AggregateShader<TColor>`
- `OwningAggregateShaderT<TColor>`
- `GammaShaderSettings<TColor>`
- `GammaShader<TColor>`
- `WledGammaShader<TColor>`
- `CurrentLimiterShaderSettings<TColor>`
- `CurrentLimiterShader<TColor>`
- `AutoWhiteBalanceShaderSettings<TColor>`
- `AutoWhiteBalanceShader<TColor, TKelvinToRgbStrategy>`
- `CCTColorInterlock`
- `CCTWhiteBalanceShaderSettings<TColor, TKelvinToRgbStrategy>`
- `CCTWhiteBalanceShader<TColor, TKelvinToRgbStrategy>`
- `KelvinToRgbExactStrategy<TComponent>`
- `KelvinToRgbLut64Strategy<TComponent>`

## Palette API (`lw`)
- `PaletteSampleOptions`
- `PaletteStop<TColor>`
- `Palette<TColor>`
- `SolidPaletteGenerator<TColor>`
- `RainbowPaletteGenerator<TColor>`
- `RandomSmoothPaletteGenerator<TColor>`
- `RandomCyclePaletteGenerator<TColor>`
- Blend operation structs:
- `BlendOpLinear`
- `BlendOpStep`
- `BlendOpHoldMidpoint`
- `BlendOpSmoothstep`
- `BlendOpCubic`
- `BlendOpCosine`
- `BlendOpGammaLinear`
- `BlendOpQuantized<TLevels>`
- `BlendOpDitheredLinear`
- Blend wrappers:
- `BlendLinearContiguous`
- `BlendNearestContiguous<TTieBreak>`
- `InterpolatedBlendContiguous<TOperation>`
- `BlendStepContiguous`
- `BlendHoldMidpointContiguous`
- `BlendSmoothstepContiguous`
- `BlendCubicContiguous`
- `BlendCosineContiguous`
- `BlendGammaLinearContiguous`
- `BlendQuantizedContiguous<TLevels>`
- `BlendDitheredLinearContiguous`
- Nearest policies:
- `NearestTieStable`
- `NearestTieLeft`
- `NearestTieRight`
- Wrap policies:
- `WrapClamp`
- `WrapCircular`
- `WrapMirror`
- `WrapHoldFirst`
- `WrapHoldLast`
- `WrapBlackout`
- `WrapWindow`
- `WrapModuloSpan`
- `WrapOffsetCircular`

## Light Drivers (`lw`)
- `LightDriverSettingsBase`
- `ILightDriver<TColor>`
- `NilLightDriverSettings`
- `NilLightDriver<TColor>`
- `PrintLightDriverSettingsT<TWritable>`
- `PrintLightDriverT<TColor, TWritable>`
- `PrintLightDriverSettings` (Arduino)
- `PrintLightDriver<TColor>` (Arduino)
- `AnalogPwmLightDriverSettings` (platform-gated)
- `AnalogPwmLightDriver<TColor>` (platform-gated)
- `Esp32LedcLightDriverSettings` (platform-gated)
- `Esp32LedcLightDriver<TColor>` (platform-gated)
- `Esp32SigmaDeltaLightDriverSettings` (platform-gated)
- `Esp32SigmaDeltaLightDriver<TColor>` (platform-gated)
- `RpPwmLightDriverSettings` (platform-gated)
- `RpPwmLightDriver<TColor>` (platform-gated)

## Transport API (`lw`)
- `TransportSettingsBase`
- `ITransport`
- `TransportSettingsWithInvert<TTransportSettings>`
- `TransportLike<TTransport>`
- `NilTransportSettings`
- `NilTransport`
- `SpiTransportSettings`
- `SpiTransport`
- `PrintTransportSettingsT<TWritable>`
- `PrintTransportT<TWritable>`
- `PrintTransportSettings`
- `PrintTransport`
- `OneWireTiming`
- `OneWireEncoding<TTransportSettings>`
- `EncodedClockDataBitPattern`
- `timing` namespace constants (from `OneWireTiming.h`)
- RP2040 transports (platform-gated):
- `RpPioTransportSettings`
- `RpPioTransport`
- `RpSpiTransportSettings`
- `RpSpiTransport`
- `RpUartTransportSettings`
- `RpUartTransport`
- ESP32 transports (platform-gated):
- `Esp32DmaSpiTransportSettings`
- `Esp32DmaSpiTransport`
- `Esp32I2sTransportSettings`
- `Esp32I2sTransport`
- `Esp32RmtTransportSettings`
- `Esp32RmtTransport`
- ESP8266 transports (platform-gated):
- `Esp8266DmaI2sTransportSettings`
- `Esp8266DmaI2sTransport`
- `Esp8266DmaUartTransportSettings`
- `Esp8266DmaUartTransport`

## Protocol API (`lw`)
- `ProtocolSettings`
- `IProtocol<TColor>`
- `ProtocolType<TProtocol>`
- `ProtocolExternalBufferRequired<TProtocol>`
- `ProtocolRequiredBufferSizeComputable<TProtocol>`
- `ProtocolDecoratorBase<TDerived, TWrappedProtocol, TColor>`
- `DebugProtocolSettingsT<TWrappedProtocol, TWritable>`
- `DebugProtocol<TWrappedProtocol, TWritable>`
- `Apa102ProtocolSettings`
- `Apa102Protocol<TInterfaceColor, TStripColor>`
- `Hd108ProtocolSettings`
- `Hd108Protocol<TInterfaceColor, TStripColor>`
- `Lpd6803ProtocolSettings`
- `Lpd6803ProtocolT<TInterfaceColor>`
- `Lpd6803Protocol`
- `Lpd8806ProtocolSettings`
- `Lpd8806ProtocolT<TInterfaceColor>`
- `Lpd8806Protocol`
- `NilProtocolSettings`
- `NilProtocol<TColor>`
- `P9813ProtocolSettings`
- `P9813ProtocolT<TInterfaceColor>`
- `P9813Protocol`
- `PixieProtocolSettings`
- `PixieProtocolT<TInterfaceColor>`
- `PixieProtocol`
- `Sm16716ProtocolSettings`
- `Sm16716ProtocolT<TInterfaceColor>`
- `Sm16716Protocol`
- `Sm168xProtocolSettings`
- `Sm168xProtocol<TInterfaceColor>`
- `Tlc59711Settings`
- `Tlc59711ProtocolSettings`
- `Tlc59711ProtocolT<TInterfaceColor>`
- `Tlc59711Protocol`
- `Tm1814CurrentSettings`
- `Tm1814ProtocolSettings`
- `Tm1814ProtocolT<TInterfaceColor>`
- `Tm1814Protocol`
- `Tm1914Mode`
- `Tm1914ProtocolSettings`
- `Tm1914ProtocolT<TInterfaceColor>`
- `Tm1914Protocol`
- `Ws2801ProtocolSettings`
- `Ws2801ProtocolT<TInterfaceColor>`
- `Ws2801Protocol`
- `Ws2812xProtocolSettings`
- `Ws2812xProtocol<TInterfaceColor, TStripColor>`

## Protocol Alias Layer (`lw::protocols`)
- `ResolveProtocolType<TProtocolCandidate>`
- `DotStar<...>`
- `DotStarType<...>`
- `APA102`
- `APA102Type`
- `Hd108<...>`
- `Hd108Type<...>`
- `HD108`
- `HD108Type`
- `None<TColor>`
- `NoneType<TColor>`
- `Debug<TWrappedProtocolSpec>`
- `DebugType<TWrappedProtocolSpec>`
- `Tm1814<TInterfaceColor>`
- `Tm1814Type<TInterfaceColor>`
- `Tm1914<TInterfaceColor>`
- `Tm1914Type<TInterfaceColor>`
- `Ws2812x<...>`
- `Ws2812xType<...>`
- `Ws2812<TInterfaceColor>`
- `Ws2812Type<TInterfaceColor>`
- `Ws2811<TInterfaceColor>`
- `Ws2811Type<TInterfaceColor>`
- `Ws2805<TInterfaceColor>`
- `Ws2805Type<TInterfaceColor>`
- `Sk6812<TInterfaceColor>`
- `Sk6812Type<TInterfaceColor>`
- `Tm1829<TInterfaceColor>`
- `Tm1829Type<TInterfaceColor>`

## Bus API (`lw`)
- `PlatformDefaultStaticBusDriverTransport`
- `PlatformDefaultStaticBusDriverTransportSettings`
- `PixelBus<TProtocol, TTransport, TShader>`
- `AggregateBus<TColor>`
- `LightBus<TColor, TDriver, TShader>`
- `ReferenceBus<TColor, TProtocol, TTransport, TShader>`

---

Generated manually from the `LumaWave.h -> LumaWave/All.h` include chain and symbol scans in `src/**`.
This is a broad consumer-facing dump, not a strict ABI manifest.
