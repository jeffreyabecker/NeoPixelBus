#pragma once

#include <cstddef>
#include <span>

#include "colors/Color.h"

namespace npb
{

    class IPixelBus
    {
    public:
        virtual ~IPixelBus() = default;

        virtual void begin() = 0;
        virtual void show() = 0;
        virtual bool canShow() const = 0;

        virtual size_t pixelCount() const = 0;
        virtual std::span<Color> colors() = 0;
        virtual std::span<const Color> colors() const = 0;

        virtual void setPixelColors(size_t offset,
                                    std::span<const Color> pixelData) = 0;
        virtual void getPixelColors(size_t offset,
                                    std::span<Color> pixelData) const = 0;
        // virtual Color getPixelColor(size_t offset) const
        // {
        //     Color result{};
        //     getPixelColors(offset, {&result, 1});
        //     return result;
        // }
        // virtual void setPixelColor(size_t offset, const Color &color)
        // {
        //     setPixelColors(offset, {&color, 1});
        // }
    };

} // namespace npb
