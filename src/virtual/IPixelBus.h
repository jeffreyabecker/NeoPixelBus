#pragma once

#include <cstddef>
#include <span>
#include <algorithm>

#include "colors/Color.h"
#include "colors/ColorIterator.h"

namespace npb
{

    template <typename TColor = Color>
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
                                    ColorIteratorT<TColor> first,
                                    ColorIteratorT<TColor> last) = 0;

        virtual void getPixelColors(size_t offset,
                                    ColorIteratorT<TColor> first,
                                    ColorIteratorT<TColor> last) const = 0;

        // -----------------------------------------------------------------
        // Convenience – span overloads (virtual, default delegates to
        // iterator pair; concrete buses may override for speed)
        // -----------------------------------------------------------------
        virtual void setPixelColors(size_t offset,
                                    std::span<const TColor> pixelData)
        {
            auto *ptr = const_cast<TColor *>(pixelData.data());
            std::span<TColor> mutable_span{ptr, pixelData.size()};
            SpanColorSourceT<TColor> src{mutable_span};
            setPixelColors(offset, src.begin(), src.end());
        }

        virtual void getPixelColors(size_t offset,
                                    std::span<TColor> pixelData) const
        {
            SpanColorSourceT<TColor> dest{pixelData};
            getPixelColors(offset, dest.begin(), dest.end());
        }

        // -----------------------------------------------------------------
        // Convenience – single-pixel access (virtual, default delegates
        // to iterator pair; concrete buses should override)
        // -----------------------------------------------------------------
        virtual void setPixelColor(size_t index, const TColor &color)
        {
            SolidColorSourceT<TColor> src{color, 1};
            setPixelColors(index, src.begin(), src.end());
        }

        virtual TColor getPixelColor(size_t index) const
        {
            SolidColorSourceT<TColor> dest{TColor{}, 1};
            getPixelColors(index, dest.begin(), dest.end());
            return dest.color;
        }
    };

    template <typename TColor = Color>
    class I2dPixelBus : public IPixelBus<TColor>
    {
    public:
        virtual ~I2dPixelBus() = default;

        virtual void setPixelColor(int16_t x, int16_t y, const TColor &color) = 0;
        virtual TColor getPixelColor(int16_t x, int16_t y) const = 0;
        virtual uint16_t width() const = 0;
        virtual uint16_t height() const = 0;
    };

} // namespace npb
