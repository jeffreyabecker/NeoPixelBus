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
template <
    typename TColor, typename TIndexRange, typename TOutputRange,
    typename = std::enable_if_t<ColorType<TColor> && IsBeginEndRange<std::remove_reference_t<TIndexRange>>::value &&
                                IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
size_t samplePalette(const IPalette<TColor>& palette, TIndexRange&& paletteIndexes, TOutputRange&& outputColors,
                     PaletteSampleOptions<TColor> options = {})
{
    if (options.blendMode == BlendMode::Nearest)
    {
        return sampleNearest<TColor>(palette, std::forward<TIndexRange>(paletteIndexes),
                                     std::forward<TOutputRange>(outputColors), options);
    }

    return sampleInterpolated<TColor>(palette, std::forward<TIndexRange>(paletteIndexes),
                                      std::forward<TOutputRange>(outputColors), options);
}

template <
    typename TColor, typename TOutputRange,
    typename = std::enable_if_t<ColorType<TColor> && IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
size_t samplePalette(const IPalette<TColor>& palette, size_t paletteIndex, TOutputRange&& outputColors,
                     PaletteSampleOptions<TColor> options = {})
{
    const size_t outputCount = static_cast<size_t>(std::distance(outputColors.begin(), outputColors.end()));
    IndexRange paletteIndexes(paletteIndex, static_cast<size_t>(1), outputCount);
    return samplePalette(palette, paletteIndexes, std::forward<TOutputRange>(outputColors), options);
}

template <
    typename TColor, typename TIndexRange, typename TOutputRange,
    typename = std::enable_if_t<ColorType<TColor> && IsBeginEndRange<std::remove_reference_t<TIndexRange>>::value &&
                                IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
size_t samplePalette(const IPalette<TColor>& paletteFrom, const IPalette<TColor>& paletteTo,
                     TIndexRange&& paletteIndexes, TOutputRange&& outputColors, uint8_t blendProgress8,
                     PaletteSampleOptions<TColor> options = {})
{
    const size_t writtenFrom = samplePalette(paletteFrom, paletteIndexes, outputColors, options);

    samplingtransition::BlendOutputRange<TColor, TOutputRange> blendedOutput(outputColors, blendProgress8);
    const size_t writtenTo = samplePalette(paletteTo, paletteIndexes, blendedOutput, options);
    return (writtenFrom < writtenTo) ? writtenFrom : writtenTo;
}

template <
    typename TColor, typename TOutputRange,
    typename = std::enable_if_t<ColorType<TColor> && IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
size_t samplePalette(const IPalette<TColor>& paletteFrom, const IPalette<TColor>& paletteTo, size_t firstPaletteIndex,
                     TOutputRange&& outputColors, uint8_t blendProgress8, PaletteSampleOptions<TColor> options = {})
{
    const size_t outputCount = static_cast<size_t>(std::distance(outputColors.begin(), outputColors.end()));
    IndexRange paletteIndexes(firstPaletteIndex, static_cast<size_t>(1), outputCount);
    return samplePalette(paletteFrom, paletteTo, paletteIndexes, std::forward<TOutputRange>(outputColors),
                         blendProgress8, options);
}

template <
    typename TColor, typename TIndexRange, typename TOutputRange,
    typename = std::enable_if_t<ColorType<TColor> && IsBeginEndRange<std::remove_reference_t<TIndexRange>>::value &&
                                IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
size_t samplePalette(const IPalette<TColor>& paletteFrom, const IPalette<TColor>& paletteTo,
                     TIndexRange&& paletteIndexes, TOutputRange&& outputColors, uint8_t transitionProgress,
                     uint8_t transitionDuration, PaletteSampleOptions<TColor> options = {})
{
    return samplePalette(paletteFrom, paletteTo, paletteIndexes, outputColors,
                         mapTransitionProgressToBlend8(transitionProgress, transitionDuration), options);
}

} // namespace lw::colors::palettes
