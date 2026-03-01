#pragma once

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/PrintTransport.h"

namespace lw
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
            : TransportDescriptorTraitDefaults<typename lw::PrintTransport::TransportSettingsType>
        {
            using TransportType = lw::PrintTransport;
            using SettingsType = typename TransportType::TransportSettingsType;
            using Base = TransportDescriptorTraitDefaults<SettingsType>;
            using Base::defaultSettings;
            using Base::fromConfig;

            static SettingsType normalize(SettingsType settings,
                                          uint16_t,
                                          const OneWireTiming * = nullptr)
            {
                if(settings.output == nullptr)
                {
                    settings.output = &Serial;
                }
                return settings;    
            }

            static SettingsType fromConfig(const NeoPrintOptions &config,
                                           uint16_t)
            {
                SettingsType settings{};
                settings.output = config.output;
                settings.invert = config.invert;
                return settings;
            }

        };

    } // namespace factory
} // namespace lw
