#pragma once

#ifdef ARDUINO_ARCH_ESP32

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/esp32/Esp32I2sTransport.h"

namespace npb
{
namespace factory
{

    struct Esp32I2sOptions
    {
        uint8_t pin = 0;
        bool invert = false;
        uint8_t busNumber = 0;
        int8_t clockPin = -1;
        uint32_t clockRateHz = 0;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::Esp32I2s, void>
    {
        using TransportType = npb::Esp32I2sTransport;
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

        static SettingsType fromConfig(const Esp32I2sOptions &config)
        {
            SettingsType settings{};
            settings.pin = config.pin;
            settings.invert = config.invert;
            settings.busNumber = config.busNumber;
            settings.clockPin = config.clockPin;
            settings.clockRateHz = config.clockRateHz;
            return settings;
        }
    };

} // namespace factory
} // namespace npb

#endif
