#pragma once

#include <cstdint>

#include "protocols/Tm1914Protocol.h"
#include "factory/traits/ProtocolTransportRateMutation.h"
#include "factory/traits/ProtocolDescriptorTraits.h"

namespace npb
{
namespace factory
{

    template <>
    struct ProtocolDescriptorTraits<npb::Tm1914Protocol, void>
        : ProtocolDescriptorTraitDefaults<typename npb::Tm1914Protocol::SettingsType>
    {
        using ProtocolType = npb::Tm1914Protocol;
        using SettingsType = typename ProtocolType::SettingsType;
        using ColorType = typename ProtocolType::ColorType;
        using Base = ProtocolDescriptorTraitDefaults<SettingsType>;
        using Base::defaultSettings;
        using Base::fromConfig;
        using Base::mutateTransportSettings;

        static SettingsType normalize(SettingsType settings)
        {
            settings.channelOrder = Base::template normalizeChannelOrder<ColorType>(
                settings.channelOrder,
                ChannelOrder::GRB::value);
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
