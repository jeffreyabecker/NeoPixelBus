#pragma once

#include <cstddef>
#include <cstdint>
#include <algorithm>

#include "colors/ColorIterator.h"
#include "core/Compat.h"

namespace npb
{

    class Topology;

    template <typename TColor>
    class IPixelBus
    {
    public:
        virtual ~IPixelBus() = default;

        virtual void begin() = 0;
        virtual void show() = 0;
        virtual bool canShow() const = 0;



        // -----------------------------------------------------------------
        // Contiguous buffer capability seam.
        // Default is empty span (capability absent).
        // -----------------------------------------------------------------
        virtual span<TColor> pixelBuffer()
        {
            return span<TColor>{};
        }

        virtual span<const TColor> pixelBuffer() const
        {
            return span<const TColor>{};
        }

    };

    template <typename TColor>
    class I2dPixelBus : public IPixelBus<TColor>
    {
    public:
        virtual ~I2dPixelBus() = default;

        virtual const Topology &topology() const = 0;
    };

} // namespace npb

