# Export Symbol Categorization Checklist

Source: `docs/internal/information/lumawave-public-surface-dump.md`
Generated: 2026-03-05

Instructions: Mark each symbol with your target export category (for example `Default`, `Optional`, `Never`).


## Core (`lw`)

- [ ] `lw::PixelCount` | New Alias: pixel_count_t
- [ ] `lw::ssize_t` | New Alias: 
- [ ] `lw::remove_cvref_t` | New Alias: 
- [ ] `lw::dynamic_extent` | New Alias: 
- [ ] `lw::span` | New Alias: 
- [ ] `lw::IndexSentinel` | New Alias: 
- [ ] `lw::IndexIterator` | New Alias: 
- [ ] `lw::IndexRange` | New Alias: 
- [ ] `lw::IPixelBus<TColor>` | New Alias: IPixelBus<TColor>
- [ ] `lw::PixelView<TColor>` | New Alias: PixelView<TColor>
- [ ] `lw::GridMapping` | New Alias: GridMapping
- [ ] `lw::TopologySettings` | New Alias: TopologySettings
- [ ] `lw::Topology` | New Alias: Topology

## Color Models and Traits (`lw`)

- [ ] `lw::colors::RgbBasedColor<NChannels, TComponent, InternalSize>` | New Alias: 
- [ ] `lw::colors::Rgb8Color` | New Alias: Rgb8Color
- [ ] `lw::colors::Rgbw8Color` | New Alias: Rgbw8Color
- [ ] `lw::colors::Rgbcw8Color` | New Alias: Rgbcw8Color
- [ ] `lw::colors::Rgb16Color` | New Alias: Rgb16Color
- [ ] `lw::colors::Rgbw16Color` | New Alias: Rgbw16Color
- [ ] `lw::colors::Rgbcw16Color` | New Alias: Rgbcw16Color
- [ ] `lw::colors::DefaultColorType` | New Alias: 
- [ ] `lw::colors::Color` | New Alias: Color
- [ ] `lw::colors::LargerColorType<TLeftColor, TRightColor>` | New Alias: 
- [ ] `lw::colors::LargerColorTypeT<TLeftColor, TRightColor>` | New Alias: 
- [ ] `lw::colors::ColorType<TColor>` | New Alias: 
- [ ] `lw::colors::ColorChannelsExactly<TColor, NChannels>` | New Alias: 
- [ ] `lw::colors::ColorChannelsAtLeast<TColor, MinChannels>` | New Alias: 
- [ ] `lw::colors::ColorChannelsAtMost<TColor, MaxChannels>` | New Alias: 
- [ ] `lw::colors::ColorChannelsInRange<TColor, MinChannels, MaxChannels>` | New Alias: 
- [ ] `lw::colors::ColorComponentTypeIs<TColor, TComponent>` | New Alias: 
- [ ] `lw::colors::ColorComponentBitDepth<TColor, BitDepth>` | New Alias: 
- [ ] `lw::colors::ColorComponentAtLeastAsLarge<TLeftColor, TRightColor>` | New Alias: 
- [ ] `lw::colors::ColorChannelAtLeastAsLarge<TLeftColor, TRightColor>` | New Alias: 
- [ ] `lw::colors::ColorAtLeastAsLarge<TLeftColor, TRightColor>` | New Alias: 
- [ ] `lw::colors::RequireColorChannelsExactly<TColor, NChannels>` | New Alias: 
- [ ] `lw::colors::RequireColorChannelsInRange<TColor, MinChannels, MaxChannels>` | New Alias: 
- [ ] `lw::colors::RequireColorComponentBitDepth<TColor, BitDepth>` | New Alias: 

## Channel and Color Utilities (`lw`)

