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
            bool asciiOutput = false;
            bool debugOutput = false;
            const char *identifier = nullptr;

            static NeoPrintOptions debug(const char *id)
            {
                NeoPrintOptions options{};
                options.debugOutput = true;
                options.identifier = id;
                return options;
            }
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
                return SettingsType::normalize(settings);
            }

            static SettingsType fromConfig(const NeoPrintOptions &config,
                                           uint16_t)
            {
                SettingsType settings{};
                settings.output = config.output;
                settings.invert = config.invert;
                settings.asciiOutput = config.asciiOutput;
                settings.debugOutput = config.debugOutput;
                settings.identifier = config.identifier;
                return settings;
            }

        };

    } // namespace factory
} // namespace lw
