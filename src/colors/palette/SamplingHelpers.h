#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <type_traits>

#include "colors/ColorMath.h"
#include "colors/palette/BlendModes.h"
#include "colors/palette/Sampling.h"

namespace lw
{
    namespace detail::samplingmodes
    {
        template <typename TWrap,
                  typename TColor,
                  typename TOutputRange,
                  typename = std::enable_if_t<ColorType<TColor> && IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
        constexpr size_t dispatchBlendMode(PaletteBlendMode blendMode,
                                           span<const PaletteStop<TColor>> stops,
                                           size_t firstPaletteIndex,
                                           size_t paletteIndexStep,
                                           TOutputRange &&outputColors,
                                           PaletteSampleOptions<TColor> options)
        {
            const Palette<TColor> palette(stops);
            switch (blendMode)
            {
            case PaletteBlendMode::Nearest:
                return samplePalette<BlendNearestContiguous<TWrap>>(palette,
                                                                    firstPaletteIndex,
                                                                    paletteIndexStep,
                                                                    outputColors,
                                                                    options);
            case PaletteBlendMode::Step:
                return samplePalette<BlendStepContiguous<TWrap>>(palette,
                                                                 firstPaletteIndex,
                                                                 paletteIndexStep,
                                                                 outputColors,
                                                                 options);
            case PaletteBlendMode::HoldMidpoint:
                return samplePalette<BlendHoldMidpointContiguous<TWrap>>(palette,
                                                                         firstPaletteIndex,
                                                                         paletteIndexStep,
                                                                         outputColors,
                                                                         options);
            case PaletteBlendMode::Smoothstep:
                return samplePalette<BlendSmoothstepContiguous<TWrap>>(palette,
                                                                       firstPaletteIndex,
                                                                       paletteIndexStep,
                                                                       outputColors,
                                                                       options);
            case PaletteBlendMode::Cubic:
                return samplePalette<BlendCubicContiguous<TWrap>>(palette,
                                                                  firstPaletteIndex,
                                                                  paletteIndexStep,
                                                                  outputColors,
                                                                  options);
            case PaletteBlendMode::Cosine:
                return samplePalette<BlendCosineContiguous<TWrap>>(palette,
                                                                   firstPaletteIndex,
                                                                   paletteIndexStep,
                                                                   outputColors,
                                                                   options);
            case PaletteBlendMode::GammaLinear:
                return samplePalette<BlendGammaLinearContiguous<TWrap>>(palette,
                                                                        firstPaletteIndex,
                                                                        paletteIndexStep,
                                                                        outputColors,
                                                                        options);
            case PaletteBlendMode::Quantized:
                return samplePalette<BlendQuantizedContiguous<TWrap, 8>>(palette,
                                                                         firstPaletteIndex,
                                                                         paletteIndexStep,
                                                                         outputColors,
                                                                         options);
            case PaletteBlendMode::DitheredLinear:
                return samplePalette<BlendDitheredLinearContiguous<TWrap>>(palette,
                                                                           firstPaletteIndex,
                                                                           paletteIndexStep,
                                                                           outputColors,
                                                                           options);
            case PaletteBlendMode::Linear:
            default:
                return samplePalette<BlendLinearContiguous<TWrap>>(palette,
                                                                   firstPaletteIndex,
                                                                   paletteIndexStep,
                                                                   outputColors,
                                                                   options);
            }
        }
    } // namespace detail::samplingmodes

