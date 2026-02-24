#pragma once

#include <cstdint>

#include "../ResourceHandle.h"
#include "IClockDataTransport.h"

namespace npb
{

    class IParallelDataTransport
    {
    public:
        virtual ~IParallelDataTransport() = default;

        virtual void begin() = 0;
        virtual ResourceHandle<IClockDataTransport> getLane(uint8_t lane) = 0;
        virtual bool isReadyToUpdate() const = 0;
    };

} // namespace npb
