#pragma once

#include "core/Core.h"
#include "colors/Colors.h"
#include "transports/Transports.h"
#include "protocols/Protocols.h"
#include "buses/Busses.h"

#ifndef LW_USE_EXPLICIT_NAMESPACES

using pixel_count_t = lw::PixelCount;
using Rgb8Color = lw::Rgb8Color;
using Rgbw8Color = lw::Rgbw8Color;
using Rgbcw8Color = lw::Rgbcw8Color;
using Rgb16Color = lw::Rgb16Color;
using Rgbw16Color = lw::Rgbw16Color;
using Rgbcw16Color = lw::Rgbcw16Color;
using Color = lw::colors::DefaultColorType;
using HsbColor = lw::colors::HsbColor;
using HslColor = lw::colors::HslColor;

template <typename TProtocol, typename TTransport = lw::busses::PlatformDefaultTransport,
          typename TShader =
              lw::NilShader<typename lw::busses::detail::ResolveProtocolType<TProtocol>::Type::ColorType>>
using Strip = lw::busses::PixelBus<TProtocol, TTransport, TShader>;

template <typename TColor = lw::colors::DefaultColorType,
          typename TDriver = lw::transports::PlatformDefaultLightDriver<TColor>,
          typename TShader = lw::NilShader<TColor>>
using Light = lw::busses::LightBus<TColor, TDriver, TShader>;

template <typename... TBuses> using CompositeStrip = lw::busses::CompositeBus<TBuses...>;

template <typename TColor = lw::colors::DefaultColorType> using AggregateStrip = lw::busses::AggregateBus<TColor>;

namespace HueBlend
{

using Shortest = lw::colors::HueBlendShortestDistance;
using Longest = lw::colors::HueBlendLongestDistance;
using Clockwise = lw::colors::HueBlendClockwiseDirection;
using CounterClockwise = lw::colors::HueBlendCounterClockwiseDirection;

} // namespace HueBlend

namespace ChannelOrder
{

using RGB = lw::colors::ChannelOrder::RGB;
using GRB = lw::colors::ChannelOrder::GRB;
using BGR = lw::colors::ChannelOrder::BGR;
using RGBW = lw::colors::ChannelOrder::RGBW;
using GRBW = lw::colors::ChannelOrder::GRBW;
using BGRW = lw::colors::ChannelOrder::BGRW;
using WRGB = lw::colors::ChannelOrder::WRGB;
using RGBCW = lw::colors::ChannelOrder::RGBCW;
using GRBCW = lw::colors::ChannelOrder::GRBCW;
using BGRCW = lw::colors::ChannelOrder::BGRCW;

} // namespace ChannelOrder

namespace Palette
{

template <typename TColor = lw::colors::DefaultColorType> using Palette = lw::colors::palettes::Palette<TColor>;

} // namespace Palette

namespace PaletteBlend
{

using Linear = lw::colors::palettes::BlendOpLinear;
using Step = lw::colors::palettes::BlendOpStep;
using HoldMidpoint = lw::colors::palettes::BlendOpHoldMidpoint;
using SmoothStep = lw::colors::palettes::BlendOpSmoothstep;
using Cubic = lw::colors::palettes::BlendOpCubic;
using Cosine = lw::colors::palettes::BlendOpCosine;
using GammaLinear = lw::colors::palettes::BlendOpGammaLinear;
template <size_t TLevels> using Quantized = lw::colors::palettes::BlendOpQuantized<TLevels>;
using DitheredLinear = lw::colors::palettes::BlendOpDitheredLinear;

} // namespace PaletteBlend

namespace BlendSampling
{

using Stable = lw::colors::palettes::NearestTieStable;
using Left = lw::colors::palettes::NearestTieLeft;
using Right = lw::colors::palettes::NearestTieRight;

} // namespace BlendSampling

namespace BlendWrap
{

using Clamp = lw::colors::palettes::WrapClamp;
using Circular = lw::colors::palettes::WrapCircular;
using Mirror = lw::colors::palettes::WrapMirror;
using HoldFirst = lw::colors::palettes::WrapHoldFirst;
using HoldLast = lw::colors::palettes::WrapHoldLast;
using Blackout = lw::colors::palettes::WrapBlackout;
template <uint8_t TStart = 0, uint8_t TEnd = 255> using Window = lw::colors::palettes::WrapWindow<TStart, TEnd>;
template <uint8_t TStart = 0, uint8_t TEnd = 255> using ModuloSpan = lw::colors::palettes::WrapModuloSpan<TStart, TEnd>;
template <uint8_t TOffset = 0> using OffsetCircular = lw::colors::palettes::WrapOffsetCircular<TOffset>;

} // namespace BlendWrap

