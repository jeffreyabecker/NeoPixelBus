#pragma once

#include <cstdint>

#include "protocols/Tm1914Protocol.h"
#include "factory/traits/ProtocolDescriptorTraits.h"
#include "transports/OneWireWrapper.h"

namespace lw
{
namespace factory
{

    template <>
    struct ProtocolDescriptorTraits<lw::Tm1914Protocol, void>
        : ProtocolDescriptorTraitDefaults<typename lw::Tm1914Protocol::SettingsType>
    {
        using ProtocolType = lw::Tm1914Protocol;
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
            lw::normalizeOneWireTransportClockDataBitRate(protocolSettings.timing, transportSettings);
        }
    };

} // namespace factory
} // namespace lw
