#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

#include "colors/palette/BlendOperations.h"
#include "colors/palette/Detail.h"
#include "colors/palette/NearestPolicies.h"
#include "colors/palette/Traits.h"

namespace lw
{
    template <typename TWrap,
              typename TBlendOp,
              typename TColor,
              typename TIndexIt,
              typename TIndexSentinel,
              typename TOutputIt,
              typename TSentinel,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr size_t sampleInterpolated(span<const PaletteStop<TColor>> stops,
                                        TIndexIt index,
                                        TIndexSentinel indexEnd,
                                        TOutputIt output,
                                        TSentinel outputEnd,
                                        PaletteSampleOptions<TColor> options);

    template <typename TIndexIt,
              typename = void>
    struct HasPositionMetadata : std::false_type
    {
    };

    template <typename TIndexIt>
    struct HasPositionMetadata<TIndexIt,
                               std::void_t<decltype(std::declval<const TIndexIt &>().currentPosition()),
                                           decltype(std::declval<const TIndexIt &>().positionCount())>> : std::true_type
    {
    };

    template <typename TWrap,
              typename TIndexIt>
    constexpr bool sampleIsOutOfRange(const TIndexIt &index)
    {
        if constexpr (HasPositionMetadata<TIndexIt>::value)
        {
            return TWrap::isOutOfRange(index.currentPosition(),
                                       index.positionCount());
        }

        return false;
    }

    template <typename TColor,
              typename TOutputIt>
    constexpr void writeOutOfRangeSample(TOutputIt &output,
                                         PaletteSampleOptions<TColor> options)
    {
        *output = detail::applyBrightnessScale(options.outOfRangeColor,
                                               options.brightnessScale);
    }

    struct BlendLinearContiguous
    {
        template <typename TWrap = WrapClamp,
                  typename TPaletteLike,
                  typename TIndexRange,
                  typename TOutputRange,
                  typename = std::enable_if_t<IsPaletteLike<TPaletteLike>::value &&
                                              IsBeginEndRange<std::remove_reference_t<TIndexRange>>::value &&
                                              IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
        static constexpr size_t samplePalette(const TPaletteLike &palette,
                                              TIndexRange &&paletteIndexes,
                                              TOutputRange &&outputColors,
                                              PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
        {
            using Stop = typename TPaletteLike::StopType;
            using TColor = typename Stop::ColorType;

            return sampleInterpolated<TWrap, BlendOpLinear, TColor>(palette.stops(),
                                                                     paletteIndexes.begin(),
                                                                     paletteIndexes.end(),
                                                                     outputColors.begin(),
                                                                     outputColors.end(),
                                                                     options);
        }
    };

    template <typename TTieBreak = NearestTieStable>
    struct BlendNearestContiguous
    {
        template <typename TWrap = WrapClamp,
                  typename TPaletteLike,
                  typename TIndexRange,
                  typename TOutputRange,
                  typename = std::enable_if_t<IsPaletteLike<TPaletteLike>::value &&
                                              IsBeginEndRange<std::remove_reference_t<TIndexRange>>::value &&
                                              IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
        static constexpr size_t samplePalette(const TPaletteLike &palette,
                                              TIndexRange &&paletteIndexes,
                                              TOutputRange &&outputColors,
                                              PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
        {
            using Stop = typename TPaletteLike::StopType;
            using TColor = typename Stop::ColorType;

            const span<const Stop> stops = palette.stops();
            auto index = paletteIndexes.begin();
            const auto indexEnd = paletteIndexes.end();
            auto output = outputColors.begin();
            const auto outputEnd = outputColors.end();

            if (stops.empty())
            {
                return detail::writeZeroed<TColor>(output, outputEnd);
            }

            if (stops.size() == 1)
            {
                return detail::writeScaledSolid<TColor>(stops.front().color,
                                                        options.brightnessScale,
                                                        output,
                                                        outputEnd);
            }

            size_t written = 0;
            for (; output != outputEnd && index != indexEnd; ++output, ++index)
            {
                if (sampleIsOutOfRange<TWrap>(index))
                {
                    writeOutOfRangeSample<TColor>(output, options);
                    ++written;
                    continue;
                }

                const size_t sampleIndex = *index;
                size_t nearestStopIndex = 0;
                const size_t maxIndex = Palette<TColor>(stops).maxIndex();
                size_t nearestDistance = std::numeric_limits<size_t>::max();

                for (size_t stopIndex = 0; stopIndex < stops.size(); ++stopIndex)
                {
                    const size_t distance = TWrap::distance(stops[stopIndex].index,
                                                            sampleIndex,
                                                            maxIndex);

                    if (distance < nearestDistance)
                    {
                        nearestDistance = distance;
                        nearestStopIndex = stopIndex;
                    }
                    else if (distance == nearestDistance && TTieBreak::pickCandidate(stopIndex, nearestStopIndex))
                    {
                        nearestStopIndex = stopIndex;
                    }
                }

                *output = detail::applyBrightnessScale(stops[nearestStopIndex].color, options.brightnessScale);
                ++written;
            }

            return written;
        }
    };

