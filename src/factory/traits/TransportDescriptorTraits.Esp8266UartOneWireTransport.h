#pragma once

#ifdef ARDUINO_ARCH_ESP8266

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/esp8266/Esp8266UartOneWireTransport.h"

namespace npb
{
namespace factory
{

    template <>
    struct TransportDescriptorTraits<descriptors::Esp8266UartOneWireTransport, void>
    {
        using TransportType = npb::Esp8266UartOneWireTransport;
        using SettingsType = typename TransportType::TransportSettingsType;

        static SettingsType defaultSettings()
        {
            return SettingsType{};
        }

        static SettingsType normalize(SettingsType settings)
        {
            return settings;
        }

        static SettingsType fromConfig(SettingsType settings)
        {
            return settings;
        }
    };

} // namespace factory
} // namespace npb

#endif
