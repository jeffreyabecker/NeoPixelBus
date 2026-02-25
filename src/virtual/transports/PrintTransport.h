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

    struct PrintTransportSettings
    {
        ResourceHandle<Print> output = nullptr;
    };

    class PrintTransport : public ITransport
    {
    public:
        using TransportSettingsType = PrintTransportSettings;
        using TransportCategory = AnyTransportTag;
        explicit PrintTransport(PrintTransportSettings config)
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
        PrintTransportSettings _config;
    };

} // namespace npb
