#pragma once

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/PrintTransport.h"

namespace npb
{
namespace factory
{

    template <>
    struct TransportDescriptorTraits<descriptors::PrintTransport, void>
    {
        using TransportType = npb::PrintTransport;
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
