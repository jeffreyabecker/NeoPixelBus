#pragma once

#if defined(ARDUINO_ARCH_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32S3) && !defined(CONFIG_IDF_TARGET_ESP32C3)

#include <cstdint>
#include <cstddef>
#include <memory>
#include <utility>

#include "../buses/OneWireTiming.h"
#include "Ws2812xProtocol.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"
#include "../buses/ISelfClockingTransport.h"
#include "../buses/Esp32I2sSelfClockingTransport.h"

namespace npb
{

    struct Esp32I2sOneWireProtocolSettings
    {
        uint8_t pin;
        uint8_t busNumber = 0;
        OneWireTiming timing = timing::Ws2812x;
        bool invert = false;
        ColorOrderTransformConfig colorConfig;
    };

    class Esp32I2sOneWireProtocol : public Ws2812xProtocol
    {
    public:
        Esp32I2sOneWireProtocol(
            uint16_t pixelCount,
            ResourceHandle<IShader> shader,
            Esp32I2sOneWireProtocolSettings settings)
            : Ws2812xProtocol(
                pixelCount,
                std::move(shader),
                settings.colorConfig,
                makeDefaultTransport(settings))
        {
        }

        Esp32I2sOneWireProtocol(
            uint16_t pixelCount,
            ResourceHandle<IShader> shader,
            Esp32I2sOneWireProtocolSettings settings,
            ResourceHandle<ISelfClockingTransport> transport)
            : Ws2812xProtocol(
                pixelCount,
                std::move(shader),
                settings.colorConfig,
                std::move(transport))
        {
        }

        Esp32I2sOneWireProtocol(const Esp32I2sOneWireProtocol &) = delete;
        Esp32I2sOneWireProtocol &operator=(const Esp32I2sOneWireProtocol &) = delete;
        Esp32I2sOneWireProtocol(Esp32I2sOneWireProtocol &&) = delete;
        Esp32I2sOneWireProtocol &operator=(Esp32I2sOneWireProtocol &&) = delete;

    private:
        static ResourceHandle<ISelfClockingTransport> makeDefaultTransport(
            const Esp32I2sOneWireProtocolSettings &settings)
        {
            Esp32I2sSelfClockingTransportConfig config{};
            config.pin = settings.pin;
            config.busNumber = settings.busNumber;
            config.timing = settings.timing;
            config.invert = settings.invert;

            return ResourceHandle<ISelfClockingTransport>{
                std::make_unique<Esp32I2sSelfClockingTransport>(config)};
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_ESP32 && !ESP32S3 && !ESP32C3
