#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

#include "core/Compat.h"

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
                  bool HasShader = !std::is_same<remove_cvref_t<TShaderFactory>, void>::value>
        struct BusSelector;

        template <typename TProtocolConfig,
                  typename TTransportConfig,
                  typename TShaderFactory>
        struct BusSelector<TProtocolConfig,
                           TTransportConfig,
                           TShaderFactory,
                           false>
        {
            using Type = OwningBusDriverPixelBusT<
                typename TransportConfigTraits<remove_cvref_t<TTransportConfig>>::TransportType,
                typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType>;
        };

        template <typename TProtocolConfig,
                  typename TTransportConfig,
                  typename TShaderFactory>
        struct BusSelector<TProtocolConfig,
                           TTransportConfig,
                           TShaderFactory,
                           true>
        {
            using ProtocolType = typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType;
            using ColorType = typename ProtocolType::ColorType;
            using ShaderType = decltype(std::declval<const remove_cvref_t<TShaderFactory> &>().template make<ColorType>());

            using Type = OwningBusDriverPixelBusT<
                typename TransportConfigTraits<remove_cvref_t<TTransportConfig>>::TransportType,
                WithEmbeddedShader<ColorType, ShaderType, ProtocolType>>;
        };

    } // namespace detail

    template <typename TProtocolConfig,
              typename TTransportConfig,
              typename TShaderFactory = void,
              typename = std::enable_if_t<FactoryProtocolConfig<TProtocolConfig> && FactoryTransportConfig<TTransportConfig>>>
    using Bus = typename detail::BusSelector<TProtocolConfig,
                                             TTransportConfig,
                                             TShaderFactory>::Type;

    template <typename TProtocolConfig,
              typename TTransportConfig,
              typename = std::enable_if_t<FactoryProtocolConfig<TProtocolConfig> && FactoryTransportConfig<TTransportConfig>>>
    Bus<TProtocolConfig, TTransportConfig> makeBus(uint16_t pixelCount,
                                                   TProtocolConfig protocolConfig,
                                                   TTransportConfig transportConfig)
    {
        using ProtocolTraits = ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>;
        using TransportTraits = TransportConfigTraits<remove_cvref_t<TTransportConfig>>;

        using ProtocolType = typename ProtocolTraits::ProtocolType;
        using TransportType = typename TransportTraits::TransportType;

        return makeOwningDriverPixelBus<TransportType, ProtocolType>(
            pixelCount,
            TransportTraits::toSettings(std::move(transportConfig)),
            ProtocolTraits::toSettings(std::move(protocolConfig)));
    }

    template <typename TProtocolConfig,
              typename TTransportConfig,
              typename TShaderFactory,
              typename = std::enable_if_t<FactoryProtocolConfig<TProtocolConfig> &&
                                          FactoryTransportConfig<TTransportConfig> &&
                                          FactoryShaderForColor<TShaderFactory,
                                                                typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType::ColorType>>>
    Bus<TProtocolConfig, TTransportConfig, TShaderFactory> makeBus(uint16_t pixelCount,
                                                                    TProtocolConfig protocolConfig,
                                                                    TTransportConfig transportConfig,
                                                                    TShaderFactory shaderFactory)
    {
        using ProtocolTraits = ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>;
        using TransportTraits = TransportConfigTraits<remove_cvref_t<TTransportConfig>>;

        using BaseProtocolType = typename ProtocolTraits::ProtocolType;
        using ColorType = typename BaseProtocolType::ColorType;
        using ShaderType = decltype(std::declval<const remove_cvref_t<TShaderFactory> &>().template make<ColorType>());
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

    template <typename TProtocolConfig,
              typename TTransportConfig,
              typename = std::enable_if_t<FactoryProtocolConfig<TProtocolConfig> && FactoryTransportConfig<TTransportConfig>>>
    Bus<TProtocolConfig, TTransportConfig> makeBus(uint16_t pixelCount)
    {
        return makeBus(pixelCount,
                       TProtocolConfig{},
                       TTransportConfig{});
    }

} // namespace npb::factory


