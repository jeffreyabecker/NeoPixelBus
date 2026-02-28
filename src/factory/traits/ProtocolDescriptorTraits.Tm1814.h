#pragma once

#include <cstdint>
#include <type_traits>

#include "protocols/Tm1814Protocol.h"
#include "factory/traits/ProtocolTransportRateMutation.h"
#include "factory/traits/ProtocolDescriptorTraits.h"

namespace npb
{
namespace factory
{

    template <>
    struct ProtocolDescriptorTraits<npb::Tm1814Protocol, void>
        : ProtocolDescriptorTraitDefaults<typename npb::Tm1814Protocol::SettingsType>
    {
        using ProtocolType = npb::Tm1814Protocol;
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
                "WRGB");
            return settings;
        }

        template <typename TTransportSettings>
        static void mutateTransportSettings(uint16_t,
                                            const SettingsType &protocolSettings,
                                            TTransportSettings &transportSettings)
        {
            applyEncodedOneWireRateIfUnset(protocolSettings.timing.encodedDataRateHz(), transportSettings);
        }
    };

} // namespace factory
} // namespace npb
