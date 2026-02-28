#pragma once

#include "colors/Color.h"
#include "transports/ITransport.h"

namespace npb
{
namespace factory
{
namespace descriptors
{

    template <typename TColor = npb::Rgb8Color,
              typename TCapabilityRequirement = npb::TransportTag,
              typename TDefaultChannelOrder = npb::ChannelOrder::BGR>
    struct DotStar
    {
        using ColorType = TColor;
        using CapabilityRequirement = TCapabilityRequirement;
        using DefaultChannelOrder = TDefaultChannelOrder;
    };

    using APA102 = DotStar<npb::Rgb8Color,
                           npb::TransportTag,
                           npb::ChannelOrder::BGR>;

    template <typename TColor = npb::Rgb8Color,
              typename TCapabilityRequirement = npb::OneWireTransportTag,
              typename TDefaultChannelOrder = npb::ChannelOrder::GRB>
    struct Ws2812x
    {
        using ColorType = TColor;
        using CapabilityRequirement = TCapabilityRequirement;
        using DefaultChannelOrder = TDefaultChannelOrder;
    };

} // namespace descriptors
} // namespace factory
} // namespace npb
