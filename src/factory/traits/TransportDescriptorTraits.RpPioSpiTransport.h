#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/rp2040/RpPioSpiTransport.h"

namespace npb
{
namespace factory
{

    struct RpPioSpiOptions
    {
        bool invert = false;
        uint32_t clockRateHz = 0;
        BitOrder bitOrder = MSBFIRST;
        uint8_t dataMode = SPI_MODE0;
        int clockPin = -1;
        int dataPin = -1;
        uint8_t pioIndex = 1;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::RpPioSpi, void>
        : TransportDescriptorTraitDefaults<typename npb::RpPioSpiTransport::TransportSettingsType>
    {
        using TransportType = npb::RpPioSpiTransport;
        using SettingsType = typename TransportType::TransportSettingsType;
        using Base = TransportDescriptorTraitDefaults<SettingsType>;
        using Base::defaultSettings;
        using Base::fromConfig;

        static SettingsType normalize(SettingsType settings,
                                      uint16_t,
                                      const OneWireTiming *timing = nullptr)
        {
            if (settings.clockRateHz == 0 && timing != nullptr)
            {
                settings.clockRateHz = oneWireEncodedDataRateHz(*timing);
            }
            if (settings.clockRateHz == 0)
            {
                settings.clockRateHz = RpPioClockDataDefaultHz;
            }
            return settings;
        }

        static SettingsType fromConfig(const RpPioSpiOptions &config,
                                       uint16_t,
                                       const OneWireTiming * = nullptr)
        {
            SettingsType settings{};
            settings.clockPin = config.clockPin;
            settings.dataPin = config.dataPin;
            settings.pioIndex = config.pioIndex;
            settings.invert = config.invert;
            settings.clockRateHz = config.clockRateHz;
            settings.bitOrder = config.bitOrder;
            settings.dataMode = config.dataMode;
            return settings;
        }

    };

} // namespace factory
} // namespace npb

#endif
