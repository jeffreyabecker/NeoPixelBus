#pragma once

#include <cstddef>
#include <iterator>

#include "core/IndexIterator.h"
#include "colors/palette/WrapModes.h"

namespace lw::colors::palettes
{
    template <typename TWrap = WrapClamp>
    class WrappedPaletteIndexes
    {
    public:
        class Iterator
        {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = size_t;
            using difference_type = std::ptrdiff_t;
            using reference = size_t;
            using pointer = void;

            constexpr Iterator(size_t positionStart,
                               size_t positionStep,
                               size_t remaining,
                                                             size_t positionCount,
                                                             size_t maxPaletteIndex)
                : _position(positionStart),
                  _positionStep(positionStep),
                  _remaining(remaining),
                                    _positionCount(positionCount),
                                    _maxPaletteIndex(maxPaletteIndex)
            {
            }

            constexpr value_type operator*() const
            {
                return TWrap::mapPositionToPaletteIndex(_position,
                                                        _positionCount,
                                                        _maxPaletteIndex);
            }

            constexpr size_t currentPosition() const
            {
                return _position;
            }

            constexpr size_t positionCount() const
            {
                return _positionCount;
            }

            constexpr Iterator &operator++()
            {
                _position += _positionStep;
                if (_remaining > 0)
                {
                    --_remaining;
                }
                return *this;
            }

            constexpr Iterator operator++(int)
            {
                Iterator copy = *this;
                ++(*this);
                return copy;
            }

            friend constexpr bool operator==(const Iterator &it,
                                             IndexSentinel)
            {
                return it._remaining == 0;
            }

            friend constexpr bool operator!=(const Iterator &it,
                                             IndexSentinel sentinel)
            {
                return !(it == sentinel);
            }

        private:
            size_t _position;
            size_t _positionStep;
            size_t _remaining;
            size_t _positionCount;
            size_t _maxPaletteIndex;
        };

        constexpr WrappedPaletteIndexes(size_t firstPosition,
                                        size_t positionStep,
                                        size_t positionCount,
                                size_t sampleCount,
                                size_t maxPaletteIndex)
            : _firstPosition(firstPosition),
              _positionStep(positionStep),
              _positionCount(positionCount),
              _sampleCount(sampleCount),
              _maxPaletteIndex(maxPaletteIndex)
        {
        }

        constexpr Iterator begin() const
        {
            return Iterator(_firstPosition,
                            _positionStep,
                            _sampleCount,
                            _positionCount,
                            _maxPaletteIndex);
        }

        constexpr IndexSentinel end() const
        {
            return IndexSentinel{};
        }

    private:
        size_t _firstPosition;
        size_t _positionStep;
        size_t _positionCount;
        size_t _sampleCount;
        size_t _maxPaletteIndex;
    };

} // namespace lw::colors::palettes
