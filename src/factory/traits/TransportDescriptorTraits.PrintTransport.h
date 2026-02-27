#pragma once

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/PrintTransport.h"

namespace npb
{
namespace factory
{

    struct NeoPrintOptions
    {
        Print *output = nullptr;
        bool invert = false;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::NeoPrint, void>
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

        static SettingsType fromConfig(const NeoPrintOptions &config)
        {
            SettingsType settings{};
            settings.output = config.output;
            settings.invert = config.invert;
            return settings;
        }
    };

} // namespace factory
} // namespace npb
