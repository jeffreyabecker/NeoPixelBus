#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

#include "buses/PixelBus.h"
#include "colors/NilShader.h"
#include "core/Compat.h"
#include "core/TypeConstraints.h"
#include "protocols/ProtocolAliases.h"

namespace lw::busses
{

    namespace detail
    {

        template <typename TProtocolSettings, typename = void>
        struct PixelBusProtocolSettingsHasTiming : std::false_type
        {
        };

        template <typename TProtocolSettings>
        struct PixelBusProtocolSettingsHasTiming<TProtocolSettings,
                                                 std::void_t<decltype(std::declval<TProtocolSettings &>().timing)>>
            : std::true_type
        {
        };

        template <typename TProtocolSettings>
        void assignPixelBusProtocolTimingIfPresent(TProtocolSettings &settings,
                                                   OneWireTiming timing)
        {
            if constexpr (PixelBusProtocolSettingsHasTiming<TProtocolSettings>::value)
            {
                settings.timing = timing;
            }
        }

        template <typename TProtocol,
                  typename TTransport,
                  typename TProtocolConfig,
                  typename TTransportConfig,
                  typename = void>
        struct DirectMakeBusCompatible : std::false_type
        {
        };

        template <typename TProtocol,
                  typename TTransport,
                  typename TProtocolConfig,
                  typename TTransportConfig>
        struct DirectMakeBusCompatible<TProtocol,
                                       TTransport,
                                       TProtocolConfig,
                                       TTransportConfig,
                                       std::void_t<typename TProtocol::ColorType,
                                                   typename TProtocol::SettingsType,
                                                   typename TTransport::TransportSettingsType>>
            : std::integral_constant<bool,
                                     std::is_convertible<TProtocol *, IProtocol<typename TProtocol::ColorType> *>::value &&
                                         SettingsConstructibleTransportLike<TTransport> &&
                                         ProtocolSettingsConstructibleWithTransport<TProtocol, TTransport> &&
                                         std::is_convertible<lw::remove_cvref_t<TProtocolConfig>, typename TProtocol::SettingsType>::value &&
                                         std::is_convertible<lw::remove_cvref_t<TTransportConfig>, typename TTransport::TransportSettingsType>::value>
        {
        };

        template <typename TShader,
                  typename TColor,
                  typename = void>
        struct DirectMakeBusShaderCompatible : std::false_type
        {
        };

        template <typename TShader,
                  typename TColor>
        struct DirectMakeBusShaderCompatible<TShader,
                                             TColor,
                                             std::void_t<decltype(static_cast<IShader<TColor> *>(std::declval<TShader *>()))>>
            : std::true_type
        {
        };

        template <typename TProtocolAlias,
                  typename = void>
        struct IsWs2812xProtocolAlias : std::false_type
        {
        };

        template <typename TProtocolAlias>
        struct IsWs2812xProtocolAlias<TProtocolAlias,
                                      std::void_t<typename TProtocolAlias::ProtocolType,
                                                  typename TProtocolAlias::SettingsType,
                                                  decltype(TProtocolAlias::defaultSettings()),
                                                  decltype(TProtocolAlias::normalizeSettings(std::declval<typename TProtocolAlias::SettingsType>()))>>
            : std::true_type
        {
        };

    } // namespace detail

    template <typename TProtocol,
              typename TTransport = PlatformDefaultStaticBusDriverTransport,
              typename TProtocolConfig,
              typename TTransportConfig,
              typename = std::enable_if_t<detail::DirectMakeBusCompatible<TProtocol,
                                                                          TTransport,
                                                                          TProtocolConfig,
                                                                          TTransportConfig>::value>>
    PixelBus<TProtocol,
             TTransport,
             NilShader<typename TProtocol::ColorType>> makePixelBus(PixelCount pixelCount,
                                                                    TProtocolConfig &&protocolConfig,
                                                                    TTransportConfig &&transportConfig)
    {
        using ProtocolSettingsType = typename TProtocol::SettingsType;
        using TransportSettingsType = typename TTransport::TransportSettingsType;

        return PixelBus<TProtocol,
                        TTransport,
                        NilShader<typename TProtocol::ColorType>>{pixelCount,
                                                                  static_cast<ProtocolSettingsType>(std::forward<TProtocolConfig>(protocolConfig)),
                                                                  static_cast<TransportSettingsType>(std::forward<TTransportConfig>(transportConfig))};
    }