    template <typename TWrap,
              typename TBlendOp,
              typename TColor,
              typename TIndexIt,
              typename TIndexSentinel,
              typename TOutputIt,
              typename TSentinel,
              typename>
    constexpr size_t sampleInterpolated(span<const PaletteStop<TColor>> stops,
                                        TIndexIt index,
                                        TIndexSentinel indexEnd,
                                        TOutputIt output,
                                        TSentinel outputEnd,
                                        PaletteSampleOptions<TColor> options)
    {
        if (stops.empty())
        {
            return detail::writeZeroed<TColor>(output, outputEnd);
        }

        if (stops.size() == 1)
        {
            return detail::writeScaledSolid<TColor>(stops.front().color,
                                                    options.brightnessScale,
                                                    output,
                                                    outputEnd);
        }

        size_t written = 0;
        const size_t maxIndex = Palette<TColor>(stops).maxIndex();
        for (; output != outputEnd && index != indexEnd; ++output, ++index)
        {
            if (sampleIsOutOfRange<TWrap>(index))
            {
                writeOutOfRangeSample<TColor>(output, options);
                ++written;
                continue;
            }

            const size_t sampleIndex = *index;
            TColor sampled{};

            if constexpr (std::is_same<TWrap, WrapClamp>::value ||
                          std::is_same<TWrap, WrapHoldFirst>::value ||
                          std::is_same<TWrap, WrapHoldLast>::value ||
                          std::is_same<TWrap, WrapBlackout>::value)
            {
                if (sampleIndex <= stops.front().index)
                {
                    if constexpr (std::is_same<TWrap, WrapHoldLast>::value)
                    {
                        sampled = stops.back().color;
                    }
                    else
                    {
                        sampled = stops.front().color;
                    }

                    *output = detail::applyBrightnessScale(sampled, options.brightnessScale);
                    ++written;
                    continue;
                }
            }

            bool foundSpan = false;
            for (size_t stopIndex = 1; stopIndex < stops.size(); ++stopIndex)
            {
                if (sampleIndex <= stops[stopIndex].index)
                {
                    const auto &left = stops[stopIndex - 1];
                    const auto &right = stops[stopIndex];
                    const size_t spanWidth = right.index - left.index;

                    if (spanWidth == 0)
                    {
                        sampled = right.color;
                    }
                    else
                    {
                        const size_t offset = sampleIndex - left.index;
                        const uint8_t progress = static_cast<uint8_t>((offset * 255u) / spanWidth);
                        sampled = TBlendOp::template apply<TColor>(left.color,
                                                                   right.color,
                                                                   progress,
                                                                   sampleIndex);
                    }

                    foundSpan = true;
                    break;
                }
            }

            if (!foundSpan)
            {
                if constexpr (std::is_same<TWrap, WrapClamp>::value ||
                              std::is_same<TWrap, WrapHoldFirst>::value ||
                              std::is_same<TWrap, WrapHoldLast>::value ||
                              std::is_same<TWrap, WrapBlackout>::value)
                {
                    if constexpr (std::is_same<TWrap, WrapHoldFirst>::value)
                    {
                        sampled = stops.front().color;
                    }
                    else if constexpr (std::is_same<TWrap, WrapBlackout>::value)
                    {
                        sampled = TColor{};
                    }
                    else
                    {
                        sampled = stops.back().color;
                    }
                }
                else
                {
                    const auto &left = stops.back();
                    const auto &right = stops.front();
                    const size_t wrapPeriod = std::max(maxIndex, left.index) + 1;

                    const size_t leftIndex = left.index;
                    const size_t rightIndex = right.index + wrapPeriod;
                    const size_t wrappedSampleIndex = (sampleIndex >= left.index)
                                                        ? sampleIndex
                                                        : (sampleIndex + wrapPeriod);

                    const size_t spanWidth = rightIndex - leftIndex;
                    if (spanWidth == 0)
                    {
                        sampled = left.color;
                    }
                    else
                    {
                        const size_t offset = wrappedSampleIndex - leftIndex;
                        const uint8_t progress = static_cast<uint8_t>((offset * 255u) / spanWidth);
                        sampled = TBlendOp::template apply<TColor>(left.color,
                                                                   right.color,
                                                                   progress,
                                                                   sampleIndex);
                    }
                }
            }

            *output = detail::applyBrightnessScale(sampled, options.brightnessScale);
            ++written;
        }

        return written;
    }

