#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

namespace npb
{

class IClockDataBus
{
public:
    virtual ~IClockDataBus() = default;

    virtual void begin() = 0;
    virtual void beginTransaction() = 0;
    virtual void endTransaction() = 0;
    virtual void transmitByte(uint8_t data) = 0;
    virtual void transmitBytes(std::span<const uint8_t> data) = 0;
};

} // namespace npb
