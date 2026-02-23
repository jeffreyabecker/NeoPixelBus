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
#include "../buses/Esp32I2sParallelSelfClockingTransport.h"

namespace npb
{

    struct Esp32I2sParallelOneWireProtocolSettings
    {
        uint8_t pin;
        uint8_t busNumber = 1;
        OneWireTiming timing = timing::Ws2812x;
        bool invert = false;
        ColorOrderTransformConfig colorConfig;
    };

    class Esp32I2sParallelOneWireProtocol : public Ws2812xProtocol
    {
    public:
        Esp32I2sParallelOneWireProtocol(
            uint16_t pixelCount,
            ResourceHandle<IShader> shader,
            Esp32I2sParallelOneWireProtocolSettings settings)
            : Ws2812xProtocol(
                pixelCount,
                std::move(shader),
                settings.colorConfig,
                makeDefaultTransport(settings))
        {
        }

        Esp32I2sParallelOneWireProtocol(
            uint16_t pixelCount,
            ResourceHandle<IShader> shader,
            Esp32I2sParallelOneWireProtocolSettings settings,
            ResourceHandle<ISelfClockingTransport> transport)
            : Ws2812xProtocol(
                pixelCount,
                std::move(shader),
                settings.colorConfig,
                std::move(transport))
        {
        }

        Esp32I2sParallelOneWireProtocol(const Esp32I2sParallelOneWireProtocol &) = delete;
        Esp32I2sParallelOneWireProtocol &operator=(const Esp32I2sParallelOneWireProtocol &) = delete;
        Esp32I2sParallelOneWireProtocol(Esp32I2sParallelOneWireProtocol &&) = delete;
        Esp32I2sParallelOneWireProtocol &operator=(Esp32I2sParallelOneWireProtocol &&) = delete;

        bool alwaysUpdate() const override
        {
            return true;
        }

    private:
        static ResourceHandle<ISelfClockingTransport> makeDefaultTransport(
            const Esp32I2sParallelOneWireProtocolSettings &settings)
        {
            Esp32I2sParallelSelfClockingTransportConfig config{};
            config.pin = settings.pin;
            config.busNumber = settings.busNumber;
            config.timing = settings.timing;
            config.invert = settings.invert;

            return ResourceHandle<ISelfClockingTransport>{
                std::make_unique<Esp32I2sParallelSelfClockingTransport>(config)};
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_ESP32 && !ESP32S3 && !ESP32C3
