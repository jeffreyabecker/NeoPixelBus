#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <type_traits>
#include <utility>

#include "core/IndexIterator.h"
#include "colors/palette/Blends.h"
#include "colors/palette/SamplingTransition.h"
#include "colors/palette/Traits.h"

namespace lw::colors::palettes
{
    template <typename TBlend = BlendLinearContiguous,
              typename TWrap = WrapClamp,
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
        return TBlend::template samplePalette<TWrap>(palette,
                                                     std::forward<TIndexRange>(paletteIndexes),
                                                     std::forward<TOutputRange>(outputColors),
                                                     options);
    }

    template <typename TBlend = BlendLinearContiguous,
              typename TWrap = WrapClamp,
              typename TColor,
              typename TOutputRange,
              typename = std::enable_if_t<ColorType<TColor> &&
                                          IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
    constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                   size_t firstPaletteIndex,
                                   TOutputRange &&outputColors,
                                   PaletteSampleOptions<TColor> options = {})
    {
        const Palette<TColor> palette(stops);
        const size_t outputCount = static_cast<size_t>(std::distance(outputColors.begin(), outputColors.end()));
        IndexRange paletteIndexes(firstPaletteIndex,
                                  static_cast<size_t>(1),
                                  outputCount);
        return samplePalette<TBlend, TWrap>(palette,
                                            paletteIndexes,
                                            std::forward<TOutputRange>(outputColors),
                                            options);
    }

    template <typename TBlend = BlendLinearContiguous,
              typename TWrap = WrapClamp,
              typename TPaletteLike,
              typename TIndexRange,
              typename TOutputRange,
              typename = std::enable_if_t<IsPaletteLike<TPaletteLike>::value &&
                                          IsBeginEndRange<std::remove_reference_t<TIndexRange>>::value &&
                                          IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
    constexpr size_t samplePalette(const TPaletteLike &paletteFrom,
                                   const TPaletteLike &paletteTo,
                                   TIndexRange &&paletteIndexes,
                                   TOutputRange &&outputColors,
                                   uint8_t blendProgress8,
                                   PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        using Stop = typename TPaletteLike::StopType;
        using TColor = typename Stop::ColorType;

        const size_t writtenFrom = samplePalette<TBlend, TWrap>(paletteFrom,
                                                                paletteIndexes,
                                                                outputColors,
                                                                options);

        samplingtransition::BlendOutputRange<TColor, TOutputRange> blendedOutput(outputColors,
                                                                                 blendProgress8);
        const size_t writtenTo = samplePalette<TBlend, TWrap>(paletteTo,
                                                              paletteIndexes,
                                                              blendedOutput,
                                                              options);
        return (writtenFrom < writtenTo)
                   ? writtenFrom
                   : writtenTo;
    }

    template <typename TBlend = BlendLinearContiguous,
              typename TWrap = WrapClamp,
              typename TPaletteLike,
              typename TOutputRange,
              typename = std::enable_if_t<IsPaletteLike<TPaletteLike>::value &&
                                          IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
    constexpr size_t samplePalette(const TPaletteLike &paletteFrom,
                                   const TPaletteLike &paletteTo,
                                   size_t firstPaletteIndex,
                                   TOutputRange &&outputColors,
                                   uint8_t blendProgress8,
                                   PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        const size_t outputCount = static_cast<size_t>(std::distance(outputColors.begin(), outputColors.end()));
        IndexRange paletteIndexes(firstPaletteIndex,
                                  static_cast<size_t>(1),
                                  outputCount);
        return samplePalette<TBlend, TWrap>(paletteFrom,
                                            paletteTo,
                                            paletteIndexes,
                                            std::forward<TOutputRange>(outputColors),
                                            blendProgress8,
                                            options);
    }

    template <typename TBlend = BlendLinearContiguous,
              typename TWrap = WrapClamp,
              typename TPaletteLike,
              typename TIndexRange,
              typename TOutputRange,
              typename = std::enable_if_t<IsPaletteLike<TPaletteLike>::value &&
                                          IsBeginEndRange<std::remove_reference_t<TIndexRange>>::value &&
                                          IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
    constexpr size_t samplePalette(const TPaletteLike &paletteFrom,
                                   const TPaletteLike &paletteTo,
                                   TIndexRange &&paletteIndexes,
                                   TOutputRange &&outputColors,
                                   uint8_t transitionProgress,
                                   uint8_t transitionDuration,
                                   PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        return samplePalette<TBlend, TWrap>(paletteFrom,
                                            paletteTo,
                                            paletteIndexes,
                                            outputColors,
                                            mapTransitionProgressToBlend8(transitionProgress,
                                                                          transitionDuration),
                                            options);
    }

} // namespace lw::colors::palettes
