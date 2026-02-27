#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/rp2040/RpPioOneWireTransport.h"

namespace npb
{
namespace factory
{

    struct RpPioOneWireOptions
    {
        uint8_t pin = 0;
        uint8_t pioIndex = 1;
        size_t frameBytes = 0;
        bool invert = false;
        OneWireTiming timing = timing::Ws2812x;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::RpPioOneWire, void>
    {
        using TransportType = npb::RpPioOneWireTransport;
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

        static SettingsType fromConfig(const RpPioOneWireOptions &config)
        {
            SettingsType settings{};
            settings.pin = config.pin;
            settings.pioIndex = config.pioIndex;
            settings.frameBytes = config.frameBytes;
            settings.invert = config.invert;
            settings.timing = config.timing;
            return settings;
        }
    };

} // namespace factory
} // namespace npb

#endif