- [ ] `lw::colors::ChannelMap<TColor, TValue>` | New Alias: 
- [ ] `lw::colors::ChannelSource<TColor, ...>` | New Alias: 
- [ ] `lw::colors::ColorChannelIndexIterator<NChannels>` | New Alias: 
- [ ] `lw::colors::ColorChannelIndexRange<NChannels>` | New Alias: 
- [ ] `lw::colors::ColorHexCodec` | New Alias: 
- [ ] `lw::colors::ColorIteratorT<TColor>` | New Alias: 
- [ ] `lw::colors::SolidColorSourceT<TColor>` | New Alias: 
- [ ] `lw::colors::SpanColorSourceT<TColor>` | New Alias: 
- [ ] `lw::colors::FillColorSourceT<TColor>` | New Alias: 
- [ ] `lw::colors::HsbColor` | New Alias: HsbColor
- [ ] `lw::colors::HslColor` | New Alias: HslColor
- [ ] `lw::colors::HueBlendBase` | New Alias: 
- [ ] `lw::colors::HueBlendShortestDistance` | New Alias: HueBlend::Shortest
- [ ] `lw::colors::HueBlendLongestDistance` | New Alias: HuBlend::Longest
- [ ] `lw::colors::HueBlendClockwiseDirection` | New Alias: HueBlend::Clockwise
- [ ] `lw::colors::HueBlendCounterClockwiseDirection` | New Alias: HueBlend::CounterClockwise
- `ChannelOrder` namespace tags:
- [ ] `lw::colors::ChannelOrder::RGB` | New Alias: ChannelOrder::RGB
- [ ] `lw::colors::ChannelOrder::GRB` | New Alias: ChannelOrder:GRB
- [ ] `lw::colors::ChannelOrder::BGR` | New Alias: ChannelOrder::BGR
- [ ] `lw::colors::ChannelOrder::RGBW` | New Alias: ChannelOrder::RGBW
- [ ] `lw::colors::ChannelOrder::GRBW` | New Alias: ChannelOrder::GRBw
- [ ] `lw::colors::ChannelOrder::BGRW` | New Alias: ChannelOrder::BGRW
- [ ] `lw::colors::ChannelOrder::WRGB` | New Alias: ChannelOrder::WRGB
- [ ] `lw::colors::ChannelOrder::W` | New Alias: 
- [ ] `lw::colors::ChannelOrder::CW` | New Alias: 
- [ ] `lw::colors::ChannelOrder::RGBCW` | New Alias: ChannelOrder::RGBCW
- [ ] `lw::colors::ChannelOrder::GRBCW` | New Alias: ChannelOrder::GRBCW
- [ ] `lw::colors::ChannelOrder::BGRCW` | New Alias: ChannelOrder::BGRCW

## Shaders (`lw`)

- [ ] `lw::shaders::IShader<TColor>` | New Alias: IShader<TColor>
- [ ] `lw::shaders::NilShader<TColor>` | New Alias: NoShader<TColor>
- [ ] `lw::shaders::AggregateShaderSettings<TColor>` | New Alias: 
- [ ] `lw::shaders::AggregateShader<TColor>` | New Alias: 
- [ ] `lw::shaders::OwningAggregateShaderT<TColor>` | New Alias: 
- [ ] `lw::shaders::GammaShaderSettings<TColor>` | New Alias: 
- [ ] `lw::shaders::GammaShader<TColor>` | New Alias: 
- [ ] `lw::shaders::WledGammaShader<TColor>` | New Alias: 
- [ ] `lw::shaders::CurrentLimiterShaderSettings<TColor>` | New Alias: 
- [ ] `lw::shaders::CurrentLimiterShader<TColor>` | New Alias: 
- [ ] `lw::shaders::AutoWhiteBalanceShaderSettings<TColor>` | New Alias: 
- [ ] `lw::shaders::AutoWhiteBalanceShader<TColor, TKelvinToRgbStrategy>` | New Alias: 
- [ ] `lw::shaders::CCTColorInterlock` | New Alias: 
- [ ] `lw::shaders::CCTWhiteBalanceShaderSettings<TColor, TKelvinToRgbStrategy>` | New Alias: 
- [ ] `lw::shaders::CCTWhiteBalanceShader<TColor, TKelvinToRgbStrategy>` | New Alias: 
- [ ] `lw::shaders::KelvinToRgbExactStrategy<TComponent>` | New Alias: 
- [ ] `lw::shaders::KelvinToRgbLut64Strategy<TComponent>` | New Alias: 

## Palette API (`lw`)

