#pragma once

#include <cstddef>
#include <cstdint>

#include "buses/composite/CompositeBusConfig.h"
#include "core/Compat.h"

namespace lw
{
#if LW_ENABLE_COMPOSITE_BUS

    template <typename TColor>
    class IBufferAccess
    {
    public:
        virtual ~IBufferAccess() = default;
        
        virtual void init(){}

        virtual span<TColor> rootPixels() = 0;
        virtual span<const TColor> rootPixels() const = 0;

        virtual span<TColor> shaderScratch() = 0;
        virtual span<const TColor> shaderScratch() const = 0;

        virtual size_t protocolSliceCount() const = 0;
        virtual span<uint8_t> protocolSlice(size_t strandIndex) = 0;
        virtual span<const uint8_t> protocolSlice(size_t strandIndex) const = 0;
    };

#endif

} // namespace lw
