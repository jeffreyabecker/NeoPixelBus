#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "colors/palette/Types.h"

namespace lw::colors::palettes
{
template <typename TRange, typename = void> struct IsBeginEndRange : std::false_type
{
};

template <typename TRange>
struct IsBeginEndRange<TRange,
                       std::void_t<decltype(std::declval<TRange&>().begin()), decltype(std::declval<TRange&>().end())>>
    : std::true_type
{
};

template <typename TPaletteLike, typename = void> struct IsPaletteLike : std::false_type
{
};

template <typename TStops, typename TStop, typename = void> struct IsPaletteStopsView : std::false_type
{
};

template <typename TStops, typename TStop>
struct IsPaletteStopsView<
    TStops, TStop,
    std::void_t<decltype(std::declval<const TStops&>().empty()), decltype(std::declval<const TStops&>().size()),
                decltype(std::declval<const TStops&>().front()), decltype(std::declval<const TStops&>().back()),
                decltype(std::declval<const TStops&>()[std::declval<size_t>()])>>
    : std::integral_constant<
          bool,
          std::is_convertible<decltype(std::declval<const TStops&>().empty()), bool>::value &&
              std::is_convertible<decltype(std::declval<const TStops&>().size()), size_t>::value &&
              std::is_convertible<decltype(std::declval<const TStops&>().front()), const TStop&>::value &&
              std::is_convertible<decltype(std::declval<const TStops&>().back()), const TStop&>::value &&
              std::is_convertible<decltype(std::declval<const TStops&>()[std::declval<size_t>()]), const TStop&>::value>
{
};

template <typename TPaletteLike>
struct IsPaletteLike<
    TPaletteLike, std::void_t<typename TPaletteLike::StopType, decltype(std::declval<const TPaletteLike&>().stops())>>
    : IsPaletteStopsView<decltype(std::declval<const TPaletteLike&>().stops()), typename TPaletteLike::StopType>
{
};

template <typename TPaletteLike> using EnableIfPaletteLike = std::enable_if_t<IsPaletteLike<TPaletteLike>::value>;

} // namespace lw::colors::palettes
