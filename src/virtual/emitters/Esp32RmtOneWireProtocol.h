#pragma once

#if defined(ARDUINO_ARCH_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32C6) && !defined(CONFIG_IDF_TARGET_ESP32H2)

#include <cstdint>
#include <cstddef>
#include <memory>
#include <utility>

#include "OneWireTiming.h"
#include "Ws2812xProtocol.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"
#include "../buses/ISelfClockingTransport.h"
#include "../buses/Esp32RmtSelfClockingTransport.h"

namespace npb
{

    /// Construction settings for Esp32RmtOneWireProtocol.
    struct Esp32RmtOneWireProtocolSettings
    {
        uint8_t pin;
        rmt_channel_t channel = RMT_CHANNEL_0;
        OneWireTiming timing = timing::Ws2812x;
        bool invert = false;
        ColorOrderTransformConfig colorConfig;
    };

    /// One-wire NRZ emitter for ESP32 using the RMT peripheral.
    ///
    /// Each instance uses one RMT channel. The RMT translator callback
    /// converts pixel bytes to RMT items on the fly, avoiding a large
    /// pre-encoded buffer.
    ///
    /// Signal inversion swaps the RMT item polarity and idle level.
    class Esp32RmtOneWireProtocol : public Ws2812xProtocol
    {
    public:
        Esp32RmtOneWireProtocol(
            uint16_t pixelCount,
            ResourceHandle<IShader> shader,
            Esp32RmtOneWireProtocolSettings settings)
            : Ws2812xProtocol(
                pixelCount,
                std::move(shader),
                settings.colorConfig,
                makeDefaultTransport(settings))
        {
        }

        Esp32RmtOneWireProtocol(
            uint16_t pixelCount,
            ResourceHandle<IShader> shader,
            Esp32RmtOneWireProtocolSettings settings,
            ResourceHandle<ISelfClockingTransport> transport)
            : Ws2812xProtocol(
                pixelCount,
                std::move(shader),
                settings.colorConfig,
                std::move(transport))
        {
        }

        Esp32RmtOneWireProtocol(const Esp32RmtOneWireProtocol &) = delete;
        Esp32RmtOneWireProtocol &operator=(const Esp32RmtOneWireProtocol &) = delete;
        Esp32RmtOneWireProtocol(Esp32RmtOneWireProtocol &&) = delete;
        Esp32RmtOneWireProtocol &operator=(Esp32RmtOneWireProtocol &&) = delete;

    private:
        static ResourceHandle<ISelfClockingTransport> makeDefaultTransport(
            const Esp32RmtOneWireProtocolSettings &settings)
        {
            Esp32RmtSelfClockingTransportConfig cfg{};
            cfg.pin = settings.pin;
            cfg.channel = settings.channel;
            cfg.timing = settings.timing;
            cfg.invert = settings.invert;

            return ResourceHandle<ISelfClockingTransport>{
                std::make_unique<Esp32RmtSelfClockingTransport>(cfg)};
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_ESP32
