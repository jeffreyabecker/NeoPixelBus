#pragma once

#ifdef ARDUINO_ARCH_ESP8266

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/esp8266/Esp8266DmaUartTransport.h"

namespace npb
{
namespace factory
{

    struct Esp8266DmaUartOptions
    {
        uint8_t uartNumber = 1;
        bool invert = false;
        uint32_t baudRate = 3200000UL;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::Esp8266DmaUart, void>
    {
        using TransportType = npb::Esp8266DmaUartTransport;
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

        static SettingsType fromConfig(const Esp8266DmaUartOptions &config)
        {
            SettingsType settings{};
            settings.uartNumber = config.uartNumber;
            settings.invert = config.invert;
            settings.baudRate = config.baudRate;
            return settings;
        }
    };

} // namespace factory
} // namespace npb

#endif
