#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <utility>

#include <Arduino.h>

#include "ITransport.h"
#include "../ResourceHandle.h"

namespace npb
{

    struct PrintTransportConfig
    {
        ResourceHandle<Print> output = nullptr;
    };

    class PrintTransport : public ITransport
    {
    public:
        using TransportConfigType = PrintTransportConfig;
        using TransportCategory = AnyTransportTag;
        explicit PrintTransport(PrintTransportConfig config)
            : _config{std::move(config)}
        {
        }

        explicit PrintTransport(Print &output)
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
        PrintTransportConfig _config;
    };

} // namespace npb
