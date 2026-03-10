#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "colors/Color.h"
#include "colors/palette/Generators.h"

namespace lw::palettes
{

namespace detail
{

template <typename TColor>
constexpr colors::palettes::PaletteStop<TColor> stop(size_t index, uint8_t r, uint8_t g, uint8_t b)
{
    return colors::palettes::PaletteStop<TColor>{index, TColor{r, g, b}};
}

template <typename TColor, size_t N>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor>
makeGenerator(const std::array<colors::palettes::PaletteStop<TColor>, N>& stops)
{
    return colors::palettes::StaticStopsPaletteGenerator<TColor>(
        span<const colors::palettes::PaletteStop<TColor>>(stops.data(), stops.size()));
}

} // namespace detail

template <typename TColor = colors::DefaultColorType> struct NamedPalette
{
    const char* name;
    colors::palettes::StaticStopsPaletteGenerator<TColor> (*create)();
};

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Default()
{
    return colors::palettes::StaticStopsPaletteGenerator<TColor>(span<const colors::palettes::PaletteStop<TColor>>());
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Color1(const TColor& primary)
{
    const std::array<colors::palettes::PaletteStop<TColor>, 2> stops = {
        colors::palettes::PaletteStop<TColor>{0, primary},
        colors::palettes::PaletteStop<TColor>{255, primary},
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor> Colors1And2(const TColor& primary,
                                                                            const TColor& secondary)
{
    const std::array<colors::palettes::PaletteStop<TColor>, 4> stops = {
        colors::palettes::PaletteStop<TColor>{0, primary},
        colors::palettes::PaletteStop<TColor>{127, primary},
        colors::palettes::PaletteStop<TColor>{128, secondary},
        colors::palettes::PaletteStop<TColor>{255, secondary},
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor>
ColorGradient(const TColor& primary, const TColor& secondary, const TColor& tertiary)
{
    const std::array<colors::palettes::PaletteStop<TColor>, 3> stops = {
        colors::palettes::PaletteStop<TColor>{0, tertiary},
        colors::palettes::PaletteStop<TColor>{127, secondary},
        colors::palettes::PaletteStop<TColor>{255, primary},
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType>
constexpr colors::palettes::StaticStopsPaletteGenerator<TColor>
ColorsOnly(const TColor& primary, const TColor& secondary, const TColor& tertiary)
{
    const std::array<colors::palettes::PaletteStop<TColor>, 16> stops = {
        colors::palettes::PaletteStop<TColor>{0, primary},     colors::palettes::PaletteStop<TColor>{16, primary},
        colors::palettes::PaletteStop<TColor>{32, primary},    colors::palettes::PaletteStop<TColor>{48, primary},
        colors::palettes::PaletteStop<TColor>{64, primary},    colors::palettes::PaletteStop<TColor>{80, secondary},
        colors::palettes::PaletteStop<TColor>{96, secondary},  colors::palettes::PaletteStop<TColor>{112, secondary},
        colors::palettes::PaletteStop<TColor>{128, secondary}, colors::palettes::PaletteStop<TColor>{144, secondary},
        colors::palettes::PaletteStop<TColor>{160, tertiary},  colors::palettes::PaletteStop<TColor>{176, tertiary},
        colors::palettes::PaletteStop<TColor>{192, tertiary},  colors::palettes::PaletteStop<TColor>{208, tertiary},
        colors::palettes::PaletteStop<TColor>{224, tertiary},  colors::palettes::PaletteStop<TColor>{255, primary},
    };
    return detail::makeGenerator(stops);
}

template <typename TColor = colors::DefaultColorType, size_t TStopCount = 8>
using RandomSmoothGenerator = colors::palettes::RandomSmoothPaletteGenerator<TColor, TStopCount>;

template <typename TColor = colors::DefaultColorType, size_t TStopCount = 8>
using RandomCycleGenerator = colors::palettes::RandomCyclePaletteGenerator<TColor, TStopCount>;

} // namespace lw::palettes