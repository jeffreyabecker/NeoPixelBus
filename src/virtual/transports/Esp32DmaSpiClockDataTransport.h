#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <memory>

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
                        , _spi{&spi}
                {
                }

#if defined(ARDUINO_ARCH_ESP32)
                explicit Esp32DmaSpiClockDataTransport(uint8_t spiBus,
                                                                                           uint32_t clockHz = Esp32DmaSpiClockDefaultHz)
                        : _clockHz{clockHz}
                        , _ownedSpi{std::make_unique<SPIClass>(spiBus)}
                        , _spi{_ownedSpi.get()}
        {
        }
#endif
#else
        explicit Esp32DmaSpiClockDataTransport(uint32_t clockHz = Esp32DmaSpiClockDefaultHz)
            : _clockHz{clockHz}
        {
        }
#endif

        void begin() override
        {
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP32_DMA_SPI
                        if (_spi)
                        {
                                _spi->begin();
                        }
#endif
        }

        void beginTransaction() override
        {
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP32_DMA_SPI
                        if (_spi)
                        {
                                _spi->beginTransaction(SPISettings(_clockHz, MSBFIRST, SPI_MODE0));
                        }
#endif
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP32_DMA_SPI
#if defined(ARDUINO_ARCH_ESP32)
                        if (_spi)
                        {
                                _spi->writeBytes(const_cast<uint8_t *>(data.data()), data.size());
                        }
#else
                        if (_spi)
            {
                                for (const auto byte : data)
                                {
                                        _spi->transfer(byte);
                                }
            }
#endif
    #else
            (void)data;
    #endif
        }

        void endTransaction() override
        {
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP32_DMA_SPI
                        if (_spi)
                        {
                                _spi->endTransaction();
                        }
#endif
        }

    private:
        uint32_t _clockHz;
#if NEOPIXELBUS_HAS_SPI_HEADER_ESP32_DMA_SPI
        std::unique_ptr<SPIClass> _ownedSpi;
        SPIClass *_spi;
#endif
    };

} // namespace npb
