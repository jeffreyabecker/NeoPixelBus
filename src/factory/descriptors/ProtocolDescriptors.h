#pragma once

#include "colors/Color.h"
#include "transports/ITransport.h"

namespace npb
{
namespace factory
{
namespace descriptors
{

    struct ChannelOrderRGB
    {
        static constexpr const char *value = ChannelOrder::RGB;
    };

    struct ChannelOrderGRB
    {
        static constexpr const char *value = ChannelOrder::GRB;
    };

    struct ChannelOrderBGR
    {
        static constexpr const char *value = ChannelOrder::BGR;
    };

    struct ChannelOrderRGBW
    {
        static constexpr const char *value = ChannelOrder::RGBW;
    };

    struct ChannelOrderGRBW
    {
        static constexpr const char *value = ChannelOrder::GRBW;
    };

    struct ChannelOrderBGRW
    {
        static constexpr const char *value = ChannelOrder::BGRW;
    };

    template <typename TColor = npb::Rgb8Color,
              typename TCapabilityRequirement = npb::TransportTag,
              typename TDefaultChannelOrder = ChannelOrderBGR>
    struct DotStar
    {
        using ColorType = TColor;
        using CapabilityRequirement = TCapabilityRequirement;
        using DefaultChannelOrder = TDefaultChannelOrder;
    };

    using APA102 = DotStar<npb::Rgb8Color,
                           npb::TransportTag,
                           ChannelOrderBGR>;

    template <typename TColor = npb::Rgb8Color,
              typename TCapabilityRequirement = npb::OneWireTransportTag,
              typename TDefaultChannelOrder = ChannelOrderGRB>
    struct Ws2812x
    {
        using ColorType = TColor;
        using CapabilityRequirement = TCapabilityRequirement;
        using DefaultChannelOrder = TDefaultChannelOrder;
    };

} // namespace descriptors
} // namespace factory
} // namespace npb
