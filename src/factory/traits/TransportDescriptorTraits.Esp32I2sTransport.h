#pragma once

#ifdef ARDUINO_ARCH_ESP32

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/esp32/Esp32I2sTransport.h"

namespace lw
{
namespace factory
{

    struct Esp32I2sOptions
    {
        bool invert = false;
        uint32_t clockRateHz = 0;
        uint8_t bitOrder = static_cast<uint8_t>(MSBFIRST);
        uint8_t dataMode = SPI_MODE0;
        int clockPin = -1;
        int dataPin = -1;
        uint8_t busNumber = 0;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::Esp32I2s, void>
        : TransportDescriptorTraitDefaults<typename lw::Esp32I2sTransport::TransportSettingsType>
    {
        using TransportType = lw::Esp32I2sTransport;
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

        static SettingsType fromConfig(const Esp32I2sOptions &config,
                                       uint16_t)
        {
            SettingsType settings{};
            settings.invert = config.invert;
            settings.clockRateHz = config.clockRateHz;
            settings.bitOrder = config.bitOrder;
            settings.dataMode = config.dataMode;
            settings.clockPin = config.clockPin;
            settings.dataPin = config.dataPin;
            settings.busNumber = config.busNumber;
            return settings;
        }
    };

} // namespace factory
} // namespace lw

#endif
