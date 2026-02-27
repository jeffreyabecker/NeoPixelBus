#pragma once

#include "transports/NilTransport.h"
#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"

namespace npb
{
namespace factory
{

    struct NilOptions
    {
        bool invert = false;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::Nil, void>
    {
        using TransportType = npb::NilTransport;
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

        static SettingsType fromConfig(const NilOptions &config)
        {
            SettingsType settings{};
            settings.invert = config.invert;
            return settings;
        }
    };

} // namespace factory
} // namespace npb
