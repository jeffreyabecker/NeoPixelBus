#pragma once

#ifdef ARDUINO_ARCH_ESP32

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/esp32/Esp32RmtOneWireTransport.h"

namespace npb
{
namespace factory
{

    struct Esp32RmtOneWireOptions
    {
        rmt_channel_t channel = RMT_CHANNEL_0;
        OneWireTiming timing = timing::Ws2812x;
        uint8_t pin = 0;
        bool invert = false;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::Esp32RmtOneWire, void>
    {
        using TransportType = npb::Esp32RmtOneWireTransport;
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

        static SettingsType fromConfig(const Esp32RmtOneWireOptions &config)
        {
            SettingsType settings{};
            settings.channel = config.channel;
            settings.timing = config.timing;
            settings.pin = config.pin;
            settings.invert = config.invert;
            return settings;
        }
    };

} // namespace factory
} // namespace npb

#endif