    template <typename TProtocol,
              typename TTransport = PlatformDefaultStaticBusDriverTransport,
              typename TTransportConfig,
              typename = std::enable_if_t<detail::DirectMakeBusCompatible<TProtocol,
                                                                          TTransport,
                                                                          typename TProtocol::SettingsType,
                                                                          TTransportConfig>::value &&
                                          std::is_default_constructible<typename TProtocol::SettingsType>::value>>
    PixelBus<TProtocol,
             TTransport,
             NilShader<typename TProtocol::ColorType>> makePixelBus(PixelCount pixelCount,
                                                                    TTransportConfig &&transportConfig)
    {
        return makePixelBus<TProtocol, TTransport>(pixelCount,
                                                   typename TProtocol::SettingsType{},
                                                   std::forward<TTransportConfig>(transportConfig));
    }

    template <typename TProtocol,
              typename TTransport = PlatformDefaultStaticBusDriverTransport,
              typename TProtocolConfig,
              typename TTransportConfig,
              typename = std::enable_if_t<detail::DirectMakeBusCompatible<TProtocol,
                                                                          TTransport,
                                                                          TProtocolConfig,
                                                                          TTransportConfig>::value>>
    PixelBus<TProtocol,
             TTransport,
             NilShader<typename TProtocol::ColorType>> makePixelBus(PixelCount pixelCount,
                                                                    TProtocolConfig &&protocolConfig,
                                                                    OneWireTiming timing,
                                                                    TTransportConfig &&transportConfig)
    {
        using ProtocolSettingsType = typename TProtocol::SettingsType;

        ProtocolSettingsType protocolSettings = static_cast<ProtocolSettingsType>(std::forward<TProtocolConfig>(protocolConfig));
        detail::assignPixelBusProtocolTimingIfPresent(protocolSettings, timing);

        return makePixelBus<TProtocol, TTransport>(pixelCount,
                                                   std::move(protocolSettings),
                                                   std::forward<TTransportConfig>(transportConfig));
    }

    template <typename TProtocol,
              typename TTransport = PlatformDefaultStaticBusDriverTransport,
              typename TTransportConfig,
              typename = std::enable_if_t<detail::DirectMakeBusCompatible<TProtocol,
                                                                          TTransport,
                                                                          typename TProtocol::SettingsType,
                                                                          TTransportConfig>::value &&
                                          std::is_default_constructible<typename TProtocol::SettingsType>::value>>
    PixelBus<TProtocol,
             TTransport,
             NilShader<typename TProtocol::ColorType>> makePixelBus(PixelCount pixelCount,
                                                                    OneWireTiming timing,
                                                                    TTransportConfig &&transportConfig)
    {
        auto protocolSettings = typename TProtocol::SettingsType{};
        detail::assignPixelBusProtocolTimingIfPresent(protocolSettings, timing);

        return makePixelBus<TProtocol, TTransport>(pixelCount,
                                                   std::move(protocolSettings),
                                                   std::forward<TTransportConfig>(transportConfig));
    }

    template <typename TProtocol,
              typename TTransport = PlatformDefaultStaticBusDriverTransport,
              typename TShader,
              typename TProtocolConfig,
              typename TTransportConfig,
              typename = std::enable_if_t<detail::DirectMakeBusCompatible<TProtocol,
                                                                          TTransport,
                                                                          TProtocolConfig,
                                                                          TTransportConfig>::value &&
                                          detail::DirectMakeBusShaderCompatible<lw::remove_cvref_t<TShader>, typename TProtocol::ColorType>::value>>
    PixelBus<TProtocol,
             TTransport,
             lw::remove_cvref_t<TShader>> makePixelBus(PixelCount pixelCount,
                                                       TProtocolConfig &&protocolConfig,
                                                       TTransportConfig &&transportConfig,
                                                       TShader &&shader)
    {
        using ProtocolSettingsType = typename TProtocol::SettingsType;
        using TransportSettingsType = typename TTransport::TransportSettingsType;

        return PixelBus<TProtocol,
                        TTransport,
                        lw::remove_cvref_t<TShader>>{pixelCount,
                                                     static_cast<ProtocolSettingsType>(std::forward<TProtocolConfig>(protocolConfig)),
                                                     static_cast<TransportSettingsType>(std::forward<TTransportConfig>(transportConfig)),
                                                     std::forward<TShader>(shader)};
    }

