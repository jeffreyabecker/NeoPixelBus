#pragma once

#include <type_traits>

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

    template <typename TColor,
              typename TCapabilityRequirement,
              typename TDefaultChannelOrder>
    struct ProtocolDescriptorTraits<descriptors::Ws2812x<TColor, TCapabilityRequirement, TDefaultChannelOrder>, void>
    {
        using ProtocolType = npb::Ws2812xProtocol<TColor>;
        using SettingsType = typename ProtocolType::SettingsType;
        using ColorType = typename ProtocolType::ColorType;

        static_assert(std::is_same<TCapabilityRequirement, typename ProtocolType::TransportCategory>::value,
                      "Ws2812x descriptor capability requirement must match Ws2812xProtocol transport category.");

        static SettingsType defaultSettings()
        {
            SettingsType settings{};
            settings.channelOrder = TDefaultChannelOrder::value;
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

        static SettingsType fromConfig(const Ws2812xOptions &config)
        {
            SettingsType settings = defaultSettings();
            settings.channelOrder = (nullptr != config.channelOrder) ? config.channelOrder : TDefaultChannelOrder::value;
            return settings;
        }
    };

} // namespace factory
} // namespace npb
