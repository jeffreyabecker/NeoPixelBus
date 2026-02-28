#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

#include "buses/BusDriver.h"

namespace npb
{

    template <typename TTransport,
              typename TProtocol,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TTransport>>>
    class StaticBusDriverPixelBusT
        : private ProtocolBusDriverT<TProtocol, TTransport>
        , public BusDriverPixelBusT<ProtocolBusDriverT<TProtocol, TTransport>>
    {
    public:
        using ColorType = typename TProtocol::ColorType;
        using DriverType = ProtocolBusDriverT<TProtocol, TTransport>;
        using BusType = BusDriverPixelBusT<DriverType>;
        using ProtocolSettingsType = typename TProtocol::SettingsType;
        using TransportSettingsType = typename TTransport::TransportSettingsType;

        StaticBusDriverPixelBusT(uint16_t pixelCount,
                                 TransportSettingsType transportSettings,
                                 ProtocolSettingsType settings)
            : DriverType(pixelCount, std::move(transportSettings), std::move(settings))
            , BusType(static_cast<DriverType &>(*this))
        {
        }

        TTransport &transport()
        {
            return static_cast<DriverType &>(*this).transport();
        }

        const TTransport &transport() const
        {
            return static_cast<const DriverType &>(*this).transport();
        }

        TProtocol &protocol()
        {
            return static_cast<DriverType &>(*this).protocol();
        }

        const TProtocol &protocol() const
        {
            return static_cast<const DriverType &>(*this).protocol();
        }
    };

    template <typename TTransport,
              typename TProtocol,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TTransport> &&
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport>>>
    StaticBusDriverPixelBusT<TTransport, TProtocol> makeStaticDriverPixelBus(uint16_t pixelCount,
                                                                              typename TTransport::TransportSettingsType transportSettings,
                                                                              typename TProtocol::SettingsType settings)
    {
        return StaticBusDriverPixelBusT<TTransport, TProtocol>(pixelCount,
                                                               std::move(transportSettings),
                                                               std::move(settings));
    }

    template <typename TTransport,
              typename TProtocol,
              typename TBaseSettings,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TTransport> &&
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport> &&
                                          std::is_base_of<typename std::remove_cv<typename std::remove_reference<TBaseSettings>::type>::type,
                                                          typename TProtocol::SettingsType>::value &&
                                          std::is_constructible<typename TProtocol::SettingsType,
                                                                typename TProtocol::SettingsType>::value>>
    StaticBusDriverPixelBusT<TTransport, TProtocol> makeStaticDriverPixelBus(uint16_t pixelCount,
                                                                              typename TTransport::TransportSettingsType transportSettings,
                                                                              typename TProtocol::SettingsType settings,
                                                                              TBaseSettings &&baseSettings)
    {
        using BaseSettingsType = typename std::remove_cv<typename std::remove_reference<TBaseSettings>::type>::type;
        static_cast<BaseSettingsType &>(settings) = std::forward<TBaseSettings>(baseSettings);

        return makeStaticDriverPixelBus<TTransport, TProtocol>(pixelCount,
                                                               std::move(transportSettings),
                                                               std::move(settings));
    }

} // namespace npb
