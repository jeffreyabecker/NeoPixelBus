#pragma once

#include <cstddef>
#include <span>
#include <algorithm>

#include "colors/Color.h"
#include "colors/ColorIterator.h"

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

        // -----------------------------------------------------------------
        // Primary interface – iterator pair (pure virtual)
        // -----------------------------------------------------------------
        virtual void setPixelColors(size_t offset,
                                    ColorIterator first,
                                    ColorIterator last) = 0;

        virtual void getPixelColors(size_t offset,
                                    ColorIterator first,
                                    ColorIterator last) const = 0;

        // -----------------------------------------------------------------
        // Convenience – span overloads (virtual, default delegates to
        // iterator pair; concrete buses may override for speed)
        // -----------------------------------------------------------------
        virtual void setPixelColors(size_t offset,
                                    std::span<const Color> pixelData)
        {
            auto* ptr = const_cast<Color*>(pixelData.data());
            std::span<Color> mutable_span{ptr, pixelData.size()};
            SpanColorSource src{mutable_span};
            setPixelColors(offset, src.begin(), src.end());
        }

        virtual void getPixelColors(size_t offset,
                                    std::span<Color> pixelData) const
        {
            SpanColorSource dest{pixelData};
            getPixelColors(offset, dest.begin(), dest.end());
        }

        // -----------------------------------------------------------------
        // Convenience – single-pixel access (virtual, default delegates
        // to iterator pair; concrete buses should override)
        // -----------------------------------------------------------------
        virtual void setPixelColor(size_t index, const Color& color)
        {
            SolidColorSource src{color, 1};
            setPixelColors(index, src.begin(), src.end());
        }

        virtual Color getPixelColor(size_t index) const
        {
            SolidColorSource dest{Color{}, 1};
            getPixelColors(index, dest.begin(), dest.end());
            return dest.color;
        }
    };

} // namespace npb
