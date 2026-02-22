#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

#include <Arduino.h>
#include <SPI.h>

#include "IClockDataBus.h"

namespace npb
{

#ifndef NEOPIXELBUS_SPI_CLOCK_DEFAULT_HZ
#define NEOPIXELBUS_SPI_CLOCK_DEFAULT_HZ 10000000UL
#endif
static constexpr uint32_t SpiClockDefaultHz = NEOPIXELBUS_SPI_CLOCK_DEFAULT_HZ;

class SpiClockDataBus : public IClockDataBus
{
public:
    explicit SpiClockDataBus(uint32_t clockHz = SpiClockDefaultHz,
                             SPIClass& spi = SPI)
        : _clockHz{clockHz}
        , _spi{spi}
    {
    }

    void begin() override
    {
        _spi.begin();
    }

    void beginTransaction() override
    {
        _spi.beginTransaction(SPISettings(_clockHz, MSBFIRST, SPI_MODE0));
    }

    void endTransaction() override
    {
        _spi.endTransaction();
    }

    void transmitByte(uint8_t data) override
    {
        _spi.transfer(data);
    }

    void transmitBytes(std::span<const uint8_t> data) override
    {
#if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
        // ESPs have a non-destructive write method
        _spi.writeBytes(const_cast<uint8_t*>(data.data()), data.size());
#else
        for (auto byte : data)
        {
            _spi.transfer(byte);
        }
#endif
    }

    // SPI hardware cannot send individual bits.
    // Protocols requiring transmitBit() (SM16716, MBI6033) must use
    // BitBangClockDataBus instead.
    void transmitBit([[maybe_unused]] uint8_t bit) override
    {
        // Panic — SPI cannot send individual bits
        while (true)
        {
            delay(1000);
        }
    }

private:
    uint32_t _clockHz;
    SPIClass& _spi;
};

} // namespace npb
