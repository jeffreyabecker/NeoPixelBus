#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

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

    template <typename TProtocolConfig>
    concept FactoryProtocolConfig = requires {
                                      typename ProtocolConfigTraits<std::remove_cvref_t<TProtocolConfig>>::ProtocolType;
                                      {
                                          ProtocolConfigTraits<std::remove_cvref_t<TProtocolConfig>>::toSettings(
                                              std::declval<std::remove_cvref_t<TProtocolConfig> &&>())
                                      };
                                  };

    template <typename TTransportConfig>
    concept FactoryTransportConfig = requires {
                                       typename TransportConfigTraits<std::remove_cvref_t<TTransportConfig>>::TransportType;
                                       {
                                           TransportConfigTraits<std::remove_cvref_t<TTransportConfig>>::toSettings(
                                               std::declval<std::remove_cvref_t<TTransportConfig> &&>())
                                       };
                                   };

    template <typename TShaderFactory, typename TColor>
    concept FactoryShaderForColor = requires {
                                      {
                                          std::declval<const std::remove_cvref_t<TShaderFactory> &>().template make<TColor>()
                                      };
                                  };

} // namespace npb::factory

