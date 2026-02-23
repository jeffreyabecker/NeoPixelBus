#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

namespace npb
{

    class IClockDataTransport
    {
    public:
        virtual ~IClockDataTransport() = default;

        virtual void begin() = 0;
        virtual void beginTransaction() = 0;
        virtual void transmitBytes(std::span<const uint8_t> data) = 0;
        virtual void endTransaction() = 0;
        // Default synchronous behavior: transports are ready immediately
        // after transmitBytes() returns. DMA-backed transports can override.
        virtual bool isReadyToUpdate() const
        {
            return true;
        }
    };

} // namespace npb