    template <typename TWsAlias,
              typename TTransport = PlatformDefaultStaticBusDriverTransport,
              typename TTransportConfig,
              typename = std::enable_if_t<detail::IsWs2812xProtocolAlias<TWsAlias>::value &&
                                          SettingsConstructibleTransportLike<TTransport> &&
                                          std::is_convertible<lw::remove_cvref_t<TTransportConfig>, typename TTransport::TransportSettingsType>::value>>
    PixelBus<typename TWsAlias::ProtocolType,
             TTransport,
             NilShader<typename TWsAlias::ColorType>> makePixelBus(PixelCount pixelCount,
                                                                   TTransportConfig &&transportConfig)
    {
        using ProtocolType = typename TWsAlias::ProtocolType;
        using SettingsType = typename TWsAlias::SettingsType;
        using TransportSettingsType = typename TTransport::TransportSettingsType;

        SettingsType protocolSettings = TWsAlias::defaultSettings();
        TransportSettingsType transportSettings = static_cast<TransportSettingsType>(std::forward<TTransportConfig>(transportConfig));

        return makePixelBus<ProtocolType, TTransport>(pixelCount,
                                                      std::move(protocolSettings),
                                                      std::move(transportSettings));
    }

    template <typename TWsAlias,
              typename TTransport = PlatformDefaultStaticBusDriverTransport,
              typename TProtocolSettings,
              typename TTransportConfig,
              typename = std::enable_if_t<detail::IsWs2812xProtocolAlias<TWsAlias>::value &&
                                          SettingsConstructibleTransportLike<TTransport> &&
                                          std::is_convertible<lw::remove_cvref_t<TProtocolSettings>, typename TWsAlias::SettingsType>::value &&
                                          std::is_convertible<lw::remove_cvref_t<TTransportConfig>, typename TTransport::TransportSettingsType>::value>>
    PixelBus<typename TWsAlias::ProtocolType,
             TTransport,
             NilShader<typename TWsAlias::ColorType>> makePixelBus(PixelCount pixelCount,
                                                                   TProtocolSettings &&protocolSettings,
                                                                   TTransportConfig &&transportConfig)
    {
        using ProtocolType = typename TWsAlias::ProtocolType;
        using SettingsType = typename TWsAlias::SettingsType;
        using TransportSettingsType = typename TTransport::TransportSettingsType;

        SettingsType resolvedProtocolSettings = TWsAlias::normalizeSettings(static_cast<SettingsType>(std::forward<TProtocolSettings>(protocolSettings)));
        TransportSettingsType transportSettings = static_cast<TransportSettingsType>(std::forward<TTransportConfig>(transportConfig));

        return makePixelBus<ProtocolType, TTransport>(pixelCount,
                                                      std::move(resolvedProtocolSettings),
                                                      std::move(transportSettings));
    }

    template <typename TWsAlias,
              typename TTransport = PlatformDefaultStaticBusDriverTransport,
              typename TTransportConfig,
              typename = std::enable_if_t<detail::IsWs2812xProtocolAlias<TWsAlias>::value &&
                                          SettingsConstructibleTransportLike<TTransport> &&
                                          std::is_convertible<lw::remove_cvref_t<TTransportConfig>, typename TTransport::TransportSettingsType>::value>>
    PixelBus<typename TWsAlias::ProtocolType,
             TTransport,
             NilShader<typename TWsAlias::ColorType>> makePixelBus(PixelCount pixelCount,
                                                                   OneWireTiming timing,
                                                                   TTransportConfig &&transportConfig)
    {
        auto protocolSettings = TWsAlias::defaultSettings();
        protocolSettings.timing = timing;

        return makePixelBus<TWsAlias, TTransport>(pixelCount,
                                                  std::move(protocolSettings),
                                                  std::forward<TTransportConfig>(transportConfig));
    }

    template <typename TWsAlias,
              typename TTransport = PlatformDefaultStaticBusDriverTransport,
              typename TProtocolSettings,
              typename TTransportConfig,
              typename = std::enable_if_t<detail::IsWs2812xProtocolAlias<TWsAlias>::value &&
                                          SettingsConstructibleTransportLike<TTransport> &&
                                          std::is_convertible<lw::remove_cvref_t<TProtocolSettings>, typename TWsAlias::SettingsType>::value &&
                                          std::is_convertible<lw::remove_cvref_t<TTransportConfig>, typename TTransport::TransportSettingsType>::value>>
    PixelBus<typename TWsAlias::ProtocolType,
             TTransport,
             NilShader<typename TWsAlias::ColorType>> makePixelBus(PixelCount pixelCount,
                                                                   TProtocolSettings &&protocolSettings,
                                                                   OneWireTiming timing,
                                                                   TTransportConfig &&transportConfig)
    {
        using SettingsType = typename TWsAlias::SettingsType;
        auto wsProtocolSettings = static_cast<SettingsType>(std::forward<TProtocolSettings>(protocolSettings));
        wsProtocolSettings.timing = timing;

        return makePixelBus<TWsAlias, TTransport>(pixelCount,
                                                  std::move(wsProtocolSettings),
                                                  std::forward<TTransportConfig>(transportConfig));
    }

} // namespace lw::busses
