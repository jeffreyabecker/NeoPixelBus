# Export Symbol Categorization Checklist

Source: `docs/internal/information/lumawave-public-surface-dump.md`
Generated: 2026-03-05

Instructions: Mark each symbol with your target export category (for example `Default`, `Optional`, `Never`).


## Core (`lw`)

- [ ] `PixelCount` | Category: A
- [ ] `ssize_t` | Category: c
- [ ] `remove_cvref_t` | Category: c
- [ ] `dynamic_extent` | Category: c
- [ ] `span` | Category: 
- [ ] `IndexSentinel` | Category: c
- [ ] `IndexIterator` | Category: c
- [ ] `IndexRange` | Category: c
- [ ] `IPixelBus<TColor>` | Category: A
- [ ] `PixelView<TColor>` | Category: A
- [ ] `GridMapping` | Category: A
- [ ] `TopologySettings` | Category: A
- [ ] `Topology` | Category: A

## Color Models and Traits (`lw`)

- [ ] `RgbBasedColor<NChannels, TComponent, InternalSize>` | Category: 
- [ ] `Rgb8Color` | Category: 
- [ ] `Rgbw8Color` | Category: 
- [ ] `Rgbcw8Color` | Category: 
- [ ] `Rgb16Color` | Category: 
- [ ] `Rgbw16Color` | Category: 
- [ ] `Rgbcw16Color` | Category: 
- [ ] `DefaultColorType` | Category: 
- [ ] `Color` | Category: A
- [ ] `LargerColorType<TLeftColor, TRightColor>` | Category: 
- [ ] `LargerColorTypeT<TLeftColor, TRightColor>` | Category: 
- [ ] `ColorType<TColor>` | Category: 
- [ ] `ColorChannelsExactly<TColor, NChannels>` | Category: 
- [ ] `ColorChannelsAtLeast<TColor, MinChannels>` | Category: 
- [ ] `ColorChannelsAtMost<TColor, MaxChannels>` | Category: 
- [ ] `ColorChannelsInRange<TColor, MinChannels, MaxChannels>` | Category: 
- [ ] `ColorComponentTypeIs<TColor, TComponent>` | Category: 
- [ ] `ColorComponentBitDepth<TColor, BitDepth>` | Category: 
- [ ] `ColorComponentAtLeastAsLarge<TLeftColor, TRightColor>` | Category: 
- [ ] `ColorChannelAtLeastAsLarge<TLeftColor, TRightColor>` | Category: 
- [ ] `ColorAtLeastAsLarge<TLeftColor, TRightColor>` | Category: 
- [ ] `RequireColorChannelsExactly<TColor, NChannels>` | Category: 
- [ ] `RequireColorChannelsInRange<TColor, MinChannels, MaxChannels>` | Category: 
- [ ] `RequireColorComponentBitDepth<TColor, BitDepth>` | Category: 

## Channel and Color Utilities (`lw`)

- [ ] `ChannelMap<TColor, TValue>` | Category: 
- [ ] `ChannelSource<TColor, ...>` | Category: 
- [ ] `ColorChannelIndexIterator<NChannels>` | Category: 
- [ ] `ColorChannelIndexRange<NChannels>` | Category: 
- [ ] `ColorHexCodec` | Category: 
- [ ] `ColorIteratorT<TColor>` | Category: 
- [ ] `SolidColorSourceT<TColor>` | Category: 
- [ ] `SpanColorSourceT<TColor>` | Category: 
- [ ] `FillColorSourceT<TColor>` | Category: 
- [ ] `HsbColor` | Category: 
- [ ] `HslColor` | Category: 
- [ ] `HueBlendBase` | Category: 
- [ ] `HueBlendShortestDistance` | Category: 
- [ ] `HueBlendLongestDistance` | Category: 
- [ ] `HueBlendClockwiseDirection` | Category: 
- [ ] `HueBlendCounterClockwiseDirection` | Category: 
- `ChannelOrder` namespace tags:
- [ ] `RGB` | Category: 
- [ ] `GRB` | Category: 
- [ ] `BGR` | Category: 
- [ ] `RGBW` | Category: 
- [ ] `GRBW` | Category: 
- [ ] `BGRW` | Category: 
- [ ] `WRGB` | Category: 
- [ ] `W` | Category: 
- [ ] `CW` | Category: 
- [ ] `RGBCW` | Category: 
- [ ] `GRBCW` | Category: 
- [ ] `BGRCW` | Category: 

