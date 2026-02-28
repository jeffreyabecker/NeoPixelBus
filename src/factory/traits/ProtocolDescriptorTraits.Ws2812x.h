#pragma once

#include <cstdint>
#include <type_traits>

#include "protocols/Ws2812xProtocol.h"
#include "factory/descriptors/ProtocolDescriptors.h"
#include "factory/traits/ProtocolTransportRateMutation.h"
#include "factory/traits/ProtocolDescriptorTraits.h"

namespace npb
{
    namespace factory
    {

        struct Ws2812xOptions
        {
            const char *channelOrder = ChannelOrder::GRB::value;
            OneWireTiming timing = timing::Ws2812x;
        };

        template <typename TColor,
                  typename TCapabilityRequirement,
                  typename TDefaultChannelOrder>
        struct ProtocolDescriptorTraits<descriptors::Ws2812x<TColor, TCapabilityRequirement, TDefaultChannelOrder>, void>
            : ProtocolDescriptorTraitDefaults<typename npb::Ws2812xProtocol<TColor>::SettingsType>
        {
            using ProtocolType = npb::Ws2812xProtocol<TColor>;
            using SettingsType = typename ProtocolType::SettingsType;
            using ColorType = typename ProtocolType::ColorType;
            using Base = ProtocolDescriptorTraitDefaults<SettingsType>;
            using Base::defaultSettings;
            using Base::fromConfig;
            using Base::mutateTransportSettings;

            static_assert(std::is_same<TCapabilityRequirement, typename ProtocolType::TransportCategory>::value,
                          "Ws2812x descriptor capability requirement must match Ws2812xProtocol transport category.");

            static SettingsType normalize(SettingsType settings)
            {
                settings.channelOrder = Base::template normalizeChannelOrder<ColorType>(
                    settings.channelOrder,
                    TDefaultChannelOrder::value);
                return settings;
            }

            static SettingsType fromConfig(const Ws2812xOptions &config)
            {
                SettingsType settings = Base::defaultSettings();
                settings.channelOrder = config.channelOrder;
                settings.timing = config.timing;
                return settings;
            }

            template <typename TTransportSettings>
            static void mutateTransportSettings(uint16_t,
                                                const SettingsType &protocolSettings,
                                                TTransportSettings &transportSettings)
            {
                const uint32_t bitRateHz = static_cast<uint32_t>(protocolSettings.timing.bitRateHz());
                const uint32_t encodedBitsPerDataBit = static_cast<uint32_t>(protocolSettings.timing.bitPattern());
                const uint32_t encodedRateHz = bitRateHz * encodedBitsPerDataBit;
                applyEncodedOneWireRateIfUnset(encodedRateHz, transportSettings);
            }
        };

    } // namespace factory
} // namespace npb
