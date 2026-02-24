#pragma once

#ifdef ARDUINO_ARCH_ESP8266

#include <cstdint>
#include <cstddef>
#include <span>

#include <Arduino.h>

extern "C"
{
    #include "eagle_soc.h"
    #include "esp8266_peri.h"
}

#include "../ISelfClockingTransport.h"
#include "../SelfClockingTransportConfig.h"

namespace npb
{

    struct Esp8266UartSelfClockingTransportConfig : SelfClockingTransportConfig
    {
        uint8_t uartNumber = 1;
    };

    class Esp8266UartSelfClockingTransport : public ISelfClockingTransport
    {
    public:
        static constexpr size_t UartFifoSize = 128;
        static constexpr uint8_t Uart0Pin = 1;
        static constexpr uint8_t Uart1Pin = 2;

        explicit Esp8266UartSelfClockingTransport(Esp8266UartSelfClockingTransportConfig config)
            : _config{config}
        {
            _byteSendTimeUs = computeByteSendTimeUs();
        }

        ~Esp8266UartSelfClockingTransport()
        {
            if (!_initialised)
            {
                return;
            }

            uint8_t n = _config.uartNumber;
            while ((USS(n) >> USTXC) & 0xFF)
            {
                yield();
            }
        }

        void begin() override
        {
            if (_initialised)
            {
                return;
            }

            configureUart();
            _startTime = micros();
            _initialised = true;
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            if (!_initialised)
            {
                begin();
            }

            _lastPayloadSize = data.size();
            _startTime = micros();

            uint8_t n = _config.uartNumber;
            for (size_t i = 0; i < data.size(); ++i)
            {
                uint8_t value = data[i];

                while (((USS(n) >> USTXC) & 0xFF) > (UartFifoSize - 4))
                {
                    yield();
                }

                USF(n) = UartEncoding[(value >> 6) & 0x03];
                USF(n) = UartEncoding[(value >> 4) & 0x03];
                USF(n) = UartEncoding[(value >> 2) & 0x03];
                USF(n) = UartEncoding[(value >> 0) & 0x03];
            }
        }

        bool isReadyToUpdate() const override
        {
            if (!_initialised)
            {
                return true;
            }

            uint32_t elapsed = micros() - _startTime;
            uint32_t pixelTimeUs = static_cast<uint32_t>(_lastPayloadSize) * _byteSendTimeUs;
            return elapsed >= (pixelTimeUs + _config.timing.resetUs);
        }

    private:
        Esp8266UartSelfClockingTransportConfig _config;
        uint32_t _startTime{0};
        uint32_t _byteSendTimeUs{0};
        size_t _lastPayloadSize{0};
        bool _initialised{false};

        static constexpr uint8_t UartEncoding[4] =
        {
            0b110111,
            0b000111,
            0b110100,
            0b000100
        };

        uint32_t computeBaud() const
        {
            uint32_t nrzBitRateHz = static_cast<uint32_t>(_config.timing.bitRateHz());
            return nrzBitRateHz * 4;
        }

        uint32_t computeByteSendTimeUs() const
        {
            uint32_t baud = computeBaud();
            if (baud == 0)
            {
                return 10;
            }

            return (4UL * 8UL * 1000000UL) / baud;
        }

        void configureUart()
        {
            uint8_t n = _config.uartNumber;
            uint8_t pin = (n == 0) ? Uart0Pin : Uart1Pin;

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

            uint32_t baud = computeBaud();
            uint32_t uartClkDiv = (ESP8266_CLOCK / baud) & 0xFFFFF;
            USD(n) = uartClkDiv;
            USC0(n) = 0;

            USC0(n) &= ~(BIT(UCDTRI) | BIT(UCRTSI) | BIT(UCTXI) |
                         BIT(UCDSRI) | BIT(UCCTSI) | BIT(UCRXI));

            if (!_config.invert)
            {
                USC0(n) |= BIT(UCTXI);
            }

            uint32_t tmp = USC0(n);
            tmp |= BIT(UCTXRST);
            USC0(n) = tmp;
            tmp &= ~BIT(UCTXRST);
            USC0(n) = tmp;
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_ESP8266
