#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "core/IndexIterator.h"
#include "colors/palette/Blends.h"
#include "colors/palette/Traits.h"

namespace lw
{
    template <typename TWrap = WrapClamp>
    constexpr uint8_t mapPositionToPaletteIndex(size_t pixelIndex,
                                                size_t pixelCount)
    {
        return TWrap::mapPositionToPaletteIndex(pixelIndex, pixelCount);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TPaletteLike,
              typename TIndexIt,
              typename TIndexSentinel,
              typename TOutputIt,
              typename TSentinel,
              typename = EnableIfPaletteLike<TPaletteLike>>
    constexpr size_t samplePalette(const TPaletteLike &palette,
                                   TIndexIt index,
                                   TIndexSentinel indexEnd,
                                   TOutputIt output,
                                   TSentinel outputEnd,
                                   PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        using Stop = typename TPaletteLike::StopType;
        using Color = typename Stop::ColorType;
        return TBlend::template samplePalette<Color>(palette.stops(),
                                                     index,
                                                     indexEnd,
                                                     output,
                                                     outputEnd,
                                                     options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TPaletteLike,
              typename TIndexIt,
              typename TIndexSentinel,
              typename = EnableIfPaletteLike<TPaletteLike>>
    constexpr size_t samplePalette(const TPaletteLike &palette,
                                   TIndexIt index,
                                   TIndexSentinel indexEnd,
                                   span<typename TPaletteLike::StopType::ColorType> outputColors,
                                   PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        return samplePalette<TBlend>(palette,
                                     index,
                                     indexEnd,
                                     outputColors.begin(),
                                     outputColors.end(),
                                     options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TPaletteLike,
              typename = EnableIfPaletteLike<TPaletteLike>>
    constexpr size_t samplePalette(const TPaletteLike &palette,
                                   span<const uint8_t> paletteIndexes,
                                   span<typename TPaletteLike::StopType::ColorType> outputColors,
                                   PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        return samplePalette<TBlend>(palette,
                                     paletteIndexes.begin(),
                                     paletteIndexes.end(),
                                     outputColors,
                                     options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TPaletteLike,
              typename TOutputIt,
              typename TSentinel,
              typename = EnableIfPaletteLike<TPaletteLike>>
    constexpr size_t samplePalette(const TPaletteLike &palette,
                                   uint8_t firstPaletteIndex,
                                   uint8_t paletteIndexStep,
                                   TOutputIt output,
                                   TSentinel outputEnd,
                                   PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        IndexIterator indexBegin(firstPaletteIndex,
                                 paletteIndexStep,
                                 std::numeric_limits<size_t>::max());
        return samplePalette<TBlend>(palette,
                                     indexBegin,
                                     IndexSentinel{},
                                     output,
                                     outputEnd,
                                     options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TPaletteLike,
              typename = EnableIfPaletteLike<TPaletteLike>>
    constexpr size_t samplePalette(const TPaletteLike &palette,
                                   uint8_t firstPaletteIndex,
                                   uint8_t paletteIndexStep,
                                   span<typename TPaletteLike::StopType::ColorType> outputColors,
                                   PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        IndexIterator indexBegin(firstPaletteIndex, paletteIndexStep, outputColors.size());
        return samplePalette<TBlend>(palette,
                                     indexBegin,
                                     IndexSentinel{},
                                     outputColors,
                                     options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TPaletteLike,
              typename = EnableIfPaletteLike<TPaletteLike>>
    constexpr size_t samplePalette(const TPaletteLike &palette,
                                   uint8_t firstPaletteIndex,
                                   span<typename TPaletteLike::StopType::ColorType> outputColors,
                                   PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        return samplePalette<TBlend>(palette,
                                     firstPaletteIndex,
                                     static_cast<uint8_t>(1),
                                     outputColors,
                                     options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TPaletteLike,
              typename = EnableIfPaletteLike<TPaletteLike>>
    constexpr typename TPaletteLike::StopType::ColorType samplePalette(
        const TPaletteLike &palette,
        uint8_t paletteIndex,
        PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        using Stop = typename TPaletteLike::StopType;
        using Color = typename Stop::ColorType;

        Color sampled{};
        samplePalette<TBlend>(palette,
                              paletteIndex,
                              static_cast<uint8_t>(1),
                              span<Color>(&sampled, static_cast<size_t>(1)),
                              options);
        return sampled;
    }

} // namespace lw
