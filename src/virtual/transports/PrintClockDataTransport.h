#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

#include <Arduino.h>
#include <Print.h>

#include "IClockDataTransport.h"

namespace npb
{

    class PrintClockDataTransport : public IClockDataTransport
    {
    public:
        explicit PrintClockDataTransport(Print &output)
            : _output{output}
        {
        }

        void begin() override
        {
            // Print instance setup is owned by caller.
        }

        void beginTransaction() override
        {
            // no-op for Print sink
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            _output.write(data.data(), data.size());
        }

        void endTransaction() override
        {
            // no-op for Print sink
        }

    private:
        Print &_output;
    };

} // namespace npb