namespace Shader
{

template <typename TColor = lw::colors::DefaultColorType> using Interface = lw::shaders::IShader<TColor>;

template <typename TColor = lw::colors::DefaultColorType> using None = lw::shaders::NilShader<TColor>;

template <typename TColor = lw::colors::DefaultColorType>
using AggregateSettings = lw::shaders::AggregateShaderSettings<TColor>;

template <typename TColor = lw::colors::DefaultColorType> using Aggregate = lw::shaders::AggregateShader<TColor>;

template <typename TColor = lw::colors::DefaultColorType, typename... TShaders>
using Composite = lw::shaders::OwningAggregateShaderT<TColor, TShaders...>;

template <typename TColor = lw::colors::DefaultColorType>
using GammaSettings = lw::shaders::GammaShaderSettings<TColor>;

template <typename TColor = lw::colors::DefaultColorType> using Gamma = lw::shaders::GammaShader<TColor>;

template <typename TColor = lw::colors::DefaultColorType>
using CurrentSettings = lw::shaders::CurrentLimiterShaderSettings<TColor>;

template <typename TColor = lw::colors::DefaultColorType> using Current = lw::shaders::CurrentLimiterShader<TColor>;

template <typename TColor = lw::colors::DefaultColorType>
using AutoWhiteBalanceSettings = lw::shaders::AutoWhiteBalanceShaderSettings<TColor>;

template <typename TColor = lw::colors::DefaultColorType,
          template <typename> class TKelvinToRgbStrategy = lw::KelvinToRgbExactStrategy>
using AutoWhiteBalance = lw::shaders::AutoWhiteBalanceShader<TColor, TKelvinToRgbStrategy>;

using CCTInterlock = lw::shaders::CCTColorInterlock;

template <typename TColor = lw::colors::DefaultColorType,
          template <typename> class TKelvinToRgbStrategy = lw::KelvinToRgbLut64Strategy>
using CCTBalanceSettings = lw::shaders::CCTWhiteBalanceShaderSettings<TColor, TKelvinToRgbStrategy>;

template <typename TColor = lw::colors::DefaultColorType,
          template <typename> class TKelvinToRgbStrategy = lw::KelvinToRgbLut64Strategy>
using CCTBalance = lw::shaders::CCTWhiteBalanceShader<TColor, TKelvinToRgbStrategy>;

template <typename TComponent> using KelvinToRgbExact = lw::KelvinToRgbExactStrategy<TComponent>;

template <typename TComponent> using KelvinToRgbLut64 = lw::KelvinToRgbLut64Strategy<TComponent>;

} // namespace Shader

