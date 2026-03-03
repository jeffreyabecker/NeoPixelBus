#pragma once

#include <cstddef>
#include <cstdint>

#include "core/Compat.h"

namespace lw
{

    struct ProtocolSliceRange
    {
        size_t offset{0};
        size_t size{0};
    };

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

    template <typename TColor>
    class IBufferAccessProvider
    {
    public:
        virtual ~IBufferAccessProvider() = default;

        virtual IBufferAccess<TColor> &bufferAccess() = 0;
        virtual const IBufferAccess<TColor> &bufferAccess() const = 0;
    };

} // namespace lw
