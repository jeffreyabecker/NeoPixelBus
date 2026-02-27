#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/rp2040/RpPioSpiTransport.h"

namespace npb
{
namespace factory
{

    struct RpPioSpiOptions
    {
        uint8_t clockPin = 0;
        uint8_t dataPin = 0;
        uint8_t pioIndex = 1;
        bool invert = false;
        uint32_t clockRateHz = RpPioClockDataDefaultHz;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::RpPioSpi, void>
    {
        using TransportType = npb::RpPioSpiTransport;
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

        static SettingsType fromConfig(const RpPioSpiOptions &config)
        {
            SettingsType settings{};
            settings.clockPin = config.clockPin;
            settings.dataPin = config.dataPin;
            settings.pioIndex = config.pioIndex;
            settings.invert = config.invert;
            settings.clockRateHz = config.clockRateHz;
            return settings;
        }
    };

} // namespace factory
} // namespace npb

#endif