## Shaders (`lw`)

- [ ] `IShader<TColor>` | Category: 
- [ ] `NilShader<TColor>` | Category: 
- [ ] `AggregateShaderSettings<TColor>` | Category: 
- [ ] `AggregateShader<TColor>` | Category: 
- [ ] `OwningAggregateShaderT<TColor>` | Category: 
- [ ] `GammaShaderSettings<TColor>` | Category: 
- [ ] `GammaShader<TColor>` | Category: 
- [ ] `WledGammaShader<TColor>` | Category: 
- [ ] `CurrentLimiterShaderSettings<TColor>` | Category: 
- [ ] `CurrentLimiterShader<TColor>` | Category: 
- [ ] `AutoWhiteBalanceShaderSettings<TColor>` | Category: 
- [ ] `AutoWhiteBalanceShader<TColor, TKelvinToRgbStrategy>` | Category: 
- [ ] `CCTColorInterlock` | Category: 
- [ ] `CCTWhiteBalanceShaderSettings<TColor, TKelvinToRgbStrategy>` | Category: 
- [ ] `CCTWhiteBalanceShader<TColor, TKelvinToRgbStrategy>` | Category: 
- [ ] `KelvinToRgbExactStrategy<TComponent>` | Category: 
- [ ] `KelvinToRgbLut64Strategy<TComponent>` | Category: 

## Palette API (`lw`)

- [ ] `PaletteSampleOptions` | Category: 
- [ ] `PaletteStop<TColor>` | Category: 
- [ ] `Palette<TColor>` | Category: 
- [ ] `SolidPaletteGenerator<TColor>` | Category: 
- [ ] `RainbowPaletteGenerator<TColor>` | Category: 
- [ ] `RandomSmoothPaletteGenerator<TColor>` | Category: 
- [ ] `RandomCyclePaletteGenerator<TColor>` | Category: 
- Blend operation structs:
- [ ] `BlendOpLinear` | Category: 
- [ ] `BlendOpStep` | Category: 
- [ ] `BlendOpHoldMidpoint` | Category: 
- [ ] `BlendOpSmoothstep` | Category: 
- [ ] `BlendOpCubic` | Category: 
- [ ] `BlendOpCosine` | Category: 
- [ ] `BlendOpGammaLinear` | Category: 
- [ ] `BlendOpQuantized<TLevels>` | Category: 
- [ ] `BlendOpDitheredLinear` | Category: 
- Blend wrappers:
- [ ] `BlendLinearContiguous` | Category: 
- [ ] `BlendNearestContiguous<TTieBreak>` | Category: 
- [ ] `InterpolatedBlendContiguous<TOperation>` | Category: 
- [ ] `BlendStepContiguous` | Category: 
- [ ] `BlendHoldMidpointContiguous` | Category: 
- [ ] `BlendSmoothstepContiguous` | Category: 
- [ ] `BlendCubicContiguous` | Category: 
- [ ] `BlendCosineContiguous` | Category: 
- [ ] `BlendGammaLinearContiguous` | Category: 
- [ ] `BlendQuantizedContiguous<TLevels>` | Category: 
- [ ] `BlendDitheredLinearContiguous` | Category: 
- Nearest policies:
- [ ] `NearestTieStable` | Category: 
- [ ] `NearestTieLeft` | Category: 
- [ ] `NearestTieRight` | Category: 
- Wrap policies:
- [ ] `WrapClamp` | Category: 
- [ ] `WrapCircular` | Category: 
- [ ] `WrapMirror` | Category: 
- [ ] `WrapHoldFirst` | Category: 
- [ ] `WrapHoldLast` | Category: 
- [ ] `WrapBlackout` | Category: 
- [ ] `WrapWindow` | Category: 
- [ ] `WrapModuloSpan` | Category: 
- [ ] `WrapOffsetCircular` | Category: 

## Light Drivers (`lw`)

