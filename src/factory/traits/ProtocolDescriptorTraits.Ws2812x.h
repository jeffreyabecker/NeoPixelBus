#pragma once

#include <cstdint>
#include <type_traits>

#include "protocols/Ws2812xProtocol.h"
#include "factory/descriptors/ProtocolDescriptors.h"
#include "factory/traits/ProtocolDescriptorTraits.h"
#include "transports/OneWireWrapper.h"

namespace lw
{
    namespace factory
    {

        struct Ws2812xOptions
        {
            const char *channelOrder = ChannelOrder::GRB::value;
            OneWireTiming timing = timing::Ws2812x;
        };

        template <typename TInterfaceColor,
                  typename TDefaultChannelOrder,
                  const OneWireTiming *TDefaultTiming,
                  typename TStripColor,
                  bool TIdleHigh>
        struct ProtocolDescriptorTraits<descriptors::Ws2812x<TInterfaceColor, TDefaultChannelOrder, TDefaultTiming, TStripColor, TIdleHigh>, void>
            : ProtocolDescriptorTraitDefaults<typename lw::Ws2812xProtocol<std::conditional_t<lw::ColorAtLeastAsLarge<TInterfaceColor, TStripColor>,
                                                                                               TInterfaceColor,
                                                                                               TStripColor>,
                                                                    TStripColor>::SettingsType>
        {
            using EffectiveInterfaceColor = std::conditional_t<lw::ColorAtLeastAsLarge<TInterfaceColor, TStripColor>,
                                                               TInterfaceColor,
                                                               TStripColor>;
            using ProtocolType = lw::Ws2812xProtocol<EffectiveInterfaceColor, TStripColor>;
            using SettingsType = typename ProtocolType::SettingsType;
            using ColorType = typename ProtocolType::ColorType;
            using Base = ProtocolDescriptorTraitDefaults<SettingsType>;
            using Base::defaultSettings;
            using Base::fromConfig;
            using Base::mutateTransportSettings;

            static_assert(std::is_same<typename descriptors::Ws2812x<TInterfaceColor, TDefaultChannelOrder, TDefaultTiming, TStripColor, TIdleHigh>::CapabilityRequirement,
                                       typename ProtocolType::TransportCategory>::value,
                          "Ws2812x descriptor capability requirement must match Ws2812xProtocol transport category.");

            static SettingsType normalize(SettingsType settings)
            {
                settings.channelOrder = Base::template normalizeChannelOrder<ColorType>(
                    settings.channelOrder,
                    TDefaultChannelOrder::value);
                return settings;
            }

            static SettingsType defaultSettings()
            {
                SettingsType settings = Base::defaultSettings();
                settings.timing = defaultTiming();
                return settings;
            }

            static SettingsType fromConfig(const Ws2812xOptions &config)
            {
                SettingsType settings = defaultSettings();
                settings.channelOrder = config.channelOrder;
                settings.timing = config.timing;
                return settings;
            }

            static constexpr OneWireTiming defaultTiming()
            {
                return (TDefaultTiming != nullptr) ? *TDefaultTiming : timing::Ws2812x;
            }

            template <typename TTransportSettings>
            static void mutateTransportSettings(uint16_t,
                                                const SettingsType &protocolSettings,
                                                TTransportSettings &transportSettings)
            {
                lw::normalizeOneWireTransportClockDataBitRate(protocolSettings.timing, transportSettings);
            }
        };

        template <typename TProtocolDesc>
        struct ProtocolDescriptorTraits<TProtocolDesc,
                                        std::void_t<typename TProtocolDesc::InterfaceColorType,
                                                    typename TProtocolDesc::StripColorType,
                                                    typename TProtocolDesc::DefaultChannelOrder,
                                                    decltype(TProtocolDesc::DefaultTiming),
                                                    decltype(TProtocolDesc::IdleHigh)>>
            : ProtocolDescriptorTraits<descriptors::Ws2812x<typename TProtocolDesc::InterfaceColorType,
                                                            typename TProtocolDesc::DefaultChannelOrder,
                                                            TProtocolDesc::DefaultTiming,
                                                            typename TProtocolDesc::StripColorType,
                                                            static_cast<bool>(TProtocolDesc::IdleHigh)>,
                                      void>
        {
        };

    } // namespace factory
} // namespace lw
