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
        : TransportDescriptorTraitDefaults<typename npb::Esp8266DmaI2sTransport::TransportSettingsType>
    {
        using TransportType = npb::Esp8266DmaI2sTransport;
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

        static SettingsType fromConfig(const Esp8266DmaI2sOptions &config,
                                       uint16_t,
                                       const OneWireTiming * = nullptr)
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
