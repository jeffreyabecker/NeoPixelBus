#pragma once

#if defined(ARDUINO_ARCH_NRF52840)

#include <cstdint>
#include <cstddef>
#include <memory>
#include <utility>

#include "../buses/OneWireTiming.h"
#include "Ws2812xProtocol.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"
#include "../buses/ISelfClockingTransport.h"
#include "../buses/Nrf52PwmSelfClockingTransport.h"

namespace npb
{

    struct Nrf52PwmOneWireProtocolSettings
    {
        uint8_t pin;
        uint8_t pwmIndex = 2;
        OneWireTiming timing = timing::Ws2812x;
        bool invert = false;
        ColorOrderTransformConfig colorConfig;
    };

    class Nrf52PwmOneWireProtocol : public Ws2812xProtocol
    {
    public:
        Nrf52PwmOneWireProtocol(
            uint16_t pixelCount,
            ResourceHandle<IShader> shader,
            Nrf52PwmOneWireProtocolSettings settings)
            : Ws2812xProtocol(
                pixelCount,
                std::move(shader),
                settings.colorConfig,
                makeDefaultTransport(settings))
        {
        }

        Nrf52PwmOneWireProtocol(
            uint16_t pixelCount,
            ResourceHandle<IShader> shader,
            Nrf52PwmOneWireProtocolSettings settings,
            ResourceHandle<ISelfClockingTransport> transport)
            : Ws2812xProtocol(
                pixelCount,
                std::move(shader),
                settings.colorConfig,
                std::move(transport))
        {
        }

        Nrf52PwmOneWireProtocol(const Nrf52PwmOneWireProtocol &) = delete;
        Nrf52PwmOneWireProtocol &operator=(const Nrf52PwmOneWireProtocol &) = delete;
        Nrf52PwmOneWireProtocol(Nrf52PwmOneWireProtocol &&) = delete;
        Nrf52PwmOneWireProtocol &operator=(Nrf52PwmOneWireProtocol &&) = delete;

    private:
        static ResourceHandle<ISelfClockingTransport> makeDefaultTransport(
            const Nrf52PwmOneWireProtocolSettings &settings)
        {
            Nrf52PwmSelfClockingTransportConfig config{};
            config.pin = settings.pin;
            config.pwmIndex = settings.pwmIndex;
            config.timing = settings.timing;
            config.invert = settings.invert;

            return ResourceHandle<ISelfClockingTransport>{
                std::make_unique<Nrf52PwmSelfClockingTransport>(config)};
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_NRF52840
