#pragma once

#ifdef ARDUINO_ARCH_ESP32

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/OneWireWrapper.h"
#include "transports/esp32/Esp32RmtTransport.h"

namespace lw
{
namespace factory
{

    struct Esp32RmtOptions
    {
        rmt_channel_t channel = RMT_CHANNEL_0;
        OneWireTiming timing = timing::Ws2812x;
        uint32_t clockRateHz = 0;
        int dataPin = 0;
        bool invert = false;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::Esp32Rmt, void>
        : TransportDescriptorTraitDefaults<typename lw::Esp32RmtTransport::TransportSettingsType>
    {
        using TransportType = lw::Esp32RmtTransport;
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
                normalizeOneWireTransportClockDataBitRate(*timing, settings);
            }
            else if (settings.clockRateHz == 0)
            {
                normalizeOneWireTransportClockDataBitRate(lw::timing::Ws2812x, settings);
            }

            return settings;
        }

        static SettingsType fromConfig(const Esp32RmtOptions &config,
                                       uint16_t)
        {
            SettingsType settings{};
            settings.channel = config.channel;
            settings.clockRateHz = config.clockRateHz;
            settings.dataPin = config.dataPin;
            settings.invert = config.invert;

            if (settings.clockRateHz == 0)
            {
                normalizeOneWireTransportClockDataBitRate(config.timing, settings);
            }

            return settings;
        }
    };

} // namespace factory
} // namespace lw

#endif
