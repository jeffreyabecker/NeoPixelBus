#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "colors/ColorMath.h"
#include "colors/palette/Sampling.h"

namespace lw
{
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
        return samplePalette<TBlend>(stops,
                                     paletteIndex,
                                     options);
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

            if constexpr (std::is_same<TWrap, WrapBlackout>::value)
            {
                if (WrapBlackout::isOutOfRange(pixelIndex, pixelCount))
                {
                    outputColors[i] = TColor{};
                    ++written;
                    continue;
                }
            }

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
        const TColor fromColor = samplePalette<TBlend>(fromStops,
                                                       paletteIndex,
                                                       options);
        const TColor toColor = samplePalette<TBlend>(toStops,
                                                     paletteIndex,
                                                     options);
        const uint8_t blendProgress = mapTransitionProgressToBlend(transitionProgress,
                                                                   transitionDuration);
        return linearBlend(fromColor,
                           toColor,
                           blendProgress);
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