- [ ] `lw::colors::palettes::PaletteSampleOptions` | New Alias: 
- [ ] `lw::colors::palettes::PaletteStop<TColor>` | New Alias: 
- [ ] `lw::colors::palettes::Palette<TColor>` | New Alias: Palette<TColor>
- [ ] `lw::colors::palettes::SolidPaletteGenerator<TColor>` | New Alias: 
- [ ] `lw::colors::palettes::RainbowPaletteGenerator<TColor>` | New Alias: 
- [ ] `lw::colors::palettes::RandomSmoothPaletteGenerator<TColor>` | New Alias: 
- [ ] `lw::colors::palettes::RandomCyclePaletteGenerator<TColor>` | New Alias: 
- Blend operation structs:
- [ ] `lw::colors::palettes::BlendOpLinear` | New Alias: PaletteBlend::Linear
- [ ] `lw::colors::palettes::BlendOpStep` | New Alias: PaletteBlend::Step
- [ ] `lw::colors::palettes::BlendOpHoldMidpoint` | New Alias: PaletteBlend::HoldMidpoint
- [ ] `lw::colors::palettes::BlendOpSmoothstep` | New Alias: PaletteBlend::SmoothStep
- [ ] `lw::colors::palettes::BlendOpCubic` | New Alias: PaletteBlend::Cubic
- [ ] `lw::colors::palettes::BlendOpCosine` | New Alias: PaletteBlend::Cosine
- [ ] `lw::colors::palettes::BlendOpGammaLinear` | New Alias: PaletteBlend::GammaLinear
- [ ] `lw::colors::palettes::BlendOpQuantized<TLevels>` | New Alias: PaletteBlend::Quantized<TLevels>
- [ ] `lw::colors::palettes::BlendOpDitheredLinear` | New Alias: PaletteBlend::DitheredLinear
- Nearest policies:
- [ ] `lw::colors::palettes::NearestTieStable` | New Alias: BlendSampling::Stable
- [ ] `lw::colors::palettes::NearestTieLeft` | New Alias: BlendSampling::Left
- [ ] `lw::colors::palettes::NearestTieRight` | New Alias: BlendSampling:Right
- Wrap policies:
- [ ] `lw::colors::palettes::WrapClamp` | New Alias: BlendWrap::Clamp
- [ ] `lw::colors::palettes::WrapCircular` | New Alias: BlendWrap::Circular
- [ ] `lw::colors::palettes::WrapMirror` | New Alias: BlendWrap::Mirror
- [ ] `lw::colors::palettes::WrapHoldFirst` | New Alias: BlendWrap::HoldFirst
- [ ] `lw::colors::palettes::WrapHoldLast` | New Alias: BlendWrap::HoldLast
- [ ] `lw::colors::palettes::WrapBlackout` | New Alias: BlendWrap::Blackout
- [ ] `lw::colors::palettes::WrapWindow` | New Alias: BlendWrap::Window
- [ ] `lw::colors::palettes::WrapModuloSpan` | New Alias: BlendWrap::ModuloSpan
- [ ] `lw::colors::palettes::WrapOffsetCircular` | New Alias: BlendWrap::OffsetCirular

## Light Drivers (`lw`)

- [ ] `lw::transports::LightDriverSettingsBase` | New Alias: 
- [ ] `lw::transports::ILightDriver<TColor>` | New Alias: 
- [ ] `lw::transports::NilLightDriverSettings` | New Alias: Light::NoneSettings
- [ ] `lw::transports::NilLightDriver<TColor>` | New Alias: Light::None
- [ ] `lw::transports::PrintLightDriverSettingsT<TWritable>` | New Alias: 
- [ ] `lw::transports::PrintLightDriverT<TColor, TWritable>` | New Alias:  Light::PrintSettings
- [ ] `lw::transports::PrintLightDriverSettings` (Arduino) | New Alias: 
- [ ] `lw::transports::PrintLightDriver<TColor>` (Arduino) | New Alias: Light::Print
- [ ] `lw::transports::AnalogPwmLightDriverSettings` (platform-gated) | New Alias:
- [ ] `lw::transports::AnalogPwmLightDriver<TColor>` (platform-gated) | New Alias:
- [ ] `lw::transports::Esp32LedcLightDriverSettings` (platform-gated) | New Alias: 
- [ ] `lw::transports::Esp32LedcLightDriver<TColor>` (platform-gated) | New Alias: 
- [ ] `lw::transports::Esp32SigmaDeltaLightDriverSettings` (platform-gated) | New Alias: 
- [ ] `lw::transports::Esp32SigmaDeltaLightDriver<TColor>` (platform-gated) | New Alias: 
- [ ] `lw::transports::RpPwmLightDriverSettings` (platform-gated) | New Alias: 
- [ ] `lw::transports::RpPwmLightDriver<TColor>` (platform-gated) | New Alias: 

## Transport API (`lw`)

