#pragma once

#include <type_traits>
#include <utility>

#include "core/Compat.h"

#include "ProtocolConfigs.h"
#include "TransportConfigs.h"

namespace npb::factory
{

    template <typename TConfig>
    struct ProtocolConfigTraits;

    template <typename TProtocol>
    struct ProtocolConfigTraits<ProtocolConfig<TProtocol>>
    {
        using ProtocolType = TProtocol;

        static typename TProtocol::SettingsType toSettings(ProtocolConfig<TProtocol> config)
        {
            return std::move(config.settings);
        }
    };

    template <typename TColor>
    struct ProtocolConfigTraits<Ws2812x<TColor>>
    {
        using ProtocolType = Ws2812xProtocol<TColor>;

        static typename ProtocolType::SettingsType toSettings(const Ws2812x<TColor> &config)
        {
            typename ProtocolType::SettingsType settings{};
            settings.channelOrder = config.colorOrder;
            return settings;
        }
    };

    template <typename TConfig>
    struct TransportConfigTraits;

    template <typename TTransport>
    struct TransportConfigTraits<TransportConfig<TTransport>>
    {
        using TransportType = TTransport;

        static typename TTransport::TransportSettingsType toSettings(TransportConfig<TTransport> config)
        {
            return std::move(config.settings);
        }
    };

    template <>
    struct TransportConfigTraits<Debug>
    {
        using TransportType = DebugOneWireTransport;

        static typename TransportType::TransportSettingsType toSettings(Debug &&config)
        {
            typename TransportType::TransportSettingsType settings{};
            settings.output = std::move(config.output);
            settings.invert = config.invert;
            return settings;
        }
    };

    template <typename TProtocolConfig, typename = void>
    struct FactoryProtocolConfigImpl : std::false_type
    {
    };

    template <typename TProtocolConfig>
    struct FactoryProtocolConfigImpl<TProtocolConfig,
                                     std::void_t<typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType,
                                                 decltype(ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::toSettings(
                                                     std::declval<remove_cvref_t<TProtocolConfig> &&>()))>> : std::true_type
    {
    };

    template <typename TProtocolConfig>
    static constexpr bool FactoryProtocolConfig = FactoryProtocolConfigImpl<TProtocolConfig>::value;

    template <typename TTransportConfig, typename = void>
    struct FactoryTransportConfigImpl : std::false_type
    {
    };

    template <typename TTransportConfig>
    struct FactoryTransportConfigImpl<TTransportConfig,
                                      std::void_t<typename TransportConfigTraits<remove_cvref_t<TTransportConfig>>::TransportType,
                                                  decltype(TransportConfigTraits<remove_cvref_t<TTransportConfig>>::toSettings(
                                                      std::declval<remove_cvref_t<TTransportConfig> &&>()))>> : std::true_type
    {
    };

    template <typename TTransportConfig>
    static constexpr bool FactoryTransportConfig = FactoryTransportConfigImpl<TTransportConfig>::value;

    template <typename TShaderFactory, typename TColor, typename = void>
    struct FactoryShaderForColorImpl : std::false_type
    {
    };

    template <typename TShaderFactory, typename TColor>
    struct FactoryShaderForColorImpl<TShaderFactory,
                                     TColor,
                                     std::void_t<decltype(std::declval<const remove_cvref_t<TShaderFactory> &>().template make<TColor>())>> : std::true_type
    {
    };

    template <typename TShaderFactory, typename TColor>
    static constexpr bool FactoryShaderForColor = FactoryShaderForColorImpl<TShaderFactory, TColor>::value;

} // namespace npb::factory

