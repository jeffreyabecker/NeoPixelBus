#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

#include <Arduino.h>
#if __has_include(<SPI.h>)
#include <SPI.h>
#define NEOPIXELBUS_HAS_SPI_HEADER_ESP32_I2S 1
#else
#define NEOPIXELBUS_HAS_SPI_HEADER_ESP32_I2S 0
#endif

#include "IClockDataTransport.h"

namespace npb
{

#ifndef NEOPIXELBUS_ESP32_I2S_CLOCK_DEFAULT_HZ
#define NEOPIXELBUS_ESP32_I2S_CLOCK_DEFAULT_HZ 10000000UL
#endif
    static constexpr uint32_t Esp32I2SClockDefaultHz = NEOPIXELBUS_ESP32_I2S_CLOCK_DEFAULT_HZ;

    class Esp32I2SClockDataTransport : public IClockDataTransport
    {
    public:
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP32_I2S
        explicit Esp32I2SClockDataTransport(uint32_t clockHz = Esp32I2SClockDefaultHz,
                                            SPIClass &spi = SPI)
            : _clockHz{clockHz}
            , _spi{spi}
        {
        }
#else
        explicit Esp32I2SClockDataTransport(uint32_t clockHz = Esp32I2SClockDefaultHz)
            : _clockHz{clockHz}
        {
        }
#endif

        void begin() override
        {
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP32_I2S
            _spi.begin();
#endif
        }

        void beginTransaction() override
        {
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP32_I2S
            // Maintains the same byte-stream semantics as SPI transports.
            _spi.beginTransaction(SPISettings(_clockHz, MSBFIRST, SPI_MODE0));
#endif
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP32_I2S
#if defined(ARDUINO_ARCH_ESP32)
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
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP32_I2S
            _spi.endTransaction();
#endif
        }

    private:
        uint32_t _clockHz;
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP32_I2S
        SPIClass &_spi;
#endif
    };

} // namespace npb
