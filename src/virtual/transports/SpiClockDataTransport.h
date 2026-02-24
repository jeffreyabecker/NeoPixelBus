#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <utility>

#include <Arduino.h>
#include <SPI.h>

#include "IClockDataTransport.h"
#include "../ResourceHandle.h"

namespace npb
{

#ifndef NEOPIXELBUS_SPI_CLOCK_DEFAULT_HZ
#define NEOPIXELBUS_SPI_CLOCK_DEFAULT_HZ 10000000UL
#endif
    static constexpr uint32_t SpiClockDefaultHz = NEOPIXELBUS_SPI_CLOCK_DEFAULT_HZ;

    struct SpiClockDataTransportConfig
    {
        bool invert = false;
        uint32_t clockDataBitRateHz = SpiClockDefaultHz;
        ResourceHandle<SPIClass> spi = SPI;
    };

    class SpiClockDataTransport : public IClockDataTransport
    {
    public:
        explicit SpiClockDataTransport(SpiClockDataTransportConfig config)
            : _config{std::move(config)}
        {
        }

        explicit SpiClockDataTransport(uint32_t clockHz = SpiClockDefaultHz)
            : _config{.clockDataBitRateHz = clockHz}
        {
        }

        explicit SpiClockDataTransport(uint32_t clockHz,
                                       SPIClass &spi)
            : _config{.clockDataBitRateHz = clockHz, .spi = spi}
        {
        }

        void begin() override
        {
            _config.spi->begin();
        }

        void beginTransaction() override
        {
            _config.spi->beginTransaction(SPISettings(_config.clockDataBitRateHz, MSBFIRST, SPI_MODE0));
        }

        void endTransaction() override
        {
            _config.spi->endTransaction();
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
            // ESPs have a non-destructive write method
            _config.spi->writeBytes(const_cast<uint8_t *>(data.data()), data.size());
#else
            for (auto byte : data)
            {
                _config.spi->transfer(byte);
            }
#endif
        }

    private:
        SpiClockDataTransportConfig _config;
    };

} // namespace npb
