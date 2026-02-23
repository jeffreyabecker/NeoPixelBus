#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

#include <Print.h>

#include "ISelfClockingTransport.h"

namespace npb
{

    // Debug wrapper that prints self-clocking transport operations to a Print output.
    // Optionally wraps an inner ISelfClockingTransport to forward calls after logging.
    class DebugSelfClockingTransport : public ISelfClockingTransport
    {
    public:
        explicit DebugSelfClockingTransport(Print &output,
                                            ISelfClockingTransport *inner = nullptr)
            : _output{output}
            , _inner{inner}
        {
        }

        void begin() override
        {
            _output.println("[SELF] begin");
            if (_inner)
            {
                _inner->begin();
            }
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            static constexpr char Hex[] = "0123456789ABCDEF";

            _output.print("[SELF] bytes(");
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

        bool isReadyToUpdate() const override
        {
            if (_inner)
            {
                return _inner->isReadyToUpdate();
            }

            return true;
        }

    private:
        Print &_output;
        ISelfClockingTransport *_inner;
    };

} // namespace npb
