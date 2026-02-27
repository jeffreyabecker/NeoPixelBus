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
        : TransportDescriptorTraitDefaults<typename npb::RpPioOneWireTransport::TransportSettingsType>
    {
        using TransportType = npb::RpPioOneWireTransport;
        using SettingsType = typename TransportType::TransportSettingsType;
        using Base = TransportDescriptorTraitDefaults<SettingsType>;
        using Base::defaultSettings;
        using Base::fromConfig;

        static SettingsType normalize(SettingsType settings,
                                      uint16_t,
                                      const OneWireTiming *timing = nullptr)
        {
            if (timing != nullptr)
            {
                settings.timing = *timing;
            }
            return settings;
        }

        static SettingsType fromConfig(const RpPioOneWireOptions &config,
                                       uint16_t,
                                       const OneWireTiming * = nullptr)
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
