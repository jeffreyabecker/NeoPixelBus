#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

#include <Arduino.h>
#if __has_include(<SPI.h>)
#include <SPI.h>
#define NEOPIXELBUS_HAS_SPI_HEADER_ESP32_DMA_SPI 1
#else
#define NEOPIXELBUS_HAS_SPI_HEADER_ESP32_DMA_SPI 0
#endif

#include "IClockDataTransport.h"

namespace npb
{

#ifndef NEOPIXELBUS_ESP32_DMA_SPI_CLOCK_DEFAULT_HZ
#define NEOPIXELBUS_ESP32_DMA_SPI_CLOCK_DEFAULT_HZ 10000000UL
#endif
    static constexpr uint32_t Esp32DmaSpiClockDefaultHz = NEOPIXELBUS_ESP32_DMA_SPI_CLOCK_DEFAULT_HZ;

    class Esp32DmaSpiClockDataTransport : public IClockDataTransport
    {
    public:
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP32_DMA_SPI
        explicit Esp32DmaSpiClockDataTransport(uint32_t clockHz = Esp32DmaSpiClockDefaultHz,
                                               SPIClass &spi = SPI)
            : _clockHz{clockHz}
            , _spi{spi}
        {
        }
#else
        explicit Esp32DmaSpiClockDataTransport(uint32_t clockHz = Esp32DmaSpiClockDefaultHz)
            : _clockHz{clockHz}
        {
        }
#endif

        void begin() override
        {
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP32_DMA_SPI
            _spi.begin();
#endif
        }

        void beginTransaction() override
        {
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP32_DMA_SPI
            _spi.beginTransaction(SPISettings(_clockHz, MSBFIRST, SPI_MODE0));
#endif
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP32_DMA_SPI
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
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP32_DMA_SPI
            _spi.endTransaction();
#endif
        }

    private:
        uint32_t _clockHz;
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP32_DMA_SPI
        SPIClass &_spi;
#endif
    };

} // namespace npb
