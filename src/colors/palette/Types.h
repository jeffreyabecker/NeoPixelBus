#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "core/Compat.h"
#include "colors/Color.h"
#include "colors/palette/WrapModes.h"

namespace lw
{
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
        using ColorType = TColor;

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
