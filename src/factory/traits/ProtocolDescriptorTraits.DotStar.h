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
        DotStarMode mode = DotStarMode::FixedBrightness;
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
    {
        using DescriptorType = descriptors::DotStar<TColor, TCapabilityRequirement, TDefaultChannelOrder>;
        using ProtocolType = npb::DotStarProtocol;
        using SettingsType = typename ProtocolType::SettingsType;
        using ColorType = typename DescriptorType::ColorType;

        static_assert(std::is_same<typename DescriptorType::ColorType, typename ProtocolType::ColorType>::value,
                      "DotStar descriptor currently supports Rgb8Color only.");
        static_assert(std::is_same<typename DescriptorType::CapabilityRequirement, typename ProtocolType::TransportCategory>::value,
                      "DotStar descriptor capability requirement must match DotStarProtocol transport category.");

        static SettingsType defaultSettings()
        {
            SettingsType settings{};
            settings.channelOrder = DescriptorType::DefaultChannelOrder::value;
            return settings;
        }

        static SettingsType normalize(SettingsType settings)
        {
            return settings;
        }

        static SettingsType fromConfig(SettingsType settings)
        {
            return settings;
        }

        static SettingsType fromConfig(const DotStarOptions &config)
        {
            SettingsType settings = defaultSettings();
            settings.channelOrder = (nullptr != config.channelOrder) ? config.channelOrder : settings.channelOrder;
            settings.mode = config.mode;
            return settings;
        }
    };

} // namespace factory
} // namespace npb