    template <typename TBlendOp>
    struct InterpolatedBlendContiguous
    {
        template <typename TWrap = WrapClamp,
                  typename TPaletteLike,
                  typename TIndexRange,
                  typename TOutputRange,
                  typename = std::enable_if_t<IsPaletteLike<TPaletteLike>::value &&
                                              IsBeginEndRange<std::remove_reference_t<TIndexRange>>::value &&
                                              IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
        static constexpr size_t samplePalette(const TPaletteLike &palette,
                                              TIndexRange &&paletteIndexes,
                                              TOutputRange &&outputColors,
                                              PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
        {
            using Stop = typename TPaletteLike::StopType;
            using TColor = typename Stop::ColorType;
            return sampleInterpolated<TWrap, TBlendOp, TColor>(palette.stops(),
                                                                paletteIndexes.begin(),
                                                                paletteIndexes.end(),
                                                                outputColors.begin(),
                                                                outputColors.end(),
                                                                options);
        }
    };

    using BlendStepContiguous = InterpolatedBlendContiguous<BlendOpStep>;
    using BlendHoldMidpointContiguous = InterpolatedBlendContiguous<BlendOpHoldMidpoint>;
    using BlendSmoothstepContiguous = InterpolatedBlendContiguous<BlendOpSmoothstep>;
    using BlendCubicContiguous = InterpolatedBlendContiguous<BlendOpCubic>;
    using BlendCosineContiguous = InterpolatedBlendContiguous<BlendOpCosine>;
    using BlendGammaLinearContiguous = InterpolatedBlendContiguous<BlendOpGammaLinear>;
    template <uint8_t TLevels = 8>
    using BlendQuantizedContiguous = InterpolatedBlendContiguous<BlendOpQuantized<TLevels>>;
    using BlendDitheredLinearContiguous = InterpolatedBlendContiguous<BlendOpDitheredLinear>;

    namespace blend
    {
        namespace op
        {
            using Step = BlendOpStep;
            using HoldMidpoint = BlendOpHoldMidpoint;
            using Midpoint = BlendOpHoldMidpoint;
            using MidPoint = BlendOpHoldMidpoint;
            using Smoothstep = BlendOpSmoothstep;
            using Cubic = BlendOpCubic;
            using Cosine = BlendOpCosine;
            using GammaLinear = BlendOpGammaLinear;
            template <uint8_t TLevels = 8>
            using Quantized = BlendOpQuantized<TLevels>;
            using DitheredLinear = BlendOpDitheredLinear;
        } // namespace op

        using Linear = BlendLinearContiguous;
        template <typename TTieBreak = NearestTieStable>
        using Nearest = BlendNearestContiguous<TTieBreak>;

        template <typename TOperation>
        using Interpolated = InterpolatedBlendContiguous<TOperation>;

        using Step = BlendStepContiguous;
        using HoldMidpoint = BlendHoldMidpointContiguous;
        using Midpoint = BlendHoldMidpointContiguous;
        using MidPoint = BlendHoldMidpointContiguous;
        using Smoothstep = BlendSmoothstepContiguous;
        using Cubic = BlendCubicContiguous;
        using Cosine = BlendCosineContiguous;
        using GammaLinear = BlendGammaLinearContiguous;
        template <uint8_t TLevels = 8>
        using Quantized = BlendQuantizedContiguous<TLevels>;
        using DitheredLinear = BlendDitheredLinearContiguous;
    } // namespace blend

} // namespace lw
