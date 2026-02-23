#pragma once

#ifdef ARDUINO_ARCH_ESP8266

#include <cstdint>
#include <cstddef>
#include <memory>
#include <utility>

#include "../buses/OneWireTiming.h"
#include "Ws2812xProtocol.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"
#include "../buses/ISelfClockingTransport.h"
#include "../buses/Esp8266DmaSelfClockingTransport.h"

namespace npb
{

    struct Esp8266DmaOneWireProtocolSettings
    {
        OneWireTiming timing = timing::Ws2812x;
        bool invert = false;
        ColorOrderTransformConfig colorConfig;
    };

    class Esp8266DmaOneWireProtocol : public Ws2812xProtocol
    {
    public:
        Esp8266DmaOneWireProtocol(
            uint16_t pixelCount,
            ResourceHandle<IShader> shader,
            Esp8266DmaOneWireProtocolSettings settings)
            : Ws2812xProtocol(
                pixelCount,
                std::move(shader),
                settings.colorConfig,
                makeDefaultTransport(settings))
        {
        }

        Esp8266DmaOneWireProtocol(
            uint16_t pixelCount,
            ResourceHandle<IShader> shader,
            Esp8266DmaOneWireProtocolSettings settings,
            ResourceHandle<ISelfClockingTransport> transport)
            : Ws2812xProtocol(
                pixelCount,
                std::move(shader),
                settings.colorConfig,
                std::move(transport))
        {
        }

        Esp8266DmaOneWireProtocol(const Esp8266DmaOneWireProtocol &) = delete;
        Esp8266DmaOneWireProtocol &operator=(const Esp8266DmaOneWireProtocol &) = delete;
        Esp8266DmaOneWireProtocol(Esp8266DmaOneWireProtocol &&) = delete;
        Esp8266DmaOneWireProtocol &operator=(Esp8266DmaOneWireProtocol &&) = delete;

    private:
        static ResourceHandle<ISelfClockingTransport> makeDefaultTransport(
            const Esp8266DmaOneWireProtocolSettings &settings)
        {
            Esp8266DmaSelfClockingTransportConfig config{};
            config.timing = settings.timing;
            config.invert = settings.invert;

            return ResourceHandle<ISelfClockingTransport>{
                std::make_unique<Esp8266DmaSelfClockingTransport>(config)};
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_ESP8266
