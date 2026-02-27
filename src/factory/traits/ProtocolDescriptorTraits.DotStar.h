#pragma once

#include "protocols/DotStarProtocol.h"
#include "factory/descriptors/ProtocolDescriptors.h"
#include "factory/traits/ProtocolDescriptorTraits.h"

namespace npb
{
namespace factory
{

    struct DotStarOptions
    {
        const char *channelOrder = ChannelOrder::BGR;
        DotStarMode mode = DotStarMode::FixedBrightness;
    };

    struct DotStarChannelOrderBGR
    {
        static constexpr const char *value = ChannelOrder::BGR;
    };

    struct DotStarChannelOrderRGB
    {
        static constexpr const char *value = ChannelOrder::RGB;
    };

    struct DotStarChannelOrderGRB
    {
        static constexpr const char *value = ChannelOrder::GRB;
    };

    struct DotStarChannelOrderRGBW
    {
        static constexpr const char *value = ChannelOrder::RGBW;
    };

    struct DotStarChannelOrderGRBW
    {
        static constexpr const char *value = ChannelOrder::GRBW;
    };

    struct DotStarChannelOrderBGRW
    {
        static constexpr const char *value = ChannelOrder::BGRW;
    };

    template <typename TDefaultChannelOrder = DotStarChannelOrderBGR>
    struct DotStarOptionsT : DotStarOptions
    {
        DotStarOptionsT()
            : DotStarOptions{}
        {
            channelOrder = TDefaultChannelOrder::value;
        }
    };

    template <>
    struct ProtocolDescriptorTraits<descriptors::DotStar, void>
    {
        using ProtocolType = npb::DotStarProtocol;
        using SettingsType = typename ProtocolType::SettingsType;
        using ColorType = typename ProtocolType::ColorType;

        static SettingsType defaultSettings()
        {
            return SettingsType{};
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
            SettingsType settings{};
            settings.channelOrder = config.channelOrder;
            settings.mode = config.mode;
            return settings;
        }
    };

} // namespace factory
} // namespace npb
