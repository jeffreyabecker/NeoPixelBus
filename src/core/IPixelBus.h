#pragma once

#include <cstddef>
#include <cstdint>

#include "colors/ColorIterator.h"
#include "core/Compat.h"

namespace lw
{
    template <typename TColor>
    class IPixelBus
    {
    public:
        virtual ~IPixelBus() = default;

        virtual void begin() = 0;
        virtual void show() = 0;
        virtual bool isReadyToUpdate() const = 0;

        virtual span<TColor> pixelBuffer()= 0;
        virtual span<const TColor> pixelBuffer() const= 0;
    };

} // namespace lw
