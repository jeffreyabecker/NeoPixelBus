#pragma once

#ifdef ARDUINO_ARCH_ESP8266

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/esp8266/Esp8266DmaI2sTransport.h"

namespace npb
{
namespace factory
{

    struct Esp8266DmaI2sOptions
    {
        bool invert = false;
        uint32_t clockRateHz = 0;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::Esp8266DmaI2s, void>
    {
        using TransportType = npb::Esp8266DmaI2sTransport;
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

        static SettingsType fromConfig(const Esp8266DmaI2sOptions &config)
        {
            SettingsType settings{};
            settings.invert = config.invert;
            settings.clockRateHz = config.clockRateHz;
            return settings;
        }
    };

} // namespace factory
} // namespace npb

#endif
