#pragma once

// ESP8266 UART one-wire emitter.
// Uses the UART TX peripheral to bit-shape NeoPixel NRZ signals.
// Fixed pins:  UART0 → GPIO1 (Serial TX),  UART1 → GPIO2.
//
// Encoding: 6N1 UART framing, 4 UART bytes per pixel byte.
//   Each UART byte encodes 2 NeoPixel bits using the start/stop bit
//   integration:
//     NeoPixel 00 → UART 0b110111
//     NeoPixel 01 → UART 0b000111
//     NeoPixel 10 → UART 0b110100
//     NeoPixel 11 → UART 0b000100
//
// UART TX is hardware-inverted for "normal" NeoPixel signaling
// (start bit = 0 on wire produces the leading HIGH edge of the NRZ pulse).

#ifdef ARDUINO_ARCH_ESP8266

#include <cstdint>
#include <cstddef>
#include <memory>

#include "OneWireTiming.h"
#include "Ws2812xProtocol.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"
#include "../buses/ISelfClockingTransport.h"
#include "../buses/Esp8266UartSelfClockingTransport.h"

namespace npb
{

    /// Construction settings for Esp8266UartOneWireProtocol.
    struct Esp8266UartOneWireProtocolSettings
    {
        uint8_t uartNumber = 1;    // 0 = UART0/GPIO1, 1 = UART1/GPIO2
        OneWireTiming timing = timing::Ws2812x;
        bool invert = false;
        ColorOrderTransformConfig colorConfig;
    };

    /// One-wire NRZ emitter using ESP8266 UART TX (synchronous FIFO-fill).
    ///
    /// Supports UART0 (GPIO1) and UART1 (GPIO2).  Only one instance per
    /// UART peripheral.  Uses synchronous (blocking) FIFO writes.
    class Esp8266UartOneWireProtocol : public Ws2812xProtocol
    {
    public:
        Esp8266UartOneWireProtocol(
            uint16_t pixelCount,
            ResourceHandle<IShader> shader,
            Esp8266UartOneWireProtocolSettings settings)
            : Ws2812xProtocol(
                pixelCount,
                std::move(shader),
                settings.colorConfig,
                makeDefaultTransport(settings))
        {
        }

        Esp8266UartOneWireProtocol(
            uint16_t pixelCount,
            ResourceHandle<IShader> shader,
            Esp8266UartOneWireProtocolSettings settings,
            ResourceHandle<ISelfClockingTransport> transport)
            : Ws2812xProtocol(
                pixelCount,
                std::move(shader),
                settings.colorConfig,
                std::move(transport))
        {
        }

        Esp8266UartOneWireProtocol(const Esp8266UartOneWireProtocol &) = delete;
        Esp8266UartOneWireProtocol &operator=(const Esp8266UartOneWireProtocol &) = delete;
        Esp8266UartOneWireProtocol(Esp8266UartOneWireProtocol &&) = delete;
        Esp8266UartOneWireProtocol &operator=(Esp8266UartOneWireProtocol &&) = delete;

    private:
        static ResourceHandle<ISelfClockingTransport> makeDefaultTransport(
            const Esp8266UartOneWireProtocolSettings &settings)
        {
            Esp8266UartSelfClockingTransportConfig cfg{};
            cfg.uartNumber = settings.uartNumber;
            cfg.invert = settings.invert;
            cfg.timing = settings.timing;

            return ResourceHandle<ISelfClockingTransport>{
                std::make_unique<Esp8266UartSelfClockingTransport>(cfg)};
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_ESP8266
