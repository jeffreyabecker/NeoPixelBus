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
                  typename TStripColor>
        struct ProtocolDescriptorTraits<descriptors::Ws2812x<TInterfaceColor, TDefaultChannelOrder, TDefaultTiming, TStripColor>, void>
            : ProtocolDescriptorTraitDefaults<typename lw::Ws2812xProtocol<TInterfaceColor, TStripColor>::SettingsType>
        {
            using ProtocolType = lw::Ws2812xProtocol<TInterfaceColor, TStripColor>;
            using SettingsType = typename ProtocolType::SettingsType;
            using ColorType = typename ProtocolType::ColorType;
            using Base = ProtocolDescriptorTraitDefaults<SettingsType>;
            using Base::defaultSettings;
            using Base::fromConfig;
            using Base::mutateTransportSettings;

            static_assert(std::is_same<typename descriptors::Ws2812x<TInterfaceColor, TDefaultChannelOrder, TDefaultTiming, TStripColor>::CapabilityRequirement,
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

        template <>
        struct ProtocolDescriptorTraits<descriptors::Ws2812, void>
            : ProtocolDescriptorTraits<descriptors::Ws2812x<>, void>
        {
        };

        template <>
        struct ProtocolDescriptorTraits<descriptors::Ws2811, void>
            : ProtocolDescriptorTraits<descriptors::Ws2812x<lw::Rgb8Color, lw::ChannelOrder::GRB, &timing::Ws2811, lw::Rgb8Color>, void>
        {
        };

        template <>
        struct ProtocolDescriptorTraits<descriptors::Ws2805, void>
            : ProtocolDescriptorTraits<descriptors::Ws2812x<lw::Rgb8Color, lw::ChannelOrder::GRB, &timing::Ws2805, lw::Rgb8Color>, void>
        {
        };

        template <>
        struct ProtocolDescriptorTraits<descriptors::Sk6812, void>
            : ProtocolDescriptorTraits<descriptors::Ws2812x<lw::Rgb8Color, lw::ChannelOrder::GRB, &timing::Sk6812, lw::Rgb8Color>, void>
        {
        };

        template <>
        struct ProtocolDescriptorTraits<descriptors::Tm1814, void>
            : ProtocolDescriptorTraits<descriptors::Ws2812x<lw::Rgb8Color, lw::ChannelOrder::GRB, &timing::Tm1814, lw::Rgb8Color>, void>
        {
        };

        template <>
        struct ProtocolDescriptorTraits<descriptors::Tm1914, void>
            : ProtocolDescriptorTraits<descriptors::Ws2812x<lw::Rgb8Color, lw::ChannelOrder::GRB, &timing::Tm1914, lw::Rgb8Color>, void>
        {
        };

        template <>
        struct ProtocolDescriptorTraits<descriptors::Tm1829, void>
            : ProtocolDescriptorTraits<descriptors::Ws2812x<lw::Rgb8Color, lw::ChannelOrder::GRB, &timing::Tm1829, lw::Rgb8Color>, void>
        {
        };

        template <>
        struct ProtocolDescriptorTraits<descriptors::Apa106, void>
            : ProtocolDescriptorTraits<descriptors::Ws2812x<lw::Rgb8Color, lw::ChannelOrder::GRB, &timing::Apa106, lw::Rgb8Color>, void>
        {
        };

        template <>
        struct ProtocolDescriptorTraits<descriptors::Tx1812, void>
            : ProtocolDescriptorTraits<descriptors::Ws2812x<lw::Rgb8Color, lw::ChannelOrder::GRB, &timing::Tx1812, lw::Rgb8Color>, void>
        {
        };

        template <>
        struct ProtocolDescriptorTraits<descriptors::Gs1903, void>
            : ProtocolDescriptorTraits<descriptors::Ws2812x<lw::Rgb8Color, lw::ChannelOrder::GRB, &timing::Gs1903, lw::Rgb8Color>, void>
        {
        };

        template <>
        struct ProtocolDescriptorTraits<descriptors::Generic800, void>
            : ProtocolDescriptorTraits<descriptors::Ws2812x<lw::Rgb8Color, lw::ChannelOrder::GRB, &timing::Generic800, lw::Rgb8Color>, void>
        {
        };

        template <>
        struct ProtocolDescriptorTraits<descriptors::Generic400, void>
            : ProtocolDescriptorTraits<descriptors::Ws2812x<lw::Rgb8Color, lw::ChannelOrder::GRB, &timing::Generic400, lw::Rgb8Color>, void>
        {
        };

        template <>
        struct ProtocolDescriptorTraits<descriptors::Ws2816, void>
            : ProtocolDescriptorTraits<descriptors::Ws2812x<lw::Rgb8Color, lw::ChannelOrder::GRB, &timing::Ws2816, lw::Rgb8Color>, void>
        {
        };

        template <>
        struct ProtocolDescriptorTraits<descriptors::Ws2813, void>
            : ProtocolDescriptorTraits<descriptors::Ws2812x<lw::Rgb8Color, lw::ChannelOrder::GRB, &timing::Ws2813, lw::Rgb8Color>, void>
        {
        };

        template <>
        struct ProtocolDescriptorTraits<descriptors::Ws2814, void>
            : ProtocolDescriptorTraits<descriptors::Ws2812x<lw::Rgb8Color, lw::ChannelOrder::GRB, &timing::Ws2814, lw::Rgb8Color>, void>
        {
        };

        template <>
        struct ProtocolDescriptorTraits<descriptors::Lc8812, void>
            : ProtocolDescriptorTraits<descriptors::Ws2812x<lw::Rgb8Color, lw::ChannelOrder::GRB, &timing::Lc8812, lw::Rgb8Color>, void>
        {
        };

    } // namespace factory
} // namespace lw
