#pragma once

#include "colors/Color.h"
#include "transports/ITransport.h"
#include "transports/OneWireTiming.h"

namespace lw
{
namespace factory
{
namespace descriptors
{

    template <typename TColor = lw::Rgb8Color,
              typename TCapabilityRequirement = lw::TransportTag,
              typename TDefaultChannelOrder = lw::ChannelOrder::BGR>
    struct DotStar
    {
        using ColorType = TColor;
        using CapabilityRequirement = TCapabilityRequirement;
        using DefaultChannelOrder = TDefaultChannelOrder;
    };

    using APA102 = DotStar<lw::Rgb8Color,
                           lw::TransportTag,
                           lw::ChannelOrder::BGR>;

    template <typename TInterfaceColor = lw::Rgb8Color,
              typename TDefaultChannelOrder = lw::ChannelOrder::GRB,
              const OneWireTiming *TDefaultTiming = &timing::Generic800,
              typename TStripColor = TInterfaceColor>
    struct Ws2812x
    {
        using InterfaceColorType = TInterfaceColor;
        using StripColorType = TStripColor;
        using ColorType = InterfaceColorType;
        using CapabilityRequirement = lw::OneWireTransportTag;
        using DefaultChannelOrder = TDefaultChannelOrder;
        static constexpr const OneWireTiming *DefaultTiming = TDefaultTiming;
    };

    
    struct Ws2812 : public Ws2812x<lw::Rgb8Color,
                                   lw::ChannelOrder::GRB,
                                   &timing::Generic800,
                                   lw::Rgb8Color>
    {

    };

    struct Ws2811 : public Ws2812x<lw::Rgb8Color,
                                   lw::ChannelOrder::GRB,
                                   &timing::Generic800,
                                   lw::Rgb8Color>
    {

    };

    struct Ws2805 : public Ws2812x<lw::Rgb8Color,
                                   lw::ChannelOrder::GRB,
                                   &timing::Generic800,
                                   lw::Rgb8Color>
    {

    };

    struct Sk6812 : public Ws2812x<lw::Rgb8Color,
                                   lw::ChannelOrder::GRB,
                                   &timing::Sk6812,
                                   lw::Rgb8Color>
    {

    };

    struct Tm1814 : public Ws2812x<lw::Rgb8Color,
                                   lw::ChannelOrder::GRB,
                                   &timing::Generic800,
                                   lw::Rgb8Color>
    {

    };

    struct Tm1914 : public Ws2812x<lw::Rgb8Color,
                                   lw::ChannelOrder::GRB,
                                   &timing::Generic800,
                                   lw::Rgb8Color>
    {

    };

    struct Tm1829 : public Ws2812x<lw::Rgb8Color,
                                   lw::ChannelOrder::GRB,
                                   &timing::Generic800,
                                   lw::Rgb8Color>
    {

    };

    struct Apa106 : public Ws2812x<lw::Rgb8Color,
                                   lw::ChannelOrder::GRB,
                                   &timing::Apa106,
                                   lw::Rgb8Color>
    {

    };

    struct Tx1812 : public Ws2812x<lw::Rgb8Color,
                                   lw::ChannelOrder::GRB,
                                   &timing::Tx1812,
                                   lw::Rgb8Color>
    {

    };

    struct Gs1903 : public Ws2812x<lw::Rgb8Color,
                                   lw::ChannelOrder::GRB,
                                   &timing::Gs1903,
                                   lw::Rgb8Color>
    {
    };

    struct Generic800 : public Ws2812x<lw::Rgb8Color,
                                       lw::ChannelOrder::GRB,
                                       &timing::Generic800,
                                       lw::Rgb8Color>
    {
    };

    struct Generic400 : public Ws2812x<lw::Rgb8Color,
                                       lw::ChannelOrder::GRB,
                                       &timing::Generic400,
                                       lw::Rgb8Color>
    {
    };

    struct Ws2816 : public Ws2812x<lw::Rgb8Color,
                                   lw::ChannelOrder::GRB,
                                   &timing::Ws2816,
                                   lw::Rgb8Color>
    {
    };

    struct Ws2813 : public Ws2812x<lw::Rgb8Color,
                                   lw::ChannelOrder::GRB,
                                   &timing::Ws2813,
                                   lw::Rgb8Color>
    {
    };

    struct Ws2814 : public Ws2812x<lw::Rgb8Color,
                                   lw::ChannelOrder::GRB,
                                   &timing::Ws2814,
                                   lw::Rgb8Color>
    {

    };

    struct Lc8812 : public Ws2812x<lw::Rgb8Color,
                                   lw::ChannelOrder::GRB,
                                   &timing::Lc8812,
                                   lw::Rgb8Color>
    {
    };

} // namespace descriptors
} // namespace factory
} // namespace lw