- [ ] `LightDriverSettingsBase` | Category: 
- [ ] `ILightDriver<TColor>` | Category: 
- [ ] `NilLightDriverSettings` | Category: 
- [ ] `NilLightDriver<TColor>` | Category: 
- [ ] `PrintLightDriverSettingsT<TWritable>` | Category: 
- [ ] `PrintLightDriverT<TColor, TWritable>` | Category: 
- [ ] `PrintLightDriverSettings` (Arduino) | Category: 
- [ ] `PrintLightDriver<TColor>` (Arduino) | Category: 
- [ ] `AnalogPwmLightDriverSettings` (platform-gated) | Category: 
- [ ] `AnalogPwmLightDriver<TColor>` (platform-gated) | Category: 
- [ ] `Esp32LedcLightDriverSettings` (platform-gated) | Category: 
- [ ] `Esp32LedcLightDriver<TColor>` (platform-gated) | Category: 
- [ ] `Esp32SigmaDeltaLightDriverSettings` (platform-gated) | Category: 
- [ ] `Esp32SigmaDeltaLightDriver<TColor>` (platform-gated) | Category: 
- [ ] `RpPwmLightDriverSettings` (platform-gated) | Category: 
- [ ] `RpPwmLightDriver<TColor>` (platform-gated) | Category: 

## Transport API (`lw`)

- [ ] `TransportSettingsBase` | Category: 
- [ ] `ITransport` | Category: 
- [ ] `TransportSettingsWithInvert<TTransportSettings>` | Category: 
- [ ] `TransportLike<TTransport>` | Category: 
- [ ] `NilTransportSettings` | Category: 
- [ ] `NilTransport` | Category: 
- [ ] `SpiTransportSettings` | Category: 
- [ ] `SpiTransport` | Category: 
- [ ] `PrintTransportSettingsT<TWritable>` | Category: 
- [ ] `PrintTransportT<TWritable>` | Category: 
- [ ] `PrintTransportSettings` | Category: 
- [ ] `PrintTransport` | Category: 
- [ ] `OneWireTiming` | Category: 
- [ ] `OneWireEncoding<TTransportSettings>` | Category: 
- [ ] `EncodedClockDataBitPattern` | Category: 
- [ ] `timing` namespace constants (from `OneWireTiming.h`) | Category: 
- RP2040 transports (platform-gated):
- [ ] `RpPioTransportSettings` | Category: 
- [ ] `RpPioTransport` | Category: 
- [ ] `RpSpiTransportSettings` | Category: 
- [ ] `RpSpiTransport` | Category: 
- [ ] `RpUartTransportSettings` | Category: 
- [ ] `RpUartTransport` | Category: 
- ESP32 transports (platform-gated):
- [ ] `Esp32DmaSpiTransportSettings` | Category: 
- [ ] `Esp32DmaSpiTransport` | Category: 
- [ ] `Esp32I2sTransportSettings` | Category: 
- [ ] `Esp32I2sTransport` | Category: 
- [ ] `Esp32RmtTransportSettings` | Category: 
- [ ] `Esp32RmtTransport` | Category: 
- ESP8266 transports (platform-gated):
- [ ] `Esp8266DmaI2sTransportSettings` | Category: 
- [ ] `Esp8266DmaI2sTransport` | Category: 
- [ ] `Esp8266DmaUartTransportSettings` | Category: 
- [ ] `Esp8266DmaUartTransport` | Category: 

## Protocol API (`lw`)

