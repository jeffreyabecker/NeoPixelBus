#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
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

    static constexpr PaletteStop fromRgb8(size_t index, uint32_t rgb)
    {
        return fromRgb8(index, static_cast<uint8_t>((rgb >> 16U) & 0xFFU), static_cast<uint8_t>((rgb >> 8U) & 0xFFU),
                        static_cast<uint8_t>(rgb & 0xFFU));
    }

    static constexpr PaletteStop fromRgb8(size_t index, uint8_t r, uint8_t g, uint8_t b)
    {
        return PaletteStop{index, TColor{r, g, b}};
    }

    size_t index{0};
    TColor color{};
};

template <typename TColor, typename = std::enable_if_t<ColorType<TColor>>> class Palette
{
  public:
    using StopType = PaletteStop<TColor>;

    constexpr Palette() = default;

    constexpr explicit Palette(span<const StopType> stops) : _stops(stops), _maxIndex(computeMaxIndex(stops)) {}

    template <size_t N>
    constexpr explicit Palette(const std::array<StopType, N>& stops)
        : _stops(stops.data(), stops.size()), _maxIndex(computeMaxIndex(_stops))
    {
    }

    constexpr span<const StopType> stops() const { return _stops; }

    constexpr bool empty() const { return _stops.empty(); }

    constexpr size_t size() const { return _stops.size(); }

    constexpr auto begin() const { return _stops.begin(); }

    constexpr auto end() const { return _stops.end(); }

    constexpr size_t maxIndex() const { return _maxIndex; }

    static constexpr Palette Default() { return Palette(); }

    static constexpr Palette Color1(const TColor& primary)
    {
        const std::array<StopType, 2> stops = {
            StopType{0, primary},
            StopType{255, primary},
        };
        return Palette(stops);
    }

    static constexpr Palette Colors1And2(const TColor& primary, const TColor& secondary)
    {
        const std::array<StopType, 4> stops = {
            StopType{0, primary},
            StopType{127, primary},
            StopType{128, secondary},
            StopType{255, secondary},
        };
        return Palette(stops);
    }

    static constexpr Palette ColorGradient(const TColor& primary, const TColor& secondary, const TColor& tertiary)
    {
        const std::array<StopType, 3> stops = {
            StopType{0, tertiary},
            StopType{127, secondary},
            StopType{255, primary},
        };
        return Palette(stops);
    }

    static constexpr Palette ColorsOnly(const TColor& primary, const TColor& secondary, const TColor& tertiary)
    {
        const std::array<StopType, 16> stops = {
            StopType{0, primary},     StopType{16, primary},    StopType{32, primary},   StopType{48, primary},
            StopType{64, primary},    StopType{80, secondary},  StopType{96, secondary}, StopType{112, secondary},
            StopType{128, secondary}, StopType{144, secondary}, StopType{160, tertiary}, StopType{176, tertiary},
            StopType{192, tertiary},  StopType{208, tertiary},  StopType{224, tertiary}, StopType{255, primary},
        };
        return Palette(stops);
    }

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
