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

    template <typename TColor = lw::Rgb8Color,
              typename TCapabilityRequirement = lw::OneWireTransportTag,
              typename TDefaultChannelOrder = lw::ChannelOrder::GRB,
              const OneWireTiming *TDefaultTiming = &timing::Ws2812x>
    struct Ws2812x
    {
        using ColorType = TColor;
        using CapabilityRequirement = TCapabilityRequirement;
        using DefaultChannelOrder = TDefaultChannelOrder;
        static constexpr const OneWireTiming *DefaultTiming = TDefaultTiming;
    };


    struct Ws2812 : public Ws2812x<lw::Rgb8Color,
                                   lw::OneWireTransportTag,
                                   lw::ChannelOrder::GRB,
                                   &timing::Ws2812x>
    {

    };

    struct Ws2811 : public Ws2812x<lw::Rgb8Color,
                                   lw::OneWireTransportTag,
                                   lw::ChannelOrder::GRB,
                                   &timing::Ws2811>
    {

    };

    struct Ws2805 : public Ws2812x<lw::Rgb8Color,
                                   lw::OneWireTransportTag,
                                   lw::ChannelOrder::GRB,
                                   &timing::Ws2805>
    {

    };

    struct Sk6812 : public Ws2812x<lw::Rgb8Color,
                                   lw::OneWireTransportTag,
                                   lw::ChannelOrder::GRB,
                                   &timing::Sk6812>
    {

    };

    struct Tm1814 : public Ws2812x<lw::Rgb8Color,
                                   lw::OneWireTransportTag,
                                   lw::ChannelOrder::GRB,
                                   &timing::Tm1814>
    {

    };

    struct Tm1914 : public Ws2812x<lw::Rgb8Color,
                                   lw::OneWireTransportTag,
                                   lw::ChannelOrder::GRB,
                                   &timing::Tm1914>
    {

    };

    struct Tm1829 : public Ws2812x<lw::Rgb8Color,
                                   lw::OneWireTransportTag,
                                   lw::ChannelOrder::GRB,
                                   &timing::Tm1829>
    {

    };

    struct Apa106 : public Ws2812x<lw::Rgb8Color,
                                   lw::OneWireTransportTag,
                                   lw::ChannelOrder::GRB,
                                   &timing::Apa106>
    {

    };

    struct Tx1812 : public Ws2812x<lw::Rgb8Color,
                                   lw::OneWireTransportTag,
                                   lw::ChannelOrder::GRB,
                                   &timing::Tx1812>
    {

    };

    struct Gs1903 : public Ws2812x<lw::Rgb8Color,
                                   lw::OneWireTransportTag,
                                   lw::ChannelOrder::GRB,
                                   &timing::Gs1903>
    {
    };

    struct Generic800 : public Ws2812x<lw::Rgb8Color,
                                       lw::OneWireTransportTag,
                                       lw::ChannelOrder::GRB,
                                       &timing::Generic800>
    {
    };

    struct Generic400 : public Ws2812x<lw::Rgb8Color,
                                       lw::OneWireTransportTag,
                                       lw::ChannelOrder::GRB,
                                       &timing::Generic400>
    {
    };

    struct Ws2816 : public Ws2812x<lw::Rgb8Color,
                                   lw::OneWireTransportTag,
                                   lw::ChannelOrder::GRB,
                                   &timing::Ws2816>
    {
    };

    struct Ws2813 : public Ws2812x<lw::Rgb8Color,
                                   lw::OneWireTransportTag,
                                   lw::ChannelOrder::GRB,
                                   &timing::Ws2813>
    {
    };

    struct Ws2814 : public Ws2812x<lw::Rgb8Color,
                                   lw::OneWireTransportTag,
                                   lw::ChannelOrder::GRB,
                                   &timing::Ws2814>
    {

    };

    struct Lc8812 : public Ws2812x<lw::Rgb8Color,
                                   lw::OneWireTransportTag,
                                   lw::ChannelOrder::GRB,
                                   &timing::Lc8812>
    {
    };

} // namespace descriptors
} // namespace factory
} // namespace lw
