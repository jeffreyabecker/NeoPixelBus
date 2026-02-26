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

        template <typename TSettings, typename... TTransports>
        struct FindTransportFromSettings
        {
            using Type = void;
        };

        template <typename TSettings, typename TFirstTransport, typename... TRestTransports>
        struct FindTransportFromSettings<TSettings, TFirstTransport, TRestTransports...>
        {
            using Type = std::conditional_t<
                std::is_same<remove_cvref_t<TSettings>, typename TFirstTransport::TransportSettingsType>::value,
                TFirstTransport,
                typename FindTransportFromSettings<TSettings, TRestTransports...>::Type>;
        };

        template <typename TSettings>
        struct TransportFromSettings
        {
            using Type = typename FindTransportFromSettings<
                TSettings,
                NilTransport,
                PrintTransport,
                DebugTransport,
                DebugOneWireTransport
#ifdef ARDUINO_ARCH_RP2040
                ,
                RpPioOneWireTransport,
                RpPioSpiTransport
#endif
#ifdef ARDUINO_ARCH_ESP32
                ,
                Esp32RmtOneWireTransport,
                Esp32I2sTransport,
                Esp32DmaSpiTransport
#endif
#ifdef ARDUINO_ARCH_ESP8266
                ,
                Esp8266UartOneWireTransport,
                Esp8266DmaI2sTransport,
                Esp8266DmaUartTransport
#endif
#if defined(ARDUINO_ARCH_NRF52840)
                ,
                Nrf52PwmOneWireTransport
#endif
                >::Type;
        };

        template <typename TSettings>
        static constexpr bool HasTransportFromSettings =
            !std::is_same<typename TransportFromSettings<TSettings>::Type, void>::value;

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
            using Type = StaticBusDriverPixelBusT<
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
            using ShaderType = ShaderTypeForColor<TShaderFactory, ColorType>;

            using Type = StaticBusDriverPixelBusT<
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

        return makeStaticDriverPixelBus<TransportType, ProtocolType>(
            pixelCount,
            TransportTraits::toSettings(std::move(transportConfig)),
            ProtocolTraits::toSettings(std::move(protocolConfig)));
    }

    template <typename TProtocolConfig,
              typename TTransportConfig,
              typename TTransportConfigDecay = remove_cvref_t<TTransportConfig>,
              typename = std::enable_if_t<FactoryProtocolConfig<TProtocolConfig> && FactoryTransportConfig<TTransportConfigDecay>>>
    Bus<TProtocolConfig, TTransportConfigDecay> makeBus(uint16_t pixelCount,
                                                        TTransportConfig transportConfig)
    {
        return makeBus(pixelCount,
                       TProtocolConfig{},
                       std::move(transportConfig));
    }

    template <typename TProtocolConfig,
              typename TTransportSettings,
              typename TTransportSettingsDecay = remove_cvref_t<TTransportSettings>,
              typename TBaseTransport = typename detail::TransportFromSettings<TTransportSettingsDecay>::Type,
              typename = std::enable_if_t<FactoryProtocolConfig<TProtocolConfig> &&
                                          detail::HasTransportFromSettings<TTransportSettingsDecay> &&
                                          TaggedTransportLike<TBaseTransport, TransportTag> &&
                                          SettingsConstructibleTransportLike<TBaseTransport>>>
    Bus<TProtocolConfig, OneWire<TBaseTransport>> makeBus(uint16_t pixelCount,
                                                          TProtocolConfig protocolConfig,
                                                          OneWireTiming oneWireTiming,
                                                          TTransportSettings transportConfig)
    {
        using BaseSettingsType = typename TBaseTransport::TransportSettingsType;

        OneWire<TBaseTransport> oneWireTransportConfig{};
        BaseSettingsType baseSettings = static_cast<BaseSettingsType>(std::move(transportConfig));
        static_cast<BaseSettingsType &>(oneWireTransportConfig.settings) = std::move(baseSettings);
        oneWireTransportConfig.settings.timing = oneWireTiming;

        return makeBus(pixelCount,
                       std::move(protocolConfig),
                       std::move(oneWireTransportConfig));
    }

    template <typename TProtocolConfig,
              typename TTransportSettings,
              typename TShaderFactory,
              typename TTransportSettingsDecay = remove_cvref_t<TTransportSettings>,
              typename TBaseTransport = typename detail::TransportFromSettings<TTransportSettingsDecay>::Type,
              std::enable_if_t<FactoryProtocolConfig<TProtocolConfig> &&
                                   detail::HasTransportFromSettings<TTransportSettingsDecay> &&
                                   TaggedTransportLike<TBaseTransport, TransportTag> &&
                                   SettingsConstructibleTransportLike<TBaseTransport> &&
                                   FactoryShaderForColor<TShaderFactory,
                                                         typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType::ColorType> &&
                                   !ShaderInstanceForColor<TShaderFactory,
                                                           typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType::ColorType>,
                               int> = 0>
    Bus<TProtocolConfig, OneWire<TBaseTransport>, TShaderFactory> makeBus(uint16_t pixelCount,
                                                                           TProtocolConfig protocolConfig,
                                                                           OneWireTiming oneWireTiming,
                                                                           TTransportSettings transportConfig,
                                                                           TShaderFactory shaderFactory)
    {
        using BaseSettingsType = typename TBaseTransport::TransportSettingsType;

        OneWire<TBaseTransport> oneWireTransportConfig{};
        BaseSettingsType baseSettings = static_cast<BaseSettingsType>(std::move(transportConfig));
        static_cast<BaseSettingsType &>(oneWireTransportConfig.settings) = std::move(baseSettings);
        oneWireTransportConfig.settings.timing = oneWireTiming;

        return makeBus(pixelCount,
                       std::move(protocolConfig),
                       std::move(oneWireTransportConfig),
                       std::move(shaderFactory));
    }

    template <typename TProtocolConfig,
              typename TTransportSettings,
              typename TShader,
              typename TTransportSettingsDecay = remove_cvref_t<TTransportSettings>,
              typename TBaseTransport = typename detail::TransportFromSettings<TTransportSettingsDecay>::Type,
              std::enable_if_t<FactoryProtocolConfig<TProtocolConfig> &&
                                   detail::HasTransportFromSettings<TTransportSettingsDecay> &&
                                   TaggedTransportLike<TBaseTransport, TransportTag> &&
                                   SettingsConstructibleTransportLike<TBaseTransport> &&
                                   ShaderInstanceForColor<TShader,
                                                          typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType::ColorType> &&
                                   !FactoryShaderForColor<TShader,
                                                          typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType::ColorType>,
                               long> = 0>
    Bus<TProtocolConfig, OneWire<TBaseTransport>, TShader> makeBus(uint16_t pixelCount,
                                                                    TProtocolConfig protocolConfig,
                                                                    OneWireTiming oneWireTiming,
                                                                    TTransportSettings transportConfig,
                                                                    TShader shader)
    {
        using BaseSettingsType = typename TBaseTransport::TransportSettingsType;

        OneWire<TBaseTransport> oneWireTransportConfig{};
        BaseSettingsType baseSettings = static_cast<BaseSettingsType>(std::move(transportConfig));
        static_cast<BaseSettingsType &>(oneWireTransportConfig.settings) = std::move(baseSettings);
        oneWireTransportConfig.settings.timing = oneWireTiming;

        return makeBus(pixelCount,
                       std::move(protocolConfig),
                       std::move(oneWireTransportConfig),
                       std::move(shader));
    }

    template <typename TProtocolConfig,
              typename TTransportSettings,
              typename TTransportSettingsDecay = remove_cvref_t<TTransportSettings>,
              typename TBaseTransport = typename detail::TransportFromSettings<TTransportSettingsDecay>::Type,
              typename = std::enable_if_t<FactoryProtocolConfig<TProtocolConfig> &&
                                          detail::HasTransportFromSettings<TTransportSettingsDecay> &&
                                          TaggedTransportLike<TBaseTransport, TransportTag> &&
                                          SettingsConstructibleTransportLike<TBaseTransport>>>
    Bus<TProtocolConfig, OneWire<TBaseTransport>> makeBus(uint16_t pixelCount,
                                                          OneWireTiming oneWireTiming,
                                                          TTransportSettings transportConfig)
    {
        return makeBus(pixelCount,
                       TProtocolConfig{},
                       oneWireTiming,
                       std::move(transportConfig));
    }

    template <typename TProtocolConfig,
              typename TTransportSettings,
              typename TShaderFactory,
              typename TTransportSettingsDecay = remove_cvref_t<TTransportSettings>,
              typename TBaseTransport = typename detail::TransportFromSettings<TTransportSettingsDecay>::Type,
              std::enable_if_t<FactoryProtocolConfig<TProtocolConfig> &&
                                   detail::HasTransportFromSettings<TTransportSettingsDecay> &&
                                   TaggedTransportLike<TBaseTransport, TransportTag> &&
                                   SettingsConstructibleTransportLike<TBaseTransport> &&
                                   FactoryShaderForColor<TShaderFactory,
                                                         typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType::ColorType> &&
                                   !ShaderInstanceForColor<TShaderFactory,
                                                           typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType::ColorType>,
                               int> = 0>
    Bus<TProtocolConfig, OneWire<TBaseTransport>, TShaderFactory> makeBus(uint16_t pixelCount,
                                                                           OneWireTiming oneWireTiming,
                                                                           TTransportSettings transportConfig,
                                                                           TShaderFactory shaderFactory)
    {
        return makeBus(pixelCount,
                       TProtocolConfig{},
                       oneWireTiming,
                       std::move(transportConfig),
                       std::move(shaderFactory));
    }

    template <typename TProtocolConfig,
              typename TTransportSettings,
              typename TShader,
              typename TTransportSettingsDecay = remove_cvref_t<TTransportSettings>,
              typename TBaseTransport = typename detail::TransportFromSettings<TTransportSettingsDecay>::Type,
              std::enable_if_t<FactoryProtocolConfig<TProtocolConfig> &&
                                   detail::HasTransportFromSettings<TTransportSettingsDecay> &&
                                   TaggedTransportLike<TBaseTransport, TransportTag> &&
                                   SettingsConstructibleTransportLike<TBaseTransport> &&
                                   ShaderInstanceForColor<TShader,
                                                          typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType::ColorType> &&
                                   !FactoryShaderForColor<TShader,
                                                          typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType::ColorType>,
                               long> = 0>
    Bus<TProtocolConfig, OneWire<TBaseTransport>, TShader> makeBus(uint16_t pixelCount,
                                                                    OneWireTiming oneWireTiming,
                                                                    TTransportSettings transportConfig,
                                                                    TShader shader)
    {
        return makeBus(pixelCount,
                       TProtocolConfig{},
                       oneWireTiming,
                       std::move(transportConfig),
                       std::move(shader));
    }

    template <typename TProtocolConfig,
              typename TTransportConfig,
              typename TShaderFactory,
              std::enable_if_t<FactoryProtocolConfig<TProtocolConfig> &&
                                   FactoryTransportConfig<TTransportConfig> &&
                                   FactoryShaderForColor<TShaderFactory,
                                                         typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType::ColorType> &&
                                   !ShaderInstanceForColor<TShaderFactory,
                                                           typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType::ColorType>,
                               int> = 0>
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

        return makeStaticDriverPixelBus<typename TransportTraits::TransportType, ProtocolType>(
            pixelCount,
            TransportTraits::toSettings(std::move(transportConfig)),
            std::move(protocolSettings));
    }

    template <typename TProtocolConfig,
              typename TTransportConfig,
              typename TShader,
              std::enable_if_t<FactoryProtocolConfig<TProtocolConfig> &&
                                   FactoryTransportConfig<TTransportConfig> &&
                                   ShaderInstanceForColor<TShader,
                                                          typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType::ColorType> &&
                                   !FactoryShaderForColor<TShader,
                                                          typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType::ColorType>,
                               long> = 0>
    Bus<TProtocolConfig, TTransportConfig, TShader> makeBus(uint16_t pixelCount,
                                                            TProtocolConfig protocolConfig,
                                                            TTransportConfig transportConfig,
                                                            TShader shader)
    {
        using ProtocolTraits = ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>;
        using TransportTraits = TransportConfigTraits<remove_cvref_t<TTransportConfig>>;

        using BaseProtocolType = typename ProtocolTraits::ProtocolType;
        using ColorType = typename BaseProtocolType::ColorType;
        using ShaderType = remove_cvref_t<TShader>;
        using ProtocolType = WithEmbeddedShader<ColorType, ShaderType, BaseProtocolType>;

        using BaseSettingsType = typename BaseProtocolType::SettingsType;
        BaseSettingsType baseSettings = ProtocolTraits::toSettings(std::move(protocolConfig));
        typename ProtocolType::SettingsType protocolSettings{
            std::move(baseSettings),
            std::move(shader)};

        return makeStaticDriverPixelBus<typename TransportTraits::TransportType, ProtocolType>(
            pixelCount,
            TransportTraits::toSettings(std::move(transportConfig)),
            std::move(protocolSettings));
    }

} // namespace npb::factory
