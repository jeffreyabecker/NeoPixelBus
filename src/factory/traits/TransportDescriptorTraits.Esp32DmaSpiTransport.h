#pragma once

#ifdef ARDUINO_ARCH_ESP32

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/esp32/Esp32DmaSpiTransport.h"

namespace npb
{
namespace factory
{

    struct Esp32DmaSpiOptions
    {
        bool invert = false;
        uint32_t clockRateHz = 0;
        BitOrder bitOrder = MSBFIRST;
        uint8_t dataMode = SPI_MODE0;
        int clockPin = Esp32DmaSpiDefaultSckPin;
        int dataPin = Esp32DmaSpiDefaultDataPin;
        spi_host_device_t spiHost = Esp32DmaSpiDefaultHost;
        int8_t ssPin = -1;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::Esp32DmaSpi, void>
        : TransportDescriptorTraitDefaults<typename npb::Esp32DmaSpiTransport::TransportSettingsType>
    {
        using TransportType = npb::Esp32DmaSpiTransport;
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
                settings.clockRateHz = Esp32DmaSpiClockDefaultHz;
            }
            return settings;
        }

        static SettingsType fromConfig(const Esp32DmaSpiOptions &config,
                                       uint16_t,
                                       const OneWireTiming * = nullptr)
        {
            SettingsType settings{};
            settings.invert = config.invert;
            settings.clockRateHz = config.clockRateHz;
            settings.bitOrder = config.bitOrder;
            settings.dataMode = config.dataMode;
            settings.spiHost = config.spiHost;
            settings.clockPin = config.clockPin;
            settings.dataPin = config.dataPin;
            settings.ssPin = config.ssPin;
            return settings;
        }
    };

} // namespace factory
} // namespace npb

#endif
