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
template <typename TBlend = BlendLinearContiguous, typename TWrap = WrapClamp, typename TPaletteLike,
          typename TIndexRange, typename TOutputRange,
          typename = std::enable_if_t<IsPaletteLike<TPaletteLike>::value &&
                                      IsBeginEndRange<std::remove_reference_t<TIndexRange>>::value &&
                                      IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
size_t samplePalette(const TPaletteLike& palette, TIndexRange&& paletteIndexes, TOutputRange&& outputColors,
                     PaletteSampleOptions<typename TPaletteLike::ColorType> options = {})
{
    using TColor = typename TPaletteLike::ColorType;
    return TBlend::template samplePalette<TWrap, TColor>(static_cast<const IPalette<TColor>&>(palette),
                                                         std::forward<TIndexRange>(paletteIndexes),
                                                         std::forward<TOutputRange>(outputColors), options);
}

template <typename TBlend = BlendLinearContiguous, typename TWrap = WrapClamp, typename TPaletteLike,
          typename TOutputRange,
          typename = std::enable_if_t<IsPaletteLike<TPaletteLike>::value &&
                                      IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
size_t samplePalette(const TPaletteLike& palette, size_t paletteIndex, TOutputRange&& outputColors,
                     PaletteSampleOptions<typename TPaletteLike::ColorType> options = {})
{
    const size_t outputCount = static_cast<size_t>(std::distance(outputColors.begin(), outputColors.end()));
    IndexRange paletteIndexes(paletteIndex, static_cast<size_t>(1), outputCount);
    return samplePalette<TBlend, TWrap>(palette, paletteIndexes, std::forward<TOutputRange>(outputColors), options);
}

template <
    typename TBlend = BlendLinearContiguous, typename TWrap = WrapClamp, typename TPaletteFrom, typename TPaletteTo,
    typename TIndexRange, typename TOutputRange,
    typename = std::enable_if_t<IsPaletteLike<TPaletteFrom>::value && IsPaletteLike<TPaletteTo>::value &&
                                std::is_same<typename TPaletteFrom::ColorType, typename TPaletteTo::ColorType>::value &&
                                IsBeginEndRange<std::remove_reference_t<TIndexRange>>::value &&
                                IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
size_t samplePalette(const TPaletteFrom& paletteFrom, const TPaletteTo& paletteTo, TIndexRange&& paletteIndexes,
                     TOutputRange&& outputColors, uint8_t blendProgress8,
                     PaletteSampleOptions<typename TPaletteFrom::ColorType> options = {})
{
    using TColor = typename TPaletteFrom::ColorType;
    const auto& fromBase = static_cast<const IPalette<TColor>&>(paletteFrom);
    const auto& toBase = static_cast<const IPalette<TColor>&>(paletteTo);

    const size_t writtenFrom = samplePalette<TBlend, TWrap>(fromBase, paletteIndexes, outputColors, options);

    samplingtransition::BlendOutputRange<TColor, TOutputRange> blendedOutput(outputColors, blendProgress8);
    const size_t writtenTo = samplePalette<TBlend, TWrap>(toBase, paletteIndexes, blendedOutput, options);
    return (writtenFrom < writtenTo) ? writtenFrom : writtenTo;
}

template <
    typename TBlend = BlendLinearContiguous, typename TWrap = WrapClamp, typename TPaletteFrom, typename TPaletteTo,
    typename TOutputRange,
    typename = std::enable_if_t<IsPaletteLike<TPaletteFrom>::value && IsPaletteLike<TPaletteTo>::value &&
                                std::is_same<typename TPaletteFrom::ColorType, typename TPaletteTo::ColorType>::value &&
                                IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
size_t samplePalette(const TPaletteFrom& paletteFrom, const TPaletteTo& paletteTo, size_t firstPaletteIndex,
                     TOutputRange&& outputColors, uint8_t blendProgress8,
                     PaletteSampleOptions<typename TPaletteFrom::ColorType> options = {})
{
    const size_t outputCount = static_cast<size_t>(std::distance(outputColors.begin(), outputColors.end()));
    IndexRange paletteIndexes(firstPaletteIndex, static_cast<size_t>(1), outputCount);
    return samplePalette<TBlend, TWrap>(paletteFrom, paletteTo, paletteIndexes,
                                        std::forward<TOutputRange>(outputColors), blendProgress8, options);
}

template <
    typename TBlend = BlendLinearContiguous, typename TWrap = WrapClamp, typename TPaletteFrom, typename TPaletteTo,
    typename TIndexRange, typename TOutputRange,
    typename = std::enable_if_t<IsPaletteLike<TPaletteFrom>::value && IsPaletteLike<TPaletteTo>::value &&
                                std::is_same<typename TPaletteFrom::ColorType, typename TPaletteTo::ColorType>::value &&
                                IsBeginEndRange<std::remove_reference_t<TIndexRange>>::value &&
                                IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
size_t samplePalette(const TPaletteFrom& paletteFrom, const TPaletteTo& paletteTo, TIndexRange&& paletteIndexes,
                     TOutputRange&& outputColors, uint8_t transitionProgress, uint8_t transitionDuration,
                     PaletteSampleOptions<typename TPaletteFrom::ColorType> options = {})
{
    return samplePalette<TBlend, TWrap>(paletteFrom, paletteTo, paletteIndexes, outputColors,
                                        mapTransitionProgressToBlend8(transitionProgress, transitionDuration), options);
}

} // namespace lw::colors::palettes
