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
              typename TColor,
              typename TIndexIt,
              typename TIndexSentinel,
              typename TOutputIt,
              typename TSentinel,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                   TIndexIt index,
                                   TIndexSentinel indexEnd,
                                   TOutputIt output,
                                   TSentinel outputEnd,
                                   PaletteSampleOptions<TColor> options = {})
    {
        return TBlend::template samplePalette<TColor>(stops,
                                                      index,
                                                      indexEnd,
                                                      output,
                                                      outputEnd,
                                                      options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TColor,
              typename TIndexIt,
              typename TIndexSentinel,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                   TIndexIt index,
                                   TIndexSentinel indexEnd,
                                   span<TColor> outputColors,
                                   PaletteSampleOptions<TColor> options = {})
    {
        return TBlend::template samplePalette<TColor>(stops,
                                                      index,
                                                      indexEnd,
                                                      outputColors,
                                                      options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TColor,
              typename TOutputIt,
              typename TSentinel,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                   span<const uint8_t> paletteIndexes,
                                   TOutputIt output,
                                   TSentinel outputEnd,
                                   PaletteSampleOptions<TColor> options = {})
    {
        return samplePalette<TBlend>(stops,
                                     paletteIndexes.begin(),
                                     paletteIndexes.end(),
                                     output,
                                     outputEnd,
                                     options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                   span<const uint8_t> paletteIndexes,
                                   span<TColor> outputColors,
                                   PaletteSampleOptions<TColor> options = {})
    {
        return samplePalette<TBlend>(stops,
                                     paletteIndexes.begin(),
                                     paletteIndexes.end(),
                                     outputColors,
                                     options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TColor,
              typename TOutputIt,
              typename TSentinel,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                   uint8_t firstPaletteIndex,
                                   uint8_t paletteIndexStep,
                                   TOutputIt output,
                                   TSentinel outputEnd,
                                   PaletteSampleOptions<TColor> options = {})
    {
        IndexIterator indexBegin(firstPaletteIndex,
                                 paletteIndexStep,
                                 std::numeric_limits<size_t>::max());
        return samplePalette<TBlend>(stops,
                                     indexBegin,
                                     IndexSentinel{},
                                     output,
                                     outputEnd,
                                     options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                   uint8_t firstPaletteIndex,
                                   uint8_t paletteIndexStep,
                                   span<TColor> outputColors,
                                   PaletteSampleOptions<TColor> options = {})
    {
        IndexIterator indexBegin(firstPaletteIndex, paletteIndexStep, outputColors.size());
        return samplePalette<TBlend>(stops,
                                     indexBegin,
                                     IndexSentinel{},
                                     outputColors,
                                     options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                   uint8_t firstPaletteIndex,
                                   span<TColor> outputColors,
                                   PaletteSampleOptions<TColor> options = {})
    {
        return samplePalette<TBlend>(stops,
                                     firstPaletteIndex,
                                     static_cast<uint8_t>(1),
                                     outputColors,
                                     options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr TColor samplePalette(span<const PaletteStop<TColor>> stops,
                                   uint8_t paletteIndex,
                                   PaletteSampleOptions<TColor> options = {})
    {
        TColor output{};
        samplePalette<TBlend>(stops,
                              paletteIndex,
                              static_cast<uint8_t>(0),
                              span<TColor>(&output, 1),
                              options);
        return output;
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
                                   TSentinel outputEnd)
    {
        using Stop = typename TPaletteLike::StopType;
        using Color = typename Stop::ColorType;
        return samplePalette<TBlend, Color>(palette.stops(),
                                            index,
                                            indexEnd,
                                            output,
                                            outputEnd,
                                            PaletteSampleOptions<Color>{});
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TColor,
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
                                   PaletteSampleOptions<TColor> options)
    {
        return samplePalette<TBlend, TColor>(palette.stops(),
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
                                   span<typename TPaletteLike::StopType::ColorType> outputColors)
    {
        using Stop = typename TPaletteLike::StopType;
        using Color = typename Stop::ColorType;
        return samplePalette<TBlend, Color>(palette.stops(),
                                            index,
                                            indexEnd,
                                            outputColors,
                                            PaletteSampleOptions<Color>{});
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TColor,
              typename TPaletteLike,
              typename TIndexIt,
              typename TIndexSentinel,
              typename = EnableIfPaletteLike<TPaletteLike>>
    constexpr size_t samplePalette(const TPaletteLike &palette,
                                   TIndexIt index,
                                   TIndexSentinel indexEnd,
                                   span<TColor> outputColors,
                                   PaletteSampleOptions<TColor> options)
    {
        return samplePalette<TBlend, TColor>(palette.stops(),
                                                      index,
                                                      indexEnd,
                                                      outputColors,
                                                      options);
    }

} // namespace lw
