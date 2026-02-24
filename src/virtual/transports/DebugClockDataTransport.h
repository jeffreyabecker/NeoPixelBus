#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <utility>

#include <Print.h>

#include "IClockDataTransport.h"
#include "../ResourceHandle.h"

namespace npb
{

    struct DebugClockDataTransportConfig
    {
        ResourceHandle<Print> output = nullptr;
        ResourceHandle<IClockDataTransport> inner = nullptr;
        bool invert = false;
    };

    // Debug wrapper that prints all bus operations to a Print output.
    // Optionally wraps an inner IClockDataTransport to forward calls after logging.
    class DebugClockDataTransport : public IClockDataTransport
    {
    public:
        explicit DebugClockDataTransport(DebugClockDataTransportConfig config)
            : _config{std::move(config)}
        {
        }

        explicit DebugClockDataTransport(Print &output, IClockDataTransport *inner = nullptr)
            : _config{.output = output}
        {
            if (inner != nullptr)
            {
                _config.inner = ResourceHandle<IClockDataTransport>{*inner};
            }
        }

        void begin() override
        {
            if (_config.output != nullptr)
            {
                _config.output->println("[BUS] begin");
            }
            if (_config.inner != nullptr)
            {
                _config.inner->begin();
            }
        }

        void beginTransaction() override
        {
            if (_config.output != nullptr)
            {
                _config.output->println("[BUS] beginTransaction");
            }
            if (_config.inner != nullptr)
            {
                _config.inner->beginTransaction();
            }
        }

        void endTransaction() override
        {
            if (_config.output != nullptr)
            {
                _config.output->println("[BUS] endTransaction");
            }
            if (_config.inner != nullptr)
            {
                _config.inner->endTransaction();
            }
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            static constexpr char Hex[] = "0123456789ABCDEF";
            if (_config.output != nullptr)
            {
                _config.output->print("[BUS] bytes(");
                _config.output->print(data.size());
                _config.output->print("): ");
                for (size_t i = 0; i < data.size(); ++i)
                {
                    if (i > 0)
                    {
                        _config.output->print(' ');
                    }
                    uint8_t byte = data[i];
                    if (_config.invert)
                    {
                        byte = ~byte;
                    }
                    _config.output->print(Hex[byte >> 4]);
                    _config.output->print(Hex[byte & 0x0F]);
                }
                _config.output->println();
            }

            if (_config.inner != nullptr)
            {
                _config.inner->transmitBytes(data);
            }
        }

    private:
        DebugClockDataTransportConfig _config;
    };

} // namespace npb