- [ ] `ProtocolSettings` | Category: 
- [ ] `IProtocol<TColor>` | Category: 
- [ ] `ProtocolType<TProtocol>` | Category: 
- [ ] `ProtocolExternalBufferRequired<TProtocol>` | Category: 
- [ ] `ProtocolRequiredBufferSizeComputable<TProtocol>` | Category: 
- [ ] `ProtocolDecoratorBase<TDerived, TWrappedProtocol, TColor>` | Category: 
- [ ] `DebugProtocolSettingsT<TWrappedProtocol, TWritable>` | Category: 
- [ ] `DebugProtocol<TWrappedProtocol, TWritable>` | Category: 
- [ ] `Apa102ProtocolSettings` | Category: 
- [ ] `Apa102Protocol<TInterfaceColor, TStripColor>` | Category: 
- [ ] `Hd108ProtocolSettings` | Category: 
- [ ] `Hd108Protocol<TInterfaceColor, TStripColor>` | Category: 
- [ ] `Lpd6803ProtocolSettings` | Category: 
- [ ] `Lpd6803ProtocolT<TInterfaceColor>` | Category: 
- [ ] `Lpd6803Protocol` | Category: 
- [ ] `Lpd8806ProtocolSettings` | Category: 
- [ ] `Lpd8806ProtocolT<TInterfaceColor>` | Category: 
- [ ] `Lpd8806Protocol` | Category: 
- [ ] `NilProtocolSettings` | Category: 
- [ ] `NilProtocol<TColor>` | Category: 
- [ ] `P9813ProtocolSettings` | Category: 
- [ ] `P9813ProtocolT<TInterfaceColor>` | Category: 
- [ ] `P9813Protocol` | Category: 
- [ ] `PixieProtocolSettings` | Category: 
- [ ] `PixieProtocolT<TInterfaceColor>` | Category: 
- [ ] `PixieProtocol` | Category: 
- [ ] `Sm16716ProtocolSettings` | Category: 
- [ ] `Sm16716ProtocolT<TInterfaceColor>` | Category: 
- [ ] `Sm16716Protocol` | Category: 
- [ ] `Sm168xProtocolSettings` | Category: 
- [ ] `Sm168xProtocol<TInterfaceColor>` | Category: 
- [ ] `Tlc59711Settings` | Category: 
- [ ] `Tlc59711ProtocolSettings` | Category: 
- [ ] `Tlc59711ProtocolT<TInterfaceColor>` | Category: 
- [ ] `Tlc59711Protocol` | Category: 
- [ ] `Tm1814CurrentSettings` | Category: 
- [ ] `Tm1814ProtocolSettings` | Category: 
- [ ] `Tm1814ProtocolT<TInterfaceColor>` | Category: 
- [ ] `Tm1814Protocol` | Category: 
- [ ] `Tm1914Mode` | Category: 
- [ ] `Tm1914ProtocolSettings` | Category: 
- [ ] `Tm1914ProtocolT<TInterfaceColor>` | Category: 
- [ ] `Tm1914Protocol` | Category: 
- [ ] `Ws2801ProtocolSettings` | Category: 
- [ ] `Ws2801ProtocolT<TInterfaceColor>` | Category: 
- [ ] `Ws2801Protocol` | Category: 
- [ ] `Ws2812xProtocolSettings` | Category: 
- [ ] `Ws2812xProtocol<TInterfaceColor, TStripColor>` | Category: 

## Protocol Alias Layer (`lw::protocols`)

- [ ] `ResolveProtocolType<TProtocolCandidate>` | Category: 
- [ ] `DotStar<...>` | Category: 
- [ ] `DotStarType<...>` | Category: 
- [ ] `APA102` | Category: 
- [ ] `APA102Type` | Category: 
- [ ] `Hd108<...>` | Category: 
- [ ] `Hd108Type<...>` | Category: 
- [ ] `HD108` | Category: 
- [ ] `HD108Type` | Category: 
- [ ] `None<TColor>` | Category: 
- [ ] `NoneType<TColor>` | Category: 
- [ ] `Debug<TWrappedProtocolSpec>` | Category: 
- [ ] `DebugType<TWrappedProtocolSpec>` | Category: 
- [ ] `Tm1814<TInterfaceColor>` | Category: 
- [ ] `Tm1814Type<TInterfaceColor>` | Category: 
- [ ] `Tm1914<TInterfaceColor>` | Category: 
- [ ] `Tm1914Type<TInterfaceColor>` | Category: 
- [ ] `Ws2812x<...>` | Category: 
- [ ] `Ws2812xType<...>` | Category: 
- [ ] `Ws2812<TInterfaceColor>` | Category: 
- [ ] `Ws2812Type<TInterfaceColor>` | Category: 
- [ ] `Ws2811<TInterfaceColor>` | Category: 
- [ ] `Ws2811Type<TInterfaceColor>` | Category: 
- [ ] `Ws2805<TInterfaceColor>` | Category: 
- [ ] `Ws2805Type<TInterfaceColor>` | Category: 
- [ ] `Sk6812<TInterfaceColor>` | Category: 
- [ ] `Sk6812Type<TInterfaceColor>` | Category: 
- [ ] `Tm1829<TInterfaceColor>` | Category: 
- [ ] `Tm1829Type<TInterfaceColor>` | Category: 

## Bus API (`lw`)

- [ ] `PlatformDefaultStaticBusDriverTransport` | Category: 
- [ ] `PlatformDefaultStaticBusDriverTransportSettings` | Category: 
- [ ] `PixelBus<TProtocol, TTransport, TShader>` | Category: 
- [ ] `AggregateBus<TColor>` | Category: 
- [ ] `LightBus<TColor, TDriver, TShader>` | Category: 
- [ ] `ReferenceBus<TColor, TProtocol, TTransport, TShader>` | Category: 
