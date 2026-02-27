#pragma once

#ifdef ARDUINO_ARCH_ESP32

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/esp32/Esp32DmaSpiTransport.h"

namespace npb
{
namespace factory
{

    struct Esp32DmaSpiOptions
    {
        bool invert = false;
        spi_host_device_t spiHost = Esp32DmaSpiDefaultHost;
        int8_t clockPin = Esp32DmaSpiDefaultSckPin;
        int8_t dataPin = Esp32DmaSpiDefaultDataPin;
        int8_t ssPin = -1;
        uint32_t clockRateHz = Esp32DmaSpiClockDefaultHz;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::Esp32DmaSpi, void>
    {
        using TransportType = npb::Esp32DmaSpiTransport;
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

        static SettingsType fromConfig(const Esp32DmaSpiOptions &config)
        {
            SettingsType settings{};
            settings.invert = config.invert;
            settings.spiHost = config.spiHost;
            settings.clockPin = config.clockPin;
            settings.dataPin = config.dataPin;
            settings.ssPin = config.ssPin;
            settings.clockRateHz = config.clockRateHz;
            return settings;
        }
    };

} // namespace factory
} // namespace npb

#endif