- [ ] `lw::transports::TransportSettingsBase` | New Alias: 
- [ ] `lw::transports::ITransport` | New Alias: 
- [ ] `lw::transports::TransportSettingsWithInvert<TTransportSettings>` | New Alias: 
- [ ] `lw::transports::TransportLike<TTransport>` | New Alias: 
- [ ] `lw::transports::NilTransportSettings` | New Alias: Transport::NoneSettings
- [ ] `lw::transports::NilTransport` | New Alias: Transport::None
- [ ] `lw::transports::SpiTransportSettings` | New Alias: 
- [ ] `lw::transports::SpiTransport` | New Alias: 
- [ ] `lw::transports::PrintTransportSettingsT<TWritable>` | New Alias: Transport::PrintSettings
- [ ] `lw::transports::PrintTransportT<TWritable>` | New Alias: Transport::Print
- [ ] `lw::transports::PrintTransportSettings` | New Alias: Trasnsport::SerialSettings
- [ ] `lw::transports::PrintTransport` | New Alias: Transport::Serial
- [ ] `lw::transports::OneWireTiming` | New Alias: OneWireTiming
- [ ] `lw::transports::OneWireEncoding<TTransportSettings>` | New Alias: 
- [ ] `lw::transports::EncodedClockDataBitPattern` | New Alias: 
- [ ] `lw::transports::timing` namespace constants (from `OneWireTiming.h`) | New Alias: 
- RP2040 transports (platform-gated):
- [ ] `lw::transports::rp2040::RpPioTransportSettings` | New Alias: 
- [ ] `lw::transports::rp2040::RpPioTransport` | New Alias: 
- [ ] `lw::transports::rp2040::RpSpiTransportSettings` | New Alias: 
- [ ] `lw::transports::rp2040::RpSpiTransport` | New Alias: 
- [ ] `lw::transports::rp2040::RpUartTransportSettings` | New Alias: 
- [ ] `lw::transports::rp2040::RpUartTransport` | New Alias: 
- ESP32 transports (platform-gated):
- [ ] `lw::transports::esp32::Esp32DmaSpiTransportSettings` | New Alias: 
- [ ] `lw::transports::esp32::Esp32DmaSpiTransport` | New Alias: 
- [ ] `lw::transports::esp32::Esp32I2sTransportSettings` | New Alias: 
- [ ] `lw::transports::esp32::Esp32I2sTransport` | New Alias: 
- [ ] `lw::transports::esp32::Esp32RmtTransportSettings` | New Alias: 
- [ ] `lw::transports::esp32::Esp32RmtTransport` | New Alias: 
- ESP8266 transports (platform-gated):
- [ ] `lw::transports::esp8266::Esp8266DmaI2sTransportSettings` | New Alias: 
- [ ] `lw::transports::esp8266::Esp8266DmaI2sTransport` | New Alias: 
- [ ] `lw::transports::esp8266::Esp8266DmaUartTransportSettings` | New Alias: 
- [ ] `lw::transports::esp8266::Esp8266DmaUartTransport` | New Alias: 

## Protocol API (`lw`)

