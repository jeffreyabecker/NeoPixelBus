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
        : TransportDescriptorTraitDefaults<typename npb::Esp32I2sTransport::TransportSettingsType>
    {
        using TransportType = npb::Esp32I2sTransport;
        using SettingsType = typename TransportType::TransportSettingsType;
        using Base = TransportDescriptorTraitDefaults<SettingsType>;
        using Base::defaultSettings;
        using Base::fromConfig;

        static SettingsType normalize(SettingsType settings,
                                      uint16_t,
                                      const OneWireTiming *timing = nullptr)
        {
            if (settings.clockRateHz == 0 && timing != nullptr)
            {
                settings.clockRateHz = oneWireEncodedDataRateHz(*timing);
            }
            return settings;
        }

        static SettingsType fromConfig(const Esp32I2sOptions &config,
                                       uint16_t,
                                       const OneWireTiming * = nullptr)
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
