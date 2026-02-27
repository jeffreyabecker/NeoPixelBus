#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/rp2040/RpPioTwoWireTransport.h"

namespace npb
{
namespace factory
{

    struct RpPioTwoWireOptions
    {
        bool invert = false;
        uint32_t clockRateHz = NEOPIXELBUS_SPI_CLOCK_DEFAULT_HZ;
        BitOrder bitOrder = MSBFIRST;
        uint8_t dataMode = SPI_MODE0;
        int8_t clockPin = -1;
        int8_t dataPin = -1;
        uint8_t pioIndex = 1;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::RpPioTwoWire, void>
        : TransportDescriptorTraitDefaults<typename npb::RpPioTwoWireTransport::TransportSettingsType>
    {
        using TransportType = npb::RpPioTwoWireTransport;
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

        static SettingsType fromConfig(const RpPioTwoWireOptions &config,
                                       uint16_t,
                                       const OneWireTiming * = nullptr)
        {
            SettingsType settings{};
            settings.invert = config.invert;
            settings.clockRateHz = config.clockRateHz;
            settings.bitOrder = config.bitOrder;
            settings.dataMode = config.dataMode;
            settings.clockPin = config.clockPin;
            settings.dataPin = config.dataPin;
            settings.pioIndex = config.pioIndex;
            return settings;
        }
    };

} // namespace factory
} // namespace npb

#endif
