#pragma once

#include <cstddef>
#include <iterator>

namespace lw
{
    struct CoordinatePair
    {
        size_t X;
        size_t Y;
    };

    struct CoordinateSentinel
    {
    };

    class CoordinateIterator
    {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = CoordinatePair;
        using difference_type = std::ptrdiff_t;
        using reference = CoordinatePair;
        using pointer = void;

        constexpr CoordinateIterator(size_t upperLeftX,
                                     size_t upperLeftY,
                                     size_t lowerRightX,
                                     size_t lowerRightY,
                                     size_t stepX = 1,
                                     size_t stepY = 1)
            : _upperLeftX(upperLeftX),
              _lowerRightX(lowerRightX),
              _lowerRightY(lowerRightY),
              _stepX(stepX == 0 ? static_cast<size_t>(1) : stepX),
              _stepY(stepY == 0 ? static_cast<size_t>(1) : stepY),
              _currentX(upperLeftX),
              _currentY(upperLeftY),
              _remaining(calculateRemaining(upperLeftX,
                                            upperLeftY,
                                            lowerRightX,
                                            lowerRightY,
                                            _stepX,
                                            _stepY))
        {
        }

        constexpr CoordinatePair operator*() const
        {
            return CoordinatePair{_currentX, _currentY};
        }

        constexpr CoordinateIterator &operator++()
        {
            if (_remaining == 0)
            {
                return *this;
            }

            --_remaining;
            if (_remaining == 0)
            {
                return *this;
            }

            const size_t nextX = _currentX + _stepX;
            if (nextX <= _lowerRightX)
            {
                _currentX = nextX;
                return *this;
            }

            _currentX = _upperLeftX;
            _currentY += _stepY;
            return *this;
        }

        constexpr CoordinateIterator operator++(int)
        {
            CoordinateIterator copy = *this;
            ++(*this);
            return copy;
        }

        friend constexpr bool operator==(const CoordinateIterator &it,
                                         CoordinateSentinel)
        {
            return it._remaining == 0;
        }

        friend constexpr bool operator!=(const CoordinateIterator &it,
                                         CoordinateSentinel sentinel)
        {
            return !(it == sentinel);
        }

    private:
        static constexpr size_t calculateCount(size_t lower,
                                               size_t upper,
                                               size_t step)
        {
            if (lower > upper)
            {
                return 0;
            }

            return ((upper - lower) / step) + static_cast<size_t>(1);
        }

        static constexpr size_t calculateRemaining(size_t upperLeftX,
                                                   size_t upperLeftY,
                                                   size_t lowerRightX,
                                                   size_t lowerRightY,
                                                   size_t stepX,
                                                   size_t stepY)
        {
            const size_t xCount = calculateCount(upperLeftX,
                                                 lowerRightX,
                                                 stepX);
            const size_t yCount = calculateCount(upperLeftY,
                                                 lowerRightY,
                                                 stepY);
            return xCount * yCount;
        }

        size_t _upperLeftX;
        size_t _lowerRightX;
        size_t _lowerRightY;
        size_t _stepX;
        size_t _stepY;
        size_t _currentX;
        size_t _currentY;
        size_t _remaining;
    };

    class CoordinateRange
    {
    public:
        static constexpr CoordinateRange fromBounds(size_t upperLeftX,
                                                    size_t upperLeftY,
                                                    size_t lowerRightX,
                                                    size_t lowerRightY)
        {
            return fromBoundsStep(upperLeftX,
                                  upperLeftY,
                                  lowerRightX,
                                  lowerRightY,
                                  1,
                                  1);
        }

        static constexpr CoordinateRange fromBoundsStep(size_t upperLeftX,
                                                        size_t upperLeftY,
                                                        size_t lowerRightX,
                                                        size_t lowerRightY,
                                                        size_t stepX,
                                                        size_t stepY)
        {
            return CoordinateRange(CoordinateIterator(upperLeftX,
                                                      upperLeftY,
                                                      lowerRightX,
                                                      lowerRightY,
                                                      stepX,
                                                      stepY));
        }

        static constexpr CoordinateRange fromOriginSize(size_t width,
                                                        size_t height)
        {
            return fromOriginSizeStep(width,
                                      height,
                                      1,
                                      1);
        }

        static constexpr CoordinateRange fromOriginSizeStep(size_t width,
                                                            size_t height,
                                                            size_t stepX,
                                                            size_t stepY)
        {
            if (width == 0 || height == 0)
            {
                return CoordinateRange(CoordinateIterator(1,
                                                          1,
                                                          0,
                                                          0,
                                                          stepX,
                                                          stepY));
            }

            return CoordinateRange(CoordinateIterator(0,
                                                      0,
                                                      width - 1,
                                                      height - 1,
                                                      stepX,
                                                      stepY));
        }

        constexpr CoordinateIterator begin() const
        {
            return _begin;
        }

        constexpr CoordinateSentinel end() const
        {
            return CoordinateSentinel{};
        }

    private:
        constexpr explicit CoordinateRange(CoordinateIterator begin)
            : _begin(begin)
        {
        }

        CoordinateIterator _begin;
    };

} // namespace lw