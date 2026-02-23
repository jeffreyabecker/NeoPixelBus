#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <span>

#if __cplusplus >= 202002L
#include <compare>
#endif

#include "Color.h"

namespace npb
{

    // -----------------------------------------------------------------------
    // ColorIterator – wraps a std::function<Color&(uint16_t)> + position
    //
    // Satisfies std::random_access_iterator.  The accessor returns a mutable
    // Color& so the same iterator type works for both reading (input) and
    // writing (output) pixel data.
    //
    // The iterator does not track its own bounds — that is the range's job.
    // Source helpers (SolidColorSource, SpanColorSource) are proper ranges
    // providing begin()/end() pairs that work with STL algorithms.
    //
    // Usage:
    //   SolidColorSource fill{black, pixelCount};
    //   std::copy(fill.begin(), fill.end(), dest);
    //
    //   SpanColorSource src{mySpan};
    //   bus.setPixelColors(0, src.begin(), src.end());
    //
    //   // Subspan arithmetic:
    //   auto it = src.begin() + 10;   // skip first 10 pixels
    //
    //   // Ad-hoc lambda:
    //   std::vector<Color> buf(100);
    //   ColorIterator begin{[&](uint16_t i) -> Color& { return buf[i]; }, 0};
    //   ColorIterator end  {[&](uint16_t i) -> Color& { return buf[i]; }, 100};
    // -----------------------------------------------------------------------
    class ColorIterator
    {
    public:
        using AccessorFn = std::function<Color &(uint16_t idx)>;

        using iterator_category = std::random_access_iterator_tag;
        using value_type = Color;
        using difference_type = std::ptrdiff_t;
        using reference = Color &;

#if __cplusplus >= 202002L
        using iterator_concept = std::random_access_iterator_tag;
#endif

        // Default-constructed iterators compare equal (past-the-end)
        ColorIterator() = default;

        ColorIterator(AccessorFn accessor, uint16_t position)
            : _accessor(std::move(accessor)), _position(position)
        {
        }

        // Copyable / movable
        ColorIterator(const ColorIterator &) = default;
        ColorIterator &operator=(const ColorIterator &) = default;
        ColorIterator(ColorIterator &&) = default;
        ColorIterator &operator=(ColorIterator &&) = default;

        // -- Dereference ---------------------------------------------------

        Color &operator*() const
        {
            return _accessor(_position);
        }

        Color &operator[](difference_type n) const
        {
            return _accessor(static_cast<uint16_t>(_position + n));
        }

        // -- Increment / decrement -----------------------------------------

        ColorIterator &operator++()
        {
            ++_position;
            return *this;
        }

        ColorIterator operator++(int)
        {
            auto tmp = *this;
            ++_position;
            return tmp;
        }

        ColorIterator &operator--()
        {
            --_position;
            return *this;
        }

        ColorIterator operator--(int)
        {
            auto tmp = *this;
            --_position;
            return tmp;
        }

        // -- Compound assignment -------------------------------------------

        ColorIterator &operator+=(difference_type n)
        {
            _position = static_cast<uint16_t>(_position + n);
            return *this;
        }

        ColorIterator &operator-=(difference_type n)
        {
            _position = static_cast<uint16_t>(_position - n);
            return *this;
        }

        // -- Arithmetic ----------------------------------------------------

        friend ColorIterator operator+(ColorIterator it, difference_type n)
        {
            it += n;
            return it;
        }

        friend ColorIterator operator+(difference_type n, ColorIterator it)
        {
            it += n;
            return it;
        }

        friend ColorIterator operator-(ColorIterator it, difference_type n)
        {
            it -= n;
            return it;
        }

        friend difference_type operator-(const ColorIterator &a,
                                         const ColorIterator &b)
        {
            return static_cast<difference_type>(a._position) -
                   static_cast<difference_type>(b._position);
        }

        // -- Comparison ----------------------------------------------------

        friend bool operator==(const ColorIterator &a,
                               const ColorIterator &b)
        {
            return a._position == b._position;
        }

#if __cplusplus >= 202002L
        friend auto operator<=>(const ColorIterator &a,
                                const ColorIterator &b)
        {
            return a._position <=> b._position;
        }
#else
        friend bool operator!=(const ColorIterator &a,
                               const ColorIterator &b)
        {
            return !(a == b);
        }

        friend bool operator<(const ColorIterator &a,
                              const ColorIterator &b)
        {
            return a._position < b._position;
        }

        friend bool operator<=(const ColorIterator &a,
                               const ColorIterator &b)
        {
            return a._position <= b._position;
        }

        friend bool operator>(const ColorIterator &a,
                              const ColorIterator &b)
        {
            return a._position > b._position;
        }

        friend bool operator>=(const ColorIterator &a,
                               const ColorIterator &b)
        {
            return a._position >= b._position;
        }
#endif

        // -- Observers -----------------------------------------------------

        uint16_t position() const
        {
            return _position;
        }

    private:
        AccessorFn _accessor;
        uint16_t _position{0};
    };

    // -----------------------------------------------------------------------
    // SolidColorSource – range that yields a constant color for N pixels
    //
    // Usage:
    //   SolidColorSource fill{Color{}, 100};
    //   bus.setPixelColors(0, fill.begin(), fill.end());
    //   std::copy(fill.begin(), fill.end(), dest);
    // -----------------------------------------------------------------------
    struct SolidColorSource
    {
        Color color;
        uint16_t pixelCount;

        ColorIterator begin()
        {
            return ColorIterator{
                [this](uint16_t) -> Color &
                { return color; },
                0};
        }

        ColorIterator end()
        {
            return ColorIterator{
                [this](uint16_t) -> Color &
                { return color; },
                pixelCount};
        }
    };

    using FillColorSource = SolidColorSource;

    // -----------------------------------------------------------------------
    // SpanColorSource – range that iterates over a std::span<Color>
    //
    // Because the reference is mutable, the same source can be used with
    // both setPixelColors and getPixelColors.
    //
    // Usage:
    //   SpanColorSource src{myColors};
    //   bus.setPixelColors(destOffset, src.begin(), src.end());
    //   std::copy(src.begin(), src.end(), dest);
    // -----------------------------------------------------------------------
    struct SpanColorSource
    {
        std::span<Color> data;
        SpanColorSource() = default;

        explicit SpanColorSource(std::span<Color> span)
            : data(span)
        {
        }

        SpanColorSource(Color *ptr, size_t size)
            : data(ptr, size)
        {
        }
        ColorIterator begin()
        {
            return ColorIterator{
                [this](uint16_t idx) -> Color &
                { return data[idx]; },
                0};
        }

        ColorIterator end()
        {
            return ColorIterator{
                [this](uint16_t idx) -> Color &
                { return data[idx]; },
                static_cast<uint16_t>(data.size())};
        }
    };

} // namespace npb
