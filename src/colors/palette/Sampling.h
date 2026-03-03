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

    constexpr uint8_t mapTransitionProgressToBlend(size_t transitionProgress,
                                                   size_t transitionDuration)
    {
        if (transitionDuration == 0)
        {
            return 255;
        }

        const size_t clamped = (transitionProgress >= transitionDuration)
                                   ? transitionDuration
                                   : transitionProgress;
        return static_cast<uint8_t>((clamped * 255ull) / transitionDuration);
    }

    template <typename TWrap = WrapClamp,
              typename TBlend = BlendLinearContiguous<TWrap>,
              typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr TColor samplePaletteAtPosition(span<const PaletteStop<TColor>> stops,
                                             size_t pixelIndex,
                                             size_t pixelCount,
                                             PaletteSampleOptions<TColor> options = {})
    {
        const uint8_t paletteIndex = mapPositionToPaletteIndex<TWrap>(pixelIndex, pixelCount);
        TColor output{};
        IndexIterator indexBegin(paletteIndex, 0, 1);
        TBlend::template samplePalette<TColor>(stops,
                                               indexBegin,
                                               IndexSentinel{},
                                               span<TColor>(&output, 1),
                                               options);
        return output;
    }

    template <typename TWrap = WrapClamp,
              typename TBlend = BlendLinearContiguous<TWrap>,
              typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr size_t samplePaletteByPosition(span<const PaletteStop<TColor>> stops,
                                             size_t firstPixelIndex,
                                             size_t pixelStep,
                                             size_t pixelCount,
                                             span<TColor> outputColors,
                                             PaletteSampleOptions<TColor> options = {})
    {
        size_t written = 0;

        for (size_t i = 0; i < outputColors.size(); ++i)
        {
            const size_t pixelIndex = firstPixelIndex + (i * pixelStep);
            outputColors[i] = samplePaletteAtPosition<TWrap, TBlend>(stops,
                                                                      pixelIndex,
                                                                      pixelCount,
                                                                      options);
            ++written;
        }

        return written;
    }

    template <typename TWrap = WrapClamp,
              typename TBlend = BlendLinearContiguous<TWrap>,
              typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr size_t samplePaletteByPosition(span<const PaletteStop<TColor>> stops,
                                             size_t firstPixelIndex,
                                             size_t pixelCount,
                                             span<TColor> outputColors,
                                             PaletteSampleOptions<TColor> options = {})
    {
        return samplePaletteByPosition<TWrap, TBlend>(stops,
                                                      firstPixelIndex,
                                                      static_cast<size_t>(1),
                                                      pixelCount,
                                                      outputColors,
                                                      options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr TColor samplePaletteTransition(span<const PaletteStop<TColor>> fromStops,
                                             span<const PaletteStop<TColor>> toStops,
                                             uint8_t paletteIndex,
                                             size_t transitionProgress,
                                             size_t transitionDuration,
                                             PaletteSampleOptions<TColor> options = {})
    {
        const TColor fromColor = samplePaletteAtPosition<WrapClamp, TBlend>(fromStops,
                                                                             paletteIndex,
                                                                             256,
                                                                             options);
        const TColor toColor = samplePaletteAtPosition<WrapClamp, TBlend>(toStops,
                                                                           paletteIndex,
                                                                           256,
                                                                           options);
        const uint8_t blendProgress = mapTransitionProgressToBlend(transitionProgress,
                                                                   transitionDuration);
        return linearBlend(fromColor,
                           toColor,
                           blendProgress);
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

    template <typename TWrap = WrapClamp,
              typename TBlend = BlendLinearContiguous<TWrap>,
              typename TPaletteLike,
              typename = EnableIfPaletteLike<TPaletteLike>>
    constexpr typename TPaletteLike::StopType::ColorType samplePaletteAtPosition(
        const TPaletteLike &palette,
        size_t pixelIndex,
        size_t pixelCount,
        PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        using Stop = typename TPaletteLike::StopType;
        using Color = typename Stop::ColorType;
        return samplePaletteAtPosition<TWrap, TBlend, Color>(palette.stops(),
                                                             pixelIndex,
                                                             pixelCount,
                                                             options);
    }

    template <typename TWrap = WrapClamp,
              typename TBlend = BlendLinearContiguous<TWrap>,
              typename TPaletteLike,
              typename = EnableIfPaletteLike<TPaletteLike>>
    constexpr size_t samplePaletteByPosition(
        const TPaletteLike &palette,
        size_t firstPixelIndex,
        size_t pixelStep,
        size_t pixelCount,
        span<typename TPaletteLike::StopType::ColorType> outputColors,
        PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        using Stop = typename TPaletteLike::StopType;
        using Color = typename Stop::ColorType;
        return samplePaletteByPosition<TWrap, TBlend, Color>(palette.stops(),
                                                             firstPixelIndex,
                                                             pixelStep,
                                                             pixelCount,
                                                             outputColors,
                                                             options);
    }

    template <typename TWrap = WrapClamp,
              typename TBlend = BlendLinearContiguous<TWrap>,
              typename TPaletteLike,
              typename = EnableIfPaletteLike<TPaletteLike>>
    constexpr size_t samplePaletteByPosition(
        const TPaletteLike &palette,
        size_t firstPixelIndex,
        size_t pixelCount,
        span<typename TPaletteLike::StopType::ColorType> outputColors,
        PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        using Stop = typename TPaletteLike::StopType;
        using Color = typename Stop::ColorType;
        return samplePaletteByPosition<TWrap, TBlend, Color>(palette.stops(),
                                                             firstPixelIndex,
                                                             pixelCount,
                                                             outputColors,
                                                             options);
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
        return samplePalette<TBlend, Color>(palette.stops(),
                                            paletteIndex,
                                            options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TPaletteLike,
              typename = EnableIfPaletteLike<TPaletteLike>>
    constexpr typename TPaletteLike::StopType::ColorType samplePaletteTransition(
        const TPaletteLike &fromPalette,
        const TPaletteLike &toPalette,
        uint8_t paletteIndex,
        size_t transitionProgress,
        size_t transitionDuration,
        PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        using Stop = typename TPaletteLike::StopType;
        using Color = typename Stop::ColorType;
        return samplePaletteTransition<TBlend, Color>(fromPalette.stops(),
                                                      toPalette.stops(),
                                                      paletteIndex,
                                                      transitionProgress,
                                                      transitionDuration,
                                                      options);
    }

} // namespace lw