    template <typename TColor,
              typename TOutputRange,
              typename = std::enable_if_t<ColorType<TColor> && IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
    constexpr size_t samplePaletteWithModes(span<const PaletteStop<TColor>> stops,
                                            size_t firstPaletteIndex,
                                            size_t paletteIndexStep,
                                            TOutputRange &&outputColors,
                                            PaletteBlendMode blendMode = PaletteBlendMode::Linear,
                                            PaletteWrapMode wrapMode = PaletteWrapMode::Clamp,
                                            PaletteSampleOptions<TColor> options = {})
    {
        switch (wrapMode)
        {
        case PaletteWrapMode::Circular:
            return detail::samplingmodes::dispatchBlendMode<WrapCircular>(blendMode,
                                                                          stops,
                                                                          firstPaletteIndex,
                                                                          paletteIndexStep,
                                                                          outputColors,
                                                                          options);
        case PaletteWrapMode::Mirror:
            return detail::samplingmodes::dispatchBlendMode<WrapMirror>(blendMode,
                                                                        stops,
                                                                        firstPaletteIndex,
                                                                        paletteIndexStep,
                                                                        outputColors,
                                                                        options);
        case PaletteWrapMode::HoldFirst:
            return detail::samplingmodes::dispatchBlendMode<WrapHoldFirst>(blendMode,
                                                                           stops,
                                                                           firstPaletteIndex,
                                                                           paletteIndexStep,
                                                                           outputColors,
                                                                           options);
        case PaletteWrapMode::HoldLast:
            return detail::samplingmodes::dispatchBlendMode<WrapHoldLast>(blendMode,
                                                                          stops,
                                                                          firstPaletteIndex,
                                                                          paletteIndexStep,
                                                                          outputColors,
                                                                          options);
        case PaletteWrapMode::Blackout:
            return detail::samplingmodes::dispatchBlendMode<WrapBlackout>(blendMode,
                                                                          stops,
                                                                          firstPaletteIndex,
                                                                          paletteIndexStep,
                                                                          outputColors,
                                                                          options);
        case PaletteWrapMode::Clamp:
        default:
            return detail::samplingmodes::dispatchBlendMode<WrapClamp>(blendMode,
                                                                       stops,
                                                                       firstPaletteIndex,
                                                                       paletteIndexStep,
                                                                       outputColors,
                                                                       options);
        }
    }

    template <typename TColor,
              typename TOutputRange,
              typename = std::enable_if_t<ColorType<TColor> && IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
    constexpr size_t samplePaletteWithModes(span<const PaletteStop<TColor>> stops,
                                            size_t firstPaletteIndex,
                                            TOutputRange &&outputColors,
                                            PaletteBlendMode blendMode = PaletteBlendMode::Linear,
                                            PaletteWrapMode wrapMode = PaletteWrapMode::Clamp,
                                            PaletteSampleOptions<TColor> options = {})
    {
        return samplePaletteWithModes(stops,
                                      firstPaletteIndex,
                                      static_cast<size_t>(1),
                                      outputColors,
                                      blendMode,
                                      wrapMode,
                                      options);
    }

    template <typename TPaletteLike,
              typename TOutputRange,
              typename = std::enable_if_t<IsPaletteLike<TPaletteLike>::value && IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
    constexpr size_t samplePaletteWithModes(const TPaletteLike &palette,
                                            size_t firstPaletteIndex,
                                            size_t paletteIndexStep,
                                            TOutputRange &&outputColors,
                                            PaletteBlendMode blendMode = PaletteBlendMode::Linear,
                                            PaletteWrapMode wrapMode = PaletteWrapMode::Clamp,
                                            PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        using Stop = typename TPaletteLike::StopType;
        using Color = typename Stop::ColorType;
        return samplePaletteWithModes<Color>(palette.stops(),
                                             firstPaletteIndex,
                                             paletteIndexStep,
                                             outputColors,
                                             blendMode,
                                             wrapMode,
                                             options);
    }

