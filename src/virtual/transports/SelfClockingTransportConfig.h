#pragma once

#include <cstdint>

#include "OneWireTiming.h"

namespace npb
{

    // Shared transport-level configuration for one-wire self-clocking engines.
    // Platform-specific transports can extend this via composition and/or by
    // consuming extensionTag as a lightweight discriminator.
    struct SelfClockingTransportConfig
    {
        uint8_t pin = 0;
        uint8_t channel = 0;
        uint8_t busId = 0;
        uint8_t parallelLane = 0;
        bool invert = false;
        OneWireTiming timing = timing::Ws2812x;
        uint32_t extensionTag = 0;
    };

} // namespace npb
