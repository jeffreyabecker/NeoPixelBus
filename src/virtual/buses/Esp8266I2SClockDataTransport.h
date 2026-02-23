#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

#include <Arduino.h>
#if __has_include(<SPI.h>)
#include <SPI.h>
#define NEOPIXELBUS_HAS_SPI_HEADER_ESP8266_I2S 1
#else
#define NEOPIXELBUS_HAS_SPI_HEADER_ESP8266_I2S 0
#endif

#include "IClockDataTransport.h"

namespace npb
{

#ifndef NEOPIXELBUS_ESP8266_I2S_CLOCK_DEFAULT_HZ
#define NEOPIXELBUS_ESP8266_I2S_CLOCK_DEFAULT_HZ 4000000UL
#endif
    static constexpr uint32_t Esp8266I2SClockDefaultHz = NEOPIXELBUS_ESP8266_I2S_CLOCK_DEFAULT_HZ;

    class Esp8266I2SClockDataTransport : public IClockDataTransport
    {
    public:
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP8266_I2S
        explicit Esp8266I2SClockDataTransport(uint32_t clockHz = Esp8266I2SClockDefaultHz,
                                              SPIClass &spi = SPI)
            : _clockHz{clockHz}
            , _spi{spi}
        {
        }
#else
        explicit Esp8266I2SClockDataTransport(uint32_t clockHz = Esp8266I2SClockDefaultHz)
            : _clockHz{clockHz}
        {
        }
#endif

        void begin() override
        {
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP8266_I2S
            _spi.begin();
#endif
        }

        void beginTransaction() override
        {
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP8266_I2S
            _spi.beginTransaction(SPISettings(_clockHz, MSBFIRST, SPI_MODE0));
#endif
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP8266_I2S
#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
            _spi.writeBytes(const_cast<uint8_t *>(data.data()), data.size());
#else
            for (const auto byte : data)
            {
                _spi.transfer(byte);
            }
#endif
    #else
            (void)data;
    #endif
        }

        void endTransaction() override
        {
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP8266_I2S
            _spi.endTransaction();
#endif
        }

    private:
        uint32_t _clockHz;
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP8266_I2S
        SPIClass &_spi;
#endif
    };

} // namespace npb
