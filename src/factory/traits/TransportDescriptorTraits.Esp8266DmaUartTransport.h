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
        uint32_t baudRate = 0;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::Esp8266DmaUart, void>
        : TransportDescriptorTraitDefaults<typename npb::Esp8266DmaUartTransport::TransportSettingsType>
    {
        using TransportType = npb::Esp8266DmaUartTransport;
        using SettingsType = typename TransportType::TransportSettingsType;
        using Base = TransportDescriptorTraitDefaults<SettingsType>;
        using Base::defaultSettings;
        using Base::fromConfig;

        static SettingsType normalize(SettingsType settings,
                                      uint16_t,
                                      const OneWireTiming *timing = nullptr)
        {
            if (settings.baudRate == 0 && timing != nullptr)
            {
                settings.baudRate = oneWireEncodedDataRateHz(*timing);
            }
            if (settings.baudRate == 0)
            {
                settings.baudRate = 3200000UL;
            }
            return settings;
        }

        static SettingsType fromConfig(const Esp8266DmaUartOptions &config,
                                       uint16_t,
                                       const OneWireTiming * = nullptr)
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
