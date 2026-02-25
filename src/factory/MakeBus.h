#pragma once

#include <cstdint>
#include <concepts>
#include <type_traits>
#include <utility>

#include "ProtocolConfigs.h"
#include "ShaderFactories.h"
#include "Traits.h"

#include "buses/BusDriver.h"
#include "protocols/WithShaderProtocol.h"

namespace npb::factory
{

    namespace detail
    {

        template <typename TProtocolConfig,
                  typename TTransportConfig,
                  typename TShaderFactory = void,
                  bool HasShader = !std::same_as<std::remove_cvref_t<TShaderFactory>, void>>
        struct BusSelector;

        template <typename TProtocolConfig,
                  typename TTransportConfig,
                  typename TShaderFactory>
            requires FactoryProtocolConfig<TProtocolConfig> && FactoryTransportConfig<TTransportConfig>
        struct BusSelector<TProtocolConfig, TTransportConfig, TShaderFactory, false>
        {
            using Type = OwningBusDriverPixelBusT<
                typename TransportConfigTraits<std::remove_cvref_t<TTransportConfig>>::TransportType,
                typename ProtocolConfigTraits<std::remove_cvref_t<TProtocolConfig>>::ProtocolType>;
        };

        template <typename TProtocolConfig,
                  typename TTransportConfig,
                  typename TShaderFactory>
            requires FactoryProtocolConfig<TProtocolConfig> &&
                     FactoryTransportConfig<TTransportConfig> &&
                     FactoryShaderForColor<TShaderFactory,
                                           typename ProtocolConfigTraits<std::remove_cvref_t<TProtocolConfig>>::ProtocolType::ColorType>
        struct BusSelector<TProtocolConfig, TTransportConfig, TShaderFactory, true>
        {
            using ProtocolType = typename ProtocolConfigTraits<std::remove_cvref_t<TProtocolConfig>>::ProtocolType;
            using ColorType = typename ProtocolType::ColorType;
            using ShaderType = decltype(std::declval<const std::remove_cvref_t<TShaderFactory> &>().template make<ColorType>());

            using Type = OwningBusDriverPixelBusT<
                typename TransportConfigTraits<std::remove_cvref_t<TTransportConfig>>::TransportType,
                WithEmbeddedShader<ColorType, ShaderType, ProtocolType>>;
        };

    } // namespace detail

    template <typename TProtocolConfig,
              typename TTransportConfig,
              typename TShaderFactory = void>
        requires FactoryProtocolConfig<TProtocolConfig> && FactoryTransportConfig<TTransportConfig>
    using Bus = typename detail::BusSelector<TProtocolConfig,
                                             TTransportConfig,
                                             TShaderFactory>::Type;

    template <typename TProtocolConfig, typename TTransportConfig>
        requires FactoryProtocolConfig<TProtocolConfig> && FactoryTransportConfig<TTransportConfig>
    Bus<TProtocolConfig, TTransportConfig> makeBus(uint16_t pixelCount,
                                                   TProtocolConfig protocolConfig,
                                                   TTransportConfig transportConfig)
    {
        using ProtocolTraits = ProtocolConfigTraits<std::remove_cvref_t<TProtocolConfig>>;
        using TransportTraits = TransportConfigTraits<std::remove_cvref_t<TTransportConfig>>;

        using ProtocolType = typename ProtocolTraits::ProtocolType;
        using TransportType = typename TransportTraits::TransportType;

        return makeOwningDriverPixelBus<TransportType, ProtocolType>(
            pixelCount,
            TransportTraits::toSettings(std::move(transportConfig)),
            ProtocolTraits::toSettings(std::move(protocolConfig)));
    }

    template <typename TProtocolConfig,
              typename TTransportConfig,
              typename TShaderFactory>
        requires FactoryProtocolConfig<TProtocolConfig> &&
                 FactoryTransportConfig<TTransportConfig> &&
                 FactoryShaderForColor<TShaderFactory,
                                       typename ProtocolConfigTraits<std::remove_cvref_t<TProtocolConfig>>::ProtocolType::ColorType>
    Bus<TProtocolConfig, TTransportConfig, TShaderFactory> makeBus(uint16_t pixelCount,
                                                                    TProtocolConfig protocolConfig,
                                                                    TTransportConfig transportConfig,
                                                                    TShaderFactory shaderFactory)
    {
        using ProtocolTraits = ProtocolConfigTraits<std::remove_cvref_t<TProtocolConfig>>;
        using TransportTraits = TransportConfigTraits<std::remove_cvref_t<TTransportConfig>>;

        using BaseProtocolType = typename ProtocolTraits::ProtocolType;
        using ColorType = typename BaseProtocolType::ColorType;
        using ShaderType = decltype(std::declval<const std::remove_cvref_t<TShaderFactory> &>().template make<ColorType>());
        using ProtocolType = WithEmbeddedShader<ColorType, ShaderType, BaseProtocolType>;

        using BaseSettingsType = typename BaseProtocolType::SettingsType;
        BaseSettingsType baseSettings = ProtocolTraits::toSettings(std::move(protocolConfig));
        ShaderType shader = std::move(shaderFactory).template make<ColorType>();
        typename ProtocolType::SettingsType protocolSettings{
            std::move(baseSettings),
            std::move(shader)};

        return makeOwningDriverPixelBus<typename TransportTraits::TransportType, ProtocolType>(
            pixelCount,
            TransportTraits::toSettings(std::move(transportConfig)),
            std::move(protocolSettings));
    }

    template <typename TProtocolConfig, typename TTransportConfig>
        requires FactoryProtocolConfig<TProtocolConfig> && FactoryTransportConfig<TTransportConfig>
    Bus<TProtocolConfig, TTransportConfig> makeBus(uint16_t pixelCount)
    {
        return makeBus(pixelCount,
                       TProtocolConfig{},
                       TTransportConfig{});
    }

} // namespace npb::factory


