#pragma once

#include <array>
#include <cstddef>
#include <iterator>
#include <type_traits>

#include "core/IndexIterator.h"
#include "colors/palette/Blends.h"
#include "colors/palette/Traits.h"
#include "colors/palette/WrappedPaletteIndexes.h"

namespace lw
{
    template <typename TBlend = BlendLinearContiguous<>,
              typename TPaletteLike,
              typename TIndexRange,
              typename TOutputRange,
              typename = std::enable_if_t<IsPaletteLike<TPaletteLike>::value &&
                                          IsBeginEndRange<std::remove_reference_t<TIndexRange>>::value &&
                                          IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
    constexpr size_t samplePalette(const TPaletteLike &palette,
                                   TIndexRange &&paletteIndexes,
                                   TOutputRange &&outputColors,
                                   PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        using Stop = typename TPaletteLike::StopType;
        using Color = typename Stop::ColorType;
        return TBlend::template samplePalette<Color>(palette.stops(),
                                                     paletteIndexes.begin(),
                                                     paletteIndexes.end(),
                                                     outputColors.begin(),
                                                     outputColors.end(),
                                                     options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TPaletteLike,
              typename TOutputRange,
              typename = std::enable_if_t<IsPaletteLike<TPaletteLike>::value && IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
    constexpr size_t samplePalette(const TPaletteLike &palette,
                                   size_t firstPaletteIndex,
                                   size_t paletteIndexStep,
                                   TOutputRange &&outputColors,
                                   PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        const size_t outputCount = static_cast<size_t>(std::distance(outputColors.begin(), outputColors.end()));
        IndexRange paletteIndexes(firstPaletteIndex,
                                  paletteIndexStep,
                                  outputCount);

        return samplePalette<TBlend>(palette,
                                     paletteIndexes,
                                     outputColors,
                                     options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TPaletteLike,
              typename TOutputRange,
              typename = std::enable_if_t<IsPaletteLike<TPaletteLike>::value && IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
    constexpr size_t samplePalette(const TPaletteLike &palette,
                                                                     size_t firstPaletteIndex,
                                   TOutputRange &&outputColors,
                                   PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        return samplePalette<TBlend>(palette,
                                     firstPaletteIndex,
                                                                         static_cast<size_t>(1),
                                     outputColors,
                                     options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TPaletteLike,
              typename = EnableIfPaletteLike<TPaletteLike>>
    constexpr typename TPaletteLike::StopType::ColorType samplePalette(
        const TPaletteLike &palette,
        size_t paletteIndex,
        PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        using Stop = typename TPaletteLike::StopType;
        using Color = typename Stop::ColorType;

        std::array<Color, 1> sampled{};
        samplePalette<TBlend>(palette,
                              paletteIndex,
                              static_cast<size_t>(1),
                              sampled,
                              options);
        return sampled[0];
    }

} // namespace lw
