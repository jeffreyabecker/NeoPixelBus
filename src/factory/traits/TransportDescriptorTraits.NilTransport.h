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
        : TransportDescriptorTraitDefaults<typename npb::NilTransport::TransportSettingsType>
    {
        using TransportType = npb::NilTransport;
        using SettingsType = typename TransportType::TransportSettingsType;
        using Base = TransportDescriptorTraitDefaults<SettingsType>;
        using Base::defaultSettings;
        using Base::fromConfig;

        static SettingsType normalize(SettingsType settings,
                                      uint16_t,
                                      const OneWireTiming * = nullptr)
        {
            return settings;
        }

        static SettingsType fromConfig(const NilOptions &config,
                                       uint16_t,
                                       const OneWireTiming * = nullptr)
        {
            SettingsType settings{};
            settings.invert = config.invert;
            return settings;
        }

    };

} // namespace factory
} // namespace npb