- [ ] `lw::protocols::ProtocolSettings` | New Alias: 
- [ ] `lw::protocols::IProtocol<TColor>` | New Alias: 
- [ ] `lw::protocols::ProtocolType<TProtocol>` | New Alias: 
- [ ] `lw::protocols::ProtocolExternalBufferRequired<TProtocol>` | New Alias: 
- [ ] `lw::protocols::ProtocolRequiredBufferSizeComputable<TProtocol>` | New Alias: 
- [ ] `lw::protocols::ProtocolDecoratorBase<TDerived, TWrappedProtocol, TColor>` | New Alias: 
- [ ] `lw::protocols::DebugProtocolSettingsT<TWrappedProtocol, TWritable>` | New Alias:
- [ ] `lw::protocols::DebugProtocol<TWrappedProtocol, TWritable>` | New Alias:
- [ ] `lw::protocols::Apa102ProtocolSettings` | New Alias: 
- [ ] `lw::protocols::Apa102Protocol<TInterfaceColor, TStripColor>` | New Alias: 
- [ ] `lw::protocols::Hd108ProtocolSettings` | New Alias: 
- [ ] `lw::protocols::Hd108Protocol<TInterfaceColor, TStripColor>` | New Alias: 
- [ ] `lw::protocols::Lpd6803ProtocolSettings` | New Alias: 
- [ ] `lw::protocols::Lpd6803ProtocolT<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Lpd6803Protocol` | New Alias: 
- [ ] `lw::protocols::Lpd8806ProtocolSettings` | New Alias: 
- [ ] `lw::protocols::Lpd8806ProtocolT<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Lpd8806Protocol` | New Alias: 
- [ ] `lw::protocols::NilProtocolSettings` | New Alias: 
- [ ] `lw::protocols::NilProtocol<TColor>` | New Alias: 
- [ ] `lw::protocols::P9813ProtocolSettings` | New Alias: 
- [ ] `lw::protocols::P9813ProtocolT<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::P9813Protocol` | New Alias: 
- [ ] `lw::protocols::PixieProtocolSettings` | New Alias: 
- [ ] `lw::protocols::PixieProtocolT<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::PixieProtocol` | New Alias: 
- [ ] `lw::protocols::Sm16716ProtocolSettings` | New Alias: 
- [ ] `lw::protocols::Sm16716ProtocolT<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Sm16716Protocol` | New Alias: 
- [ ] `lw::protocols::Sm168xProtocolSettings` | New Alias: 
- [ ] `lw::protocols::Sm168xProtocol<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Tlc59711Settings` | New Alias: 
- [ ] `lw::protocols::Tlc59711ProtocolSettings` | New Alias: 
- [ ] `lw::protocols::Tlc59711ProtocolT<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Tlc59711Protocol` | New Alias: 
- [ ] `lw::protocols::Tm1814CurrentSettings` | New Alias: 
- [ ] `lw::protocols::Tm1814ProtocolSettings` | New Alias: 
- [ ] `lw::protocols::Tm1814ProtocolT<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Tm1814Protocol` | New Alias: 
- [ ] `lw::protocols::Tm1914Mode` | New Alias: 
- [ ] `lw::protocols::Tm1914ProtocolSettings` | New Alias: 
- [ ] `lw::protocols::Tm1914ProtocolT<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Tm1914Protocol` | New Alias: 
- [ ] `lw::protocols::Ws2801ProtocolSettings` | New Alias: 
- [ ] `lw::protocols::Ws2801ProtocolT<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Ws2801Protocol` | New Alias: 
- [ ] `lw::protocols::Ws2812xProtocolSettings` | New Alias: 
- [ ] `lw::protocols::Ws2812xProtocol<TInterfaceColor, TStripColor>` | New Alias: 

## Protocol Alias Layer (`lw::protocols`)

- [ ] `lw::protocols::detail::ResolveProtocolType<TProtocolCandidate>` | New Alias: 
- [ ] `lw::protocols::DotStar<...>` | New Alias: 
- [ ] `lw::protocols::DotStarType<...>` | New Alias: 
- [ ] `lw::protocols::APA102` | New Alias: Protocol::Apa102
- [ ] `lw::protocols::APA102Type` | New Alias: 
- [ ] `lw::protocols::Hd108<...>` | New Alias: 
- [ ] `lw::protocols::Hd108Type<...>` | New Alias: 
- [ ] `lw::protocols::HD108` | New Alias: Hd108
- [ ] `lw::protocols::HD108Type` | New Alias: 
- [ ] `lw::protocols::None<TColor>` | New Alias: 
- [ ] `lw::protocols::NoneType<TColor>` | New Alias: 
- [ ] `lw::protocols::Debug<TWrappedProtocolSpec>` | New Alias: 
- [ ] `lw::protocols::DebugType<TWrappedProtocolSpec>` | New Alias: 
- [ ] `lw::protocols::Tm1814<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Tm1814Type<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Tm1914<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Tm1914Type<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Ws2812x<...>` | New Alias: 
- [ ] `lw::protocols::Ws2812xType<...>` | New Alias: 
- [ ] `lw::protocols::Ws2812<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Ws2812Type<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Ws2811<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Ws2811Type<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Ws2805<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Ws2805Type<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Sk6812<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Sk6812Type<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Tm1829<TInterfaceColor>` | New Alias: 
- [ ] `lw::protocols::Tm1829Type<TInterfaceColor>` | New Alias: 

## Bus API (`lw`)

- [ ] `lw::busses::PlatformDefaultStaticBusDriverTransport` | New Alias: Transport::Default
- [ ] `lw::busses::PlatformDefaultStaticBusDriverTransportSettings` | New Alias: Transport::DefaultSettings
- [ ] `lw::busses::PixelBus<TProtocol, TTransport, TShader>` | New Alias: PixelBus<TProtocol,TTransport,TShader>
- [ ] `lw::busses::AggregateBus<TColor>` | New Alias: AggregateBus<TColor>
- [ ] `lw::busses::LightBus<TColor, TDriver, TShader>` | New Alias: Light<TColor>
- [ ] `lw::busses::ReferenceBus<TColor, TProtocol, TTransport, TShader>` | New Alias: 
