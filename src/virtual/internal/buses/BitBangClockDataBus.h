#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

#include <Arduino.h>

#include "IClockDataBus.h"

namespace npb
{

class BitBangClockDataBus : public IClockDataBus
{
public:
    BitBangClockDataBus(uint8_t pinClock, uint8_t pinData)
        : _pinClock{pinClock}
        , _pinData{pinData}
    {
        pinMode(_pinClock, OUTPUT);
        pinMode(_pinData, OUTPUT);
    }

    ~BitBangClockDataBus()
    {
        pinMode(_pinClock, INPUT);
        pinMode(_pinData, INPUT);
    }

    void begin() override
    {
        digitalWrite(_pinClock, LOW);
        digitalWrite(_pinData, LOW);
    }

    void beginTransaction() override
    {
        // no-op for bit-bang
    }

    void endTransaction() override
    {
        digitalWrite(_pinData, LOW);
    }

    void transmitBit(uint8_t bit) override
    {
        digitalWrite(_pinData, bit);
        digitalWrite(_pinClock, HIGH);
        digitalWrite(_pinClock, LOW);
    }

    void transmitByte(uint8_t data) override
    {
        for (int i = 7; i >= 0; --i)
        {
            digitalWrite(_pinData, (data & 0x80) ? HIGH : LOW);
            digitalWrite(_pinClock, HIGH);
            data <<= 1;
            digitalWrite(_pinClock, LOW);
        }
    }

    void transmitBytes(std::span<const uint8_t> data) override
    {
        for (auto byte : data)
        {
            transmitByte(byte);
        }
    }

private:
    const uint8_t _pinClock;
    const uint8_t _pinData;
};

} // namespace npb
