#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/rp2040/RpUartTransport.h"

namespace lw
{
namespace factory
{

    struct RpUartOptions
    {
        bool invert = false;
        uint32_t clockRateHz = UartClockDefaultHz;
        uint8_t bitOrder = static_cast<uint8_t>(MSBFIRST);
        uint8_t dataMode = SPI_MODE0;
        uint8_t spiIndex = 0;
        int8_t clockPin = -1;
        int8_t dataPin = -1;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::RpUart, void>
        : TransportDescriptorTraitDefaults<typename lw::RpUartTransport::TransportSettingsType>
    {
        using TransportType = lw::RpUartTransport;
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

        static SettingsType fromConfig(const RpUartOptions &config,
                                       uint16_t)
        {
            SettingsType settings{};
            settings.invert = config.invert;
            settings.clockRateHz = config.clockRateHz;
            settings.bitOrder = config.bitOrder;
            settings.dataMode = config.dataMode;
            settings.spiIndex = config.spiIndex;
            settings.clockPin = config.clockPin;
            settings.dataPin = config.dataPin;
            return settings;
        }
    };

} // namespace factory
} // namespace lw

#endif