    template <typename TPaletteLike,
              typename TOutputRange,
              typename = std::enable_if_t<IsPaletteLike<TPaletteLike>::value && IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
    constexpr size_t samplePaletteWithModes(const TPaletteLike &palette,
                                            size_t firstPaletteIndex,
                                            TOutputRange &&outputColors,
                                            PaletteBlendMode blendMode = PaletteBlendMode::Linear,
                                            PaletteWrapMode wrapMode = PaletteWrapMode::Clamp,
                                            PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        using Stop = typename TPaletteLike::StopType;
        using Color = typename Stop::ColorType;
        return samplePaletteWithModes<Color>(palette.stops(),
                                             firstPaletteIndex,
                                             outputColors,
                                             blendMode,
                                             wrapMode,
                                             options);
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
        const Palette<TColor> palette(stops);
        const size_t paletteIndex = TWrap::mapPositionToPaletteIndex(pixelIndex,
                                                                     pixelCount,
                                                                     palette.maxIndex());
        return samplePalette<TBlend>(palette,
                                     paletteIndex,
                                     options);
    }

    template <typename TWrap = WrapClamp,
              typename TBlend = BlendLinearContiguous<TWrap>,
              typename TColor,
              typename TOutputRange,
              typename = std::enable_if_t<ColorType<TColor> && IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
    constexpr size_t samplePaletteByPosition(span<const PaletteStop<TColor>> stops,
                                             size_t firstPixelIndex,
                                             size_t pixelStep,
                                             size_t pixelCount,
                                             TOutputRange &&outputColors,
                                             PaletteSampleOptions<TColor> options = {})
    {
        const Palette<TColor> palette(stops);

        if constexpr (!std::is_same<TWrap, WrapBlackout>::value)
        {
            const size_t outputCount = static_cast<size_t>(std::distance(outputColors.begin(), outputColors.end()));
            WrappedPaletteIndexes<TWrap> paletteIndexes(firstPixelIndex,
                                                        pixelStep,
                                                        pixelCount,
                                                        outputCount,
                                                        palette.maxIndex());

            return samplePalette<TBlend>(palette,
                                         paletteIndexes,
                                         outputColors,
                                         options);
        }

        size_t written = 0;

        size_t i = 0;
        for (auto output = outputColors.begin(); output != outputColors.end(); ++output, ++i)
        {
            const size_t pixelIndex = firstPixelIndex + (i * pixelStep);

            if (WrapBlackout::isOutOfRange(pixelIndex, pixelCount))
            {
                *output = TColor{};
                ++written;
                continue;
            }

            const size_t paletteIndex = TWrap::mapPositionToPaletteIndex(pixelIndex,
                                                                          pixelCount,
                                                                          palette.maxIndex());
            *output = samplePalette<TBlend>(palette,
                                            paletteIndex,
                                            options);
            ++written;
        }

        return written;
    }

    template <typename TWrap = WrapClamp,
              typename TBlend = BlendLinearContiguous<TWrap>,
              typename TColor,
              typename TOutputRange,
              typename = std::enable_if_t<ColorType<TColor> && IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
    constexpr size_t samplePaletteByPosition(span<const PaletteStop<TColor>> stops,
                                             size_t firstPixelIndex,
                                             size_t pixelCount,
                                             TOutputRange &&outputColors,
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
                                             size_t paletteIndex,
                                             size_t transitionProgress,
                                             size_t transitionDuration,
                                             PaletteSampleOptions<TColor> options = {})
    {
        const Palette<TColor> fromPalette(fromStops);
        const Palette<TColor> toPalette(toStops);

        const TColor fromColor = samplePalette<TBlend>(fromPalette,
                                                       paletteIndex,
                                                       options);
        const TColor toColor = samplePalette<TBlend>(toPalette,
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
              typename TOutputRange,
              typename = std::enable_if_t<IsPaletteLike<TPaletteLike>::value && IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
    constexpr size_t samplePaletteByPosition(
        const TPaletteLike &palette,
        size_t firstPixelIndex,
        size_t pixelStep,
        size_t pixelCount,
        TOutputRange &&outputColors,
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
              typename TOutputRange,
              typename = std::enable_if_t<IsPaletteLike<TPaletteLike>::value && IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
    constexpr size_t samplePaletteByPosition(
        const TPaletteLike &palette,
        size_t firstPixelIndex,
        size_t pixelCount,
        TOutputRange &&outputColors,
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
        size_t paletteIndex,
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
