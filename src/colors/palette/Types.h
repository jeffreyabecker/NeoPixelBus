#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "core/Compat.h"
#include "colors/Color.h"

namespace lw
{
    struct WrapClamp
    {
        static constexpr uint16_t distance(uint8_t left, uint8_t right)
        {
            return (left >= right)
                       ? static_cast<uint16_t>(left - right)
                       : static_cast<uint16_t>(right - left);
        }

        static constexpr uint8_t mapPositionToPaletteIndex(size_t pixelIndex,
                                                           size_t pixelCount)
        {
            if (pixelCount == 0)
            {
                return 0;
            }

            if (pixelCount <= 1)
            {
                return 0;
            }

            const size_t clamped = (pixelIndex >= pixelCount) ? (pixelCount - 1) : pixelIndex;
            return static_cast<uint8_t>((clamped * 255ull) / (pixelCount - 1));
        }
    };

    struct WrapCircular
    {
        static constexpr uint16_t distance(uint8_t left, uint8_t right)
        {
            const uint16_t direct = WrapClamp::distance(left, right);
            return std::min<uint16_t>(direct, static_cast<uint16_t>(256 - direct));
        }

        static constexpr uint8_t mapPositionToPaletteIndex(size_t pixelIndex,
                                                           size_t pixelCount)
        {
            if (pixelCount == 0)
            {
                return 0;
            }

            const size_t wrapped = pixelIndex % pixelCount;
            return static_cast<uint8_t>((wrapped * 256ull) / pixelCount);
        }
    };

    template <typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    struct PaletteSampleOptions
    {
        typename TColor::ComponentType brightnessScale{std::numeric_limits<typename TColor::ComponentType>::max()};
    };

    template <typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    struct PaletteStop
    {
        uint8_t index{0};
        TColor color{};
    };

    template <typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    class Palette
    {
    public:
        using StopType = PaletteStop<TColor>;

        constexpr Palette() = default;

        constexpr explicit Palette(span<const StopType> stops)
            : _stops(stops)
        {
        }

        constexpr span<const StopType> stops() const
        {
            return _stops;
        }

        constexpr bool empty() const
        {
            return _stops.empty();
        }

        constexpr size_t size() const
        {
            return _stops.size();
        }

    private:
        span<const StopType> _stops{};
    };

} // namespace lw
