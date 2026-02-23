#pragma once

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_IDF_TARGET_ESP32S3)

#include <cstdint>
#include <cstddef>
#include <memory>
#include <utility>

#include "../buses/OneWireTiming.h"
#include "Ws2812xProtocol.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"
#include "../buses/ISelfClockingTransport.h"
#include "../buses/Esp32LcdParallelSelfClockingTransport.h"

namespace npb
{

    struct Esp32LcdParallelOneWireProtocolSettings
    {
        uint8_t pin;
        OneWireTiming timing = timing::Ws2812x;
        bool invert = false;
        ColorOrderTransformConfig colorConfig;
    };

    class Esp32LcdParallelOneWireProtocol : public Ws2812xProtocol
    {
    public:
        Esp32LcdParallelOneWireProtocol(
            uint16_t pixelCount,
            ResourceHandle<IShader> shader,
            Esp32LcdParallelOneWireProtocolSettings settings)
            : Ws2812xProtocol(
                pixelCount,
                std::move(shader),
                settings.colorConfig,
                makeDefaultTransport(settings))
        {
        }

        Esp32LcdParallelOneWireProtocol(
            uint16_t pixelCount,
            ResourceHandle<IShader> shader,
            Esp32LcdParallelOneWireProtocolSettings settings,
            ResourceHandle<ISelfClockingTransport> transport)
            : Ws2812xProtocol(
                pixelCount,
                std::move(shader),
                settings.colorConfig,
                std::move(transport))
        {
        }

        Esp32LcdParallelOneWireProtocol(const Esp32LcdParallelOneWireProtocol &) = delete;
        Esp32LcdParallelOneWireProtocol &operator=(const Esp32LcdParallelOneWireProtocol &) = delete;
        Esp32LcdParallelOneWireProtocol(Esp32LcdParallelOneWireProtocol &&) = delete;
        Esp32LcdParallelOneWireProtocol &operator=(Esp32LcdParallelOneWireProtocol &&) = delete;

        bool alwaysUpdate() const override
        {
            return true;
        }

    private:
        static ResourceHandle<ISelfClockingTransport> makeDefaultTransport(
            const Esp32LcdParallelOneWireProtocolSettings &settings)
        {
            Esp32LcdParallelSelfClockingTransportConfig config{};
            config.pin = settings.pin;
            config.timing = settings.timing;
            config.invert = settings.invert;

            return ResourceHandle<ISelfClockingTransport>{
                std::make_unique<Esp32LcdParallelSelfClockingTransport>(config)};
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_ESP32 && CONFIG_IDF_TARGET_ESP32S3
