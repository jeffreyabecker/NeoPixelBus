#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>
#include <type_traits>

#include "core/Compat.h"
#include "colors/Color.h"
#include "colors/palette/WrapModes.h"

namespace lw::colors::palettes
{
template <typename TColor, typename = std::enable_if_t<ColorType<TColor>>> struct PaletteSampleOptions
{
    typename TColor::ComponentType brightnessScale{std::numeric_limits<typename TColor::ComponentType>::max()};
    TColor outOfRangeColor{};
};

template <typename TColor, typename = std::enable_if_t<ColorType<TColor>>> struct PaletteStop
{
    using ColorType = TColor;

    size_t index{0};
    TColor color{};
};

template <typename TColor, typename = std::enable_if_t<ColorType<TColor>>> class Palette
{
  public:
    using StopType = PaletteStop<TColor>;

    constexpr Palette() = default;

    constexpr explicit Palette(span<const StopType> stops) : _stops(stops), _maxIndex(computeMaxIndex(stops)) {}

    constexpr span<const StopType> stops() const { return _stops; }

    constexpr bool empty() const { return _stops.empty(); }

    constexpr size_t size() const { return _stops.size(); }

    constexpr auto begin() const { return _stops.begin(); }

    constexpr auto end() const { return _stops.end(); }

    constexpr size_t maxIndex() const { return _maxIndex; }

  private:
    static constexpr size_t computeMaxIndex(span<const StopType> stops)
    {
        size_t maxStopIndex = 0;
        for (const auto& stop : stops)
        {
            maxStopIndex = std::max(maxStopIndex, stop.index);
        }

        return maxStopIndex;
    }

    span<const StopType> _stops{};
    size_t _maxIndex{0};
};

} // namespace lw::colors::palettes
