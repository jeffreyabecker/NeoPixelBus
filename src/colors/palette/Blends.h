#pragma once

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
                const uint8_t sampleIndex = *index;
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
                        const uint16_t spanWidth = static_cast<uint16_t>(right.index - left.index);

                        if (spanWidth == 0)
                        {
                            sampled = right.color;
                        }
                        else
                        {
                            const uint16_t offset = static_cast<uint16_t>(sampleIndex - left.index);
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

                        const uint16_t leftIndex = left.index;
                        const uint16_t rightIndex = static_cast<uint16_t>(right.index) + 256u;
                        const uint16_t wrappedSampleIndex = (sampleIndex >= left.index)
                                                                ? static_cast<uint16_t>(sampleIndex)
                                                                : static_cast<uint16_t>(sampleIndex) + 256u;

                        const uint16_t spanWidth = static_cast<uint16_t>(rightIndex - leftIndex);
                        if (spanWidth == 0)
                        {
                            sampled = left.color;
                        }
                        else
                        {
                            const uint16_t offset = static_cast<uint16_t>(wrappedSampleIndex - leftIndex);
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
                const uint8_t sampleIndex = *index;
                size_t nearestStopIndex = 0;
                uint16_t nearestDistance = std::numeric_limits<uint16_t>::max();

                for (size_t stopIndex = 0; stopIndex < stops.size(); ++stopIndex)
                {
                    const uint16_t distance = TWrap::distance(stops[stopIndex].index, sampleIndex);

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
