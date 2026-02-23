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
#include <cstring>
#include <span>
#include <memory>
#include <vector>
#include <algorithm>

#include <Arduino.h>

extern "C"
{
    #include "eagle_soc.h"
    #include "esp8266_peri.h"
}

#include "IProtocol.h"
#include "ColorOrderTransform.h"
#include "OneWireTiming.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"
#include "../colors/Color.h"

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
    class Esp8266UartOneWireProtocol : public IProtocol
    {
    public:
        static constexpr size_t UartFifoSize = 128;
        static constexpr uint8_t Uart0Pin = 1;
        static constexpr uint8_t Uart1Pin = 2;

        Esp8266UartOneWireProtocol(uint16_t pixelCount,
                                 ResourceHandle<IShader> shader,
                                 Esp8266UartOneWireProtocolSettings settings)
            : _settings{settings}
            , _shader{std::move(shader)}
            , _transform{settings.colorConfig}
            , _pixelCount{pixelCount}
            , _sizeData{_transform.bytesNeeded(pixelCount)}
            , _scratchColors(pixelCount)
        {
            _data = static_cast<uint8_t *>(malloc(_sizeData));
            if (_data)
            {
                std::memset(_data, 0, _sizeData);
            }

            // Pre-compute byte send time for readiness check
            _byteSendTimeUs = computeByteSendTimeUs();
        }

        ~Esp8266UartOneWireProtocol()
        {
            if (_initialised)
            {
                // Wait for FIFO to drain
                uint8_t n = _settings.uartNumber;
                while ((USS(n) >> USTXC) & 0xFF)
                {
                    yield();
                }
            }
            free(_data);
        }

        Esp8266UartOneWireProtocol(const Esp8266UartOneWireProtocol &) = delete;
        Esp8266UartOneWireProtocol &operator=(const Esp8266UartOneWireProtocol &) = delete;
        Esp8266UartOneWireProtocol(Esp8266UartOneWireProtocol &&) = delete;
        Esp8266UartOneWireProtocol &operator=(Esp8266UartOneWireProtocol &&) = delete;

        void initialize() override
        {
            if (_initialised)
            {
                return;
            }

            configureUart();
            _startTime = micros();
            _initialised = true;
        }

        void update(std::span<const Color> colors) override
        {
            while (!isReadyToUpdate())
            {
                yield();
            }

            // Shade
            std::span<const Color> source = colors;
            if (nullptr != _shader)
            {
                std::copy(colors.begin(), colors.end(), _scratchColors.begin());
                _shader->apply(_scratchColors);
                source = _scratchColors;
            }

            // Transform
            _transform.apply(
                std::span<uint8_t>{_data, _sizeData}, source);

            // Send via UART FIFO (blocking)
            _startTime = micros();
            fillUartFifo();
        }

        bool isReadyToUpdate() const override
        {
            uint32_t elapsed = micros() - _startTime;
            uint32_t pixelTimeUs =
                static_cast<uint32_t>(_sizeData) * _byteSendTimeUs;
            return elapsed >= (pixelTimeUs + _settings.timing.resetUs);
        }

    private:
        Esp8266UartOneWireProtocolSettings _settings;
        ResourceHandle<IShader> _shader;
        ColorOrderTransform _transform;
        uint16_t _pixelCount;
        size_t _sizeData;

        std::vector<Color> _scratchColors;
        uint8_t *_data{nullptr};

        uint32_t _startTime{0};
        uint32_t _byteSendTimeUs{0};
        bool _initialised{false};

        // ---- UART lookup table ------------------------------------------
        // Maps 2-bit NeoPixel pairs to UART 6N1 bytes.
        // UART sends LSB first; start-bit (0) and stop-bit (1) are
        // integrated into the NRZ waveform.
        static constexpr uint8_t UartEncoding[4] =
        {
            0b110111, // NeoPixel bits 00
            0b000111, // NeoPixel bits 01
            0b110100, // NeoPixel bits 10
            0b000100  // NeoPixel bits 11
        };

        // ---- UART config ------------------------------------------------

        uint32_t computeBaud() const
        {
            // Each pixel byte → 4 UART bytes (6N1 = 8 bits on wire each)
            // Target: 800 kHz NRZ → baud = 3,200,000
            uint32_t nrzBitRateHz = static_cast<uint32_t>(
                _settings.timing.bitRateHz());
            return nrzBitRateHz * 4; // 4 UART bytes per pixel byte
        }

        uint32_t computeByteSendTimeUs() const
        {
            // Time to send one pixel byte via UART (4 UART bytes)
            uint32_t baud = computeBaud();
            if (baud == 0)
            {
                return 10;
            }
            // 4 UART bytes × 8 bits each / baud  (in microseconds)
            return (4UL * 8UL * 1000000UL) / baud;
        }

        void configureUart()
        {
            uint8_t n = _settings.uartNumber;
            uint8_t pin = (n == 0) ? Uart0Pin : Uart1Pin;

            // Set pin to UART TX function
            if (n == 0)
            {
                Serial.end();
                pinMode(pin, SPECIAL);
            }
            else
            {
                Serial1.end();
                pinMode(pin, SPECIAL);
            }

            // UART config: 6N1
            uint32_t baud = computeBaud();
            uint32_t uartClkDiv = (ESP8266_CLOCK / baud) & 0xFFFFF;
            USD(n) = uartClkDiv;
            USC0(n) = 0; // 6-bit, no parity, 1 stop bit

            // Clear all inversion bits first
            USC0(n) &= ~(BIT(UCDTRI) | BIT(UCRTSI) | BIT(UCTXI) |
                         BIT(UCDSRI) | BIT(UCCTSI) | BIT(UCRXI));

            // For "normal" NeoPixel signaling, we INVERT the UART TX output.
            // This is because the UART start bit (0) needs to produce a HIGH
            // edge on the NeoPixel wire.
            // For "inverted" NeoPixel signaling (e.g. TM1814), we do NOT
            // invert, so the start bit (0) becomes LOW on the wire.
            if (!_settings.invert)
            {
                USC0(n) |= BIT(UCTXI); // enable TX inversion
            }

            // Flush TX FIFO
            uint32_t tmp = USC0(n);
            tmp |= BIT(UCTXRST);
            USC0(n) = tmp;
            tmp &= ~BIT(UCTXRST);
            USC0(n) = tmp;
        }

        void fillUartFifo()
        {
            uint8_t n = _settings.uartNumber;

            for (size_t i = 0; i < _sizeData; ++i)
            {
                uint8_t value = _data[i];

                // Wait for FIFO space (need 4 bytes)
                while (((USS(n) >> USTXC) & 0xFF) > (UartFifoSize - 4))
                {
                    yield();
                }

                // Encode: 2 NeoPixel bits per UART byte, 4 UART bytes total
                USF(n) = UartEncoding[(value >> 6) & 0x03];
                USF(n) = UartEncoding[(value >> 4) & 0x03];
                USF(n) = UartEncoding[(value >> 2) & 0x03];
                USF(n) = UartEncoding[(value >> 0) & 0x03];
            }
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_ESP8266
