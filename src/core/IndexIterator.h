#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>

namespace lw
{
    struct IndexSentinel
    {
    };

    class IndexIterator
    {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = uint8_t;
        using difference_type = std::ptrdiff_t;
        using reference = uint8_t;
        using pointer = void;

        constexpr IndexIterator(uint8_t start,
                                uint8_t step,
                                size_t remaining)
            : _current(start),
              _step(step),
              _remaining(remaining)
        {
        }

        constexpr uint8_t operator*() const
        {
            return _current;
        }

        constexpr IndexIterator &operator++()
        {
            _current = static_cast<uint8_t>(_current + _step);
            if (_remaining > 0)
            {
                --_remaining;
            }
            return *this;
        }

        constexpr IndexIterator operator++(int)
        {
            IndexIterator copy = *this;
            ++(*this);
            return copy;
        }

        friend constexpr bool operator==(const IndexIterator &it,
                                         IndexSentinel)
        {
            return it._remaining == 0;
        }

        friend constexpr bool operator!=(const IndexIterator &it,
                                         IndexSentinel sentinel)
        {
            return !(it == sentinel);
        }

    private:
        uint8_t _current;
        uint8_t _step;
        size_t _remaining;
    };

} // namespace lw
