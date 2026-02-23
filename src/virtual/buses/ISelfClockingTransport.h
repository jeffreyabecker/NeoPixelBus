#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

namespace npb
{

    class ISelfClockingTransport
    {
    public:
        virtual ~ISelfClockingTransport() = default;

        // Initialize and start transport resources.
        virtual void begin() = 0;

        // Submit already-encoded protocol payload for transmission.
        virtual void transmitBytes(std::span<const uint8_t> data) = 0;

        // True when transport can accept a new frame update.
        virtual bool isReadyToUpdate() const = 0;
    };

} // namespace npb
