#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

#include <Print.h>

#include "IClockDataBus.h"

namespace npb
{

// Debug wrapper that prints all bus operations to a Print output.
// Optionally wraps an inner IClockDataBus to forward calls after logging.
class DebugClockDataBus : public IClockDataBus
{
public:
    explicit DebugClockDataBus(Print& output, IClockDataBus* inner = nullptr)
        : _output{output}
        , _inner{inner}
    {
    }

    void begin() override
    {
        _output.println("[BUS] begin");
        if (_inner)
        {
            _inner->begin();
        }
    }

    void beginTransaction() override
    {
        _output.println("[BUS] beginTransaction");
        if (_inner)
        {
            _inner->beginTransaction();
        }
    }

    void endTransaction() override
    {
        _output.println("[BUS] endTransaction");
        if (_inner)
        {
            _inner->endTransaction();
        }
    }

    void transmitByte(uint8_t data) override
    {
        static constexpr char Hex[] = "0123456789ABCDEF";
        _output.print("[BUS] byte: ");
        _output.print(Hex[data >> 4]);
        _output.println(Hex[data & 0x0F]);
        if (_inner)
        {
            _inner->transmitByte(data);
        }
    }

    void transmitBytes(std::span<const uint8_t> data) override
    {
        static constexpr char Hex[] = "0123456789ABCDEF";
        _output.print("[BUS] bytes(");
        _output.print(data.size());
        _output.print("): ");
        for (size_t i = 0; i < data.size(); ++i)
        {
            if (i > 0)
            {
                _output.print(' ');
            }
            _output.print(Hex[data[i] >> 4]);
            _output.print(Hex[data[i] & 0x0F]);
        }
        _output.println();
        if (_inner)
        {
            _inner->transmitBytes(data);
        }
    }

    void transmitBit(uint8_t bit) override
    {
        _output.print("[BUS] bit: ");
        _output.println(bit ? '1' : '0');
        if (_inner)
        {
            _inner->transmitBit(bit);
        }
    }

private:
    Print& _output;
    IClockDataBus* _inner;
};

} // namespace npb
