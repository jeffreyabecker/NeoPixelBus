#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

#include <Print.h>

#include "IEmitPixels.h"

namespace npb
{

class PrintEmitter : public IEmitPixels
{
public:
    explicit PrintEmitter(Print& output)
        : _output{output}
    {
    }

    void initialize() override
    {
        // no-op — no hardware to set up
    }

    void update(std::span<const uint8_t> data) override
    {
        static constexpr char HexDigits[] = "0123456789ABCDEF";

        for (size_t i = 0; i < data.size(); ++i)
        {
            if (i > 0)
            {
                _output.print(' ');
            }
            _output.print(HexDigits[data[i] >> 4]);
            _output.print(HexDigits[data[i] & 0x0F]);
        }
        _output.println();
    }

    bool isReadyToUpdate() const override
    {
        return true;
    }

    bool alwaysUpdate() const override
    {
        return false;
    }

private:
    Print& _output;
};

} // namespace npb
