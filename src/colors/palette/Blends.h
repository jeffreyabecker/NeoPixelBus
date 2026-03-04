#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "colors/ColorMath.h"
#include "colors/palette/Detail.h"
#include "colors/palette/NearestPolicies.h"
#include "colors/palette/Traits.h"

namespace lw
{
    template <typename TWrap = WrapClamp>
    struct BlendLinearContiguous
    {
        template <typename TColor,
                  typename TIndexIt,
                  typename TIndexSentinel,
                  typename TOutputIt,
                  typename TSentinel,
                  typename = std::enable_if_t<ColorType<TColor>>>
        static constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
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
            for (; output != outputEnd && index != indexEnd; ++output, ++index)
            {
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
                            sampled = linearBlend(left.color, right.color, progress);
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
                        const size_t wrapPeriod = std::max<size_t>(Palette<TColor>(stops).maxIndex(), left.index) + 1;

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
                            sampled = linearBlend(left.color, right.color, progress);
                        }
                    }
                }

                *output = detail::applyBrightnessScale(sampled, options.brightnessScale);
                ++written;
            }

            return written;
        }

        template <typename TColor,
                  typename TIndexIt,
                  typename TIndexSentinel,
                  typename TOutputRange,
                  typename = std::enable_if_t<ColorType<TColor> && IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
        static constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                              TIndexIt index,
                                              TIndexSentinel indexEnd,
                                              TOutputRange &&outputColors,
                                              PaletteSampleOptions<TColor> options = {})
        {
            return samplePalette<TColor>(stops,
                                         index,
                                         indexEnd,
                                         outputColors.begin(),
                                         outputColors.end(),
                                         options);
        }
    };

    template <typename TWrap = WrapClamp,
              typename TTieBreak = NearestTieStable>
    struct BlendNearestContiguous
    {
        template <typename TColor,
                  typename TIndexIt,
                  typename TIndexSentinel,
                  typename TOutputIt,
                  typename TSentinel,
                  typename = std::enable_if_t<ColorType<TColor>>>
        static constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
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
            for (; output != outputEnd && index != indexEnd; ++output, ++index)
            {
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

        template <typename TColor,
                  typename TIndexIt,
                  typename TIndexSentinel,
                  typename TOutputRange,
                  typename = std::enable_if_t<ColorType<TColor> && IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
        static constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                              TIndexIt index,
                                              TIndexSentinel indexEnd,
                                              TOutputRange &&outputColors,
                                              PaletteSampleOptions<TColor> options = {})
        {
            return samplePalette<TColor>(stops,
                                         index,
                                         indexEnd,
                                         outputColors.begin(),
                                         outputColors.end(),
                                         options);
        }
    };

} // namespace lw
