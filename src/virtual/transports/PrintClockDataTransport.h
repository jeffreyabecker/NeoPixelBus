#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <utility>

#include <Arduino.h>
#include <Print.h>

#include "IClockDataTransport.h"
#include "../ResourceHandle.h"

namespace npb
{

    struct PrintClockDataTransportConfig
    {
        ResourceHandle<Print> output = nullptr;
    };

    class PrintClockDataTransport : public IClockDataTransport
    {
    public:
        explicit PrintClockDataTransport(PrintClockDataTransportConfig config)
            : _config{std::move(config)}
        {
        }

        explicit PrintClockDataTransport(Print &output)
            : _config{.output = output}
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
            if (_config.output == nullptr)
            {
                return;
            }

            _config.output->write(data.data(), data.size());
        }

        void endTransaction() override
        {
            // no-op for Print sink
        }

    private:
        PrintClockDataTransportConfig _config;
    };

} // namespace npb
