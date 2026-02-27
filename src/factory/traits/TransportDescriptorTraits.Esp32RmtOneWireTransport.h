#pragma once

#ifdef ARDUINO_ARCH_ESP32

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/esp32/Esp32RmtOneWireTransport.h"

namespace npb
{
namespace factory
{

    template <>
    struct TransportDescriptorTraits<descriptors::Esp32RmtOneWireTransport, void>
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
    };

} // namespace factory
} // namespace npb

#endif
