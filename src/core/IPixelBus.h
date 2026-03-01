#pragma once

#include <cstddef>
#include <cstdint>

#include "colors/ColorIterator.h"
#include "core/Compat.h"

namespace lw
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

        virtual span<TColor> pixelBuffer()= 0;
        virtual span<const TColor> pixelBuffer() const= 0;
    };

    template <typename TColor>
    class IAssignableBufferBus : public IPixelBus<TColor>
    {
    public:
        virtual ~IAssignableBufferBus() = default;
        virtual uint16_t pixelCount() const = 0;
        virtual void setBuffer(span<TColor> buffer) = 0;
    };



    template <typename TColor>
    class I2dPixelBus : public IPixelBus<TColor>
    {
    public:
        virtual ~I2dPixelBus() = default;

        virtual const Topology& topology() const = 0;
    };

} // namespace lw
