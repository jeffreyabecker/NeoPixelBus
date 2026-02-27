#pragma once

#include <type_traits>

#include "protocols/DotStarProtocol.h"
#include "factory/descriptors/ProtocolDescriptors.h"
#include "factory/traits/ProtocolDescriptorTraits.h"

namespace npb
{
namespace factory
{

    struct DotStarOptions
    {
        const char *channelOrder = nullptr;
    };

    using DotStarDescriptorDefault = descriptors::DotStar<>;

    using DotStarChannelOrderBGR = descriptors::ChannelOrderBGR;
    using DotStarChannelOrderRGB = descriptors::ChannelOrderRGB;
    using DotStarChannelOrderGRB = descriptors::ChannelOrderGRB;
    using DotStarChannelOrderRGBW = descriptors::ChannelOrderRGBW;
    using DotStarChannelOrderGRBW = descriptors::ChannelOrderGRBW;
    using DotStarChannelOrderBGRW = descriptors::ChannelOrderBGRW;

    template <typename TDefaultChannelOrder = typename DotStarDescriptorDefault::DefaultChannelOrder>
    struct DotStarOptionsT : DotStarOptions
    {
        DotStarOptionsT()
            : DotStarOptions{}
        {
            channelOrder = TDefaultChannelOrder::value;
        }
    };

    template <typename TColor,
              typename TCapabilityRequirement,
              typename TDefaultChannelOrder>
    struct ProtocolDescriptorTraits<descriptors::DotStar<TColor, TCapabilityRequirement, TDefaultChannelOrder>, void>
        : ProtocolDescriptorTraitDefaults<typename npb::DotStarProtocol::SettingsType>
    {
        using DescriptorType = descriptors::DotStar<TColor, TCapabilityRequirement, TDefaultChannelOrder>;
        using ProtocolType = npb::DotStarProtocolT<typename DescriptorType::ColorType>;
        using SettingsType = typename ProtocolType::SettingsType;
        using ColorType = typename DescriptorType::ColorType;
        using Base = ProtocolDescriptorTraitDefaults<SettingsType>;
        using Base::defaultSettings;
        using Base::fromConfig;


        static_assert(std::is_same<typename DescriptorType::CapabilityRequirement, typename ProtocolType::TransportCategory>::value,
                      "DotStar descriptor capability requirement must match DotStarProtocol transport category.");

        static SettingsType normalize(SettingsType settings)
        {
            settings.channelOrder = Base::template normalizeChannelOrder<ColorType>(
                settings.channelOrder,
                DescriptorType::DefaultChannelOrder::value);

            return settings;
        }

        static SettingsType fromConfig(const DotStarOptions &config)
        {
            SettingsType settings = Base::defaultSettings();
            settings.channelOrder = config.channelOrder;
            return settings;
        }
    };

} // namespace factory
} // namespace npb