namespace Protocols
{

using APA102 = lw::protocols::DotStar<lw::Rgb8Color, lw::ChannelOrder::BGR, lw::Rgb8Color>;

using HD107S = APA102;

using HD108 = lw::protocols::Hd108<lw::Rgb16Color, lw::ChannelOrder::BGR, lw::Rgb16Color>;

template <typename TInterfaceColor = lw::Rgb8Color> using Lpd6803 = lw::protocols::Lpd6803ProtocolT<TInterfaceColor>;
template <typename TInterfaceColor = lw::Rgb8Color> using Sm16716 = lw::protocols::Sm16716ProtocolT<TInterfaceColor>;
template <typename TInterfaceColor = lw::Rgb8Color, typename TStripColor = lw::Rgb8Color>
using Ws2801 = lw::protocols::Ws2801ProtocolT<TInterfaceColor, TStripColor>;
template <typename TInterfaceColor = lw::Rgb8Color, typename TStripColor = lw::Rgb8Color>
using Ws2801S = Ws2801<TInterfaceColor, TStripColor>;

template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Ws2812 = lw::protocols::Ws2812x<TInterfaceColor, lw::ChannelOrder::GRB, &lw::transports::timing::Generic800,
                                      lw::Rgb8Color, false>;

template <typename TInterfaceColor = lw::colors::DefaultColorType> using Apa107 = Ws2812<TInterfaceColor>;
template <typename TInterfaceColor = lw::colors::DefaultColorType> using Hc2912 = Ws2812<TInterfaceColor>;

template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Ws2811 = lw::protocols::Ws2812x<TInterfaceColor, lw::ChannelOrder::RGB, &lw::transports::timing::Ws2811,
                                      lw::Rgb8Color, false>;

template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Ws2813 = lw::protocols::Ws2812x<TInterfaceColor, lw::ChannelOrder::RGB, &lw::transports::timing::Ws2813,
                                      lw::Rgb8Color, false>;

template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Ws2813Rgbw = lw::protocols::Ws2812x<TInterfaceColor, lw::ChannelOrder::GRBW, &lw::transports::timing::Ws2813,
                                          lw::Rgbw8Color, false>;

template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Ws2805 = lw::protocols::Ws2812x<TInterfaceColor, lw::ChannelOrder::RGBCW, &lw::transports::timing::Ws2805,
                                      lw::Rgbcw8Color, false>;

template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Sk6812 = lw::protocols::Ws2812x<TInterfaceColor, lw::ChannelOrder::GRB, &lw::transports::timing::Sk6812,
                                      lw::Rgb8Color, false>;
template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Sk6812White = lw::protocols::Ws2812x<TInterfaceColor, lw::ChannelOrder::RGB, &lw::transports::timing::Sk6812,
                                           lw::Rgb8Color, false>;
template <typename TInterfaceColor = lw::colors::DefaultColorType> using Sk6813 = Sk6812<TInterfaceColor>;

template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Lc8812 = lw::protocols::Ws2812x<TInterfaceColor, lw::ChannelOrder::GRB, &lw::transports::timing::Lc8812,
                                      lw::Rgb8Color, false>;

template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Tm1829 = lw::protocols::Ws2812x<TInterfaceColor, lw::ChannelOrder::RGB, &lw::transports::timing::Tm1829,
                                      lw::Rgb8Color, true>;

template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Tm1814 = lw::protocols::Tm1814<TInterfaceColor>;

template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Tm1914 = lw::protocols::Tm1914<TInterfaceColor>;

template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Ws2814 = lw::protocols::Ws2812x<TInterfaceColor, lw::ChannelOrder::RGBW, &lw::transports::timing::Ws2814,
                                      lw::Rgbw8Color, false>;
template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Ws2814A = lw::protocols::Ws2812x<TInterfaceColor, lw::ChannelOrder::WRGB, &lw::transports::timing::Ws2814,
                                       lw::Rgbw8Color, false>;

template <typename TInterfaceColor = lw::colors::DefaultColorType> using Ws2815 = Ws2812<TInterfaceColor>;

template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Ws2816 = lw::protocols::Ws2812x<TInterfaceColor, lw::ChannelOrder::GRB, &lw::transports::timing::Ws2816,
                                      lw::Rgb8Color, false>;

template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Ws2818 = lw::protocols::Ws2812x<TInterfaceColor, lw::ChannelOrder::RGB, &lw::transports::timing::Generic800,
                                      lw::Rgb8Color, false>;

template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Apa106 = lw::protocols::Ws2812x<TInterfaceColor, lw::ChannelOrder::RGB, &lw::transports::timing::Apa106,
                                      lw::Rgb8Color, false>;

template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Tx1812 = lw::protocols::Ws2812x<TInterfaceColor, lw::ChannelOrder::RGB, &lw::transports::timing::Tx1812,
                                      lw::Rgb8Color, false>;

template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Gs1903 = lw::protocols::Ws2812x<TInterfaceColor, lw::ChannelOrder::RGB, &lw::transports::timing::Gs1903,
                                      lw::Rgb8Color, false>;

template <typename TInterfaceColor = lw::colors::DefaultColorType>
using Tm1803 = lw::protocols::Ws2812x<TInterfaceColor, lw::ChannelOrder::RGB, &lw::transports::timing::Generic400,
                                      lw::Rgb8Color, false>;
template <typename TInterfaceColor = lw::colors::DefaultColorType> using Tm1804 = Tm1803<TInterfaceColor>;
template <typename TInterfaceColor = lw::colors::DefaultColorType> using Tm1809 = Tm1803<TInterfaceColor>;

} // namespace Protocols

namespace Driver
{

template <typename TColor = lw::colors::DefaultColorType>
using PlatformDefault = lw::transports::PlatformDefaultLightDriver<TColor>;

} // namespace Driver

namespace Transport
{

using Default = lw::busses::PlatformDefaultTransport;
using DefaultSettings = lw::busses::PlatformDefaultTransportSettings;

} // namespace Transport

#endif // LW_USE_EXPLICIT_NAMESPACES
