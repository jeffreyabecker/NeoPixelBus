#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/rp2040/RpUartTransport.h"

namespace npb
{
namespace factory
{

    struct RpUartOptions
    {
        bool invert = false;
        uint32_t baudRate = UartBaudDefault;
        uint8_t uartIndex = 0;
        int8_t txPin = -1;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::RpUart, void>
        : TransportDescriptorTraitDefaults<typename npb::RpUartTransport::TransportSettingsType>
    {
        using TransportType = npb::RpUartTransport;
        using SettingsType = typename TransportType::TransportSettingsType;
        using Base = TransportDescriptorTraitDefaults<SettingsType>;
        using Base::defaultSettings;
        using Base::fromConfig;

        static SettingsType normalize(SettingsType settings,
                                      uint16_t,
                                      const OneWireTiming * = nullptr)
        {
            return settings;
        }

        static SettingsType fromConfig(const RpUartOptions &config,
                                       uint16_t,
                                       const OneWireTiming * = nullptr)
        {
            SettingsType settings{};
            settings.invert = config.invert;
            settings.baudRate = config.baudRate;
            settings.uartIndex = config.uartIndex;
            settings.txPin = config.txPin;
            return settings;
        }
    };

} // namespace factory
} // namespace npb

#endif
