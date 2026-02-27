#pragma once

#include "protocols/Ws2812xProtocol.h"
#include "factory/descriptors/ProtocolDescriptors.h"
#include "factory/traits/ProtocolDescriptorTraits.h"

namespace npb
{
namespace factory
{

    struct Ws2812xOptions
    {
        const char *channelOrder = ChannelOrder::GRB;
    };

    template <typename TColor>
    struct ProtocolDescriptorTraits<descriptors::Ws2812x<TColor>, void>
    {
        using ProtocolType = npb::Ws2812xProtocol<TColor>;
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

        static SettingsType fromConfig(const Ws2812xOptions &config)
        {
            SettingsType settings{};
            settings.channelOrder = config.channelOrder;
            return settings;
        }
    };

} // namespace factory
} // namespace npb
