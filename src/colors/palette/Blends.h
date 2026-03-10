#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

#include "colors/palette/BlendOperations.h"
#include "colors/palette/Detail.h"
#include "colors/palette/ModeEnums.h"
#include "colors/palette/Traits.h"

namespace lw::colors::palettes
{
namespace detail
{
template <typename TColor> size_t maxStopIndex(span<const PaletteStop<TColor>> stops)
{
    return stops.empty() ? 0u : static_cast<size_t>(stops.back().index);
}

template <typename TColor> size_t firstStopAtOrAfter(span<const PaletteStop<TColor>> stops, size_t sampleIndex)
{
    size_t left = 1;
    size_t right = static_cast<size_t>(stops.size());

    while (left < right)
    {
        const size_t mid = left + ((right - left) / 2u);
        if (static_cast<size_t>(stops[mid].index) < sampleIndex)
        {
            left = mid + 1u;
        }
        else
        {
            right = mid;
        }
    }

    return left;
}

inline constexpr bool pickNearestTieCandidate(TieBreakPolicy tieBreakPolicy, size_t candidateIndex, size_t currentIndex)
{
    switch (tieBreakPolicy)
    {
        case TieBreakPolicy::Left:
            return candidateIndex < currentIndex;
        case TieBreakPolicy::Right:
            return candidateIndex > currentIndex;
        case TieBreakPolicy::Stable:
        default:
            return false;
    }
}

inline constexpr bool wrapPositionIsOutOfRange(WrapMode wrapMode, size_t position, size_t positionCount)
{
    switch (wrapMode)
    {
        case WrapMode::Blackout:
            return WrapBlackout::isOutOfRange(position, positionCount);
        case WrapMode::Clamp:
        case WrapMode::Circular:
        case WrapMode::Mirror:
        case WrapMode::HoldFirst:
        case WrapMode::HoldLast:
        case WrapMode::Window:
        case WrapMode::ModuloSpan:
        case WrapMode::OffsetCircular:
        default:
            return false;
    }
}

inline constexpr size_t wrapDistance(WrapMode wrapMode, size_t left, size_t right, size_t maxIndex)
{
    switch (wrapMode)
    {
        case WrapMode::Circular:
        case WrapMode::OffsetCircular:
            return WrapCircular::distance(left, right, maxIndex);
        case WrapMode::Clamp:
        case WrapMode::Mirror:
        case WrapMode::HoldFirst:
        case WrapMode::HoldLast:
        case WrapMode::Blackout:
        case WrapMode::Window:
        case WrapMode::ModuloSpan:
        default:
            return WrapClamp::distance(left, right, maxIndex);
    }
}

inline constexpr bool usesClampedBoundarySampling(WrapMode wrapMode)
{
    switch (wrapMode)
    {
        case WrapMode::Clamp:
        case WrapMode::HoldFirst:
        case WrapMode::HoldLast:
        case WrapMode::Blackout:
            return true;
        case WrapMode::Circular:
        case WrapMode::Mirror:
        case WrapMode::Window:
        case WrapMode::ModuloSpan:
        case WrapMode::OffsetCircular:
        default:
            return false;
    }
}

template <typename TColor>
TColor lowerBoundarySample(WrapMode wrapMode, span<const PaletteStop<TColor>> stops)
{
    return (wrapMode == WrapMode::HoldLast) ? stops.back().color : stops.front().color;
}

template <typename TColor>
TColor upperBoundarySample(WrapMode wrapMode, span<const PaletteStop<TColor>> stops)
{
    switch (wrapMode)
    {
        case WrapMode::HoldFirst:
            return stops.front().color;
        case WrapMode::Blackout:
            return TColor{};
        case WrapMode::Clamp:
        case WrapMode::HoldLast:
        case WrapMode::Circular:
        case WrapMode::Mirror:
        case WrapMode::Window:
        case WrapMode::ModuloSpan:
        case WrapMode::OffsetCircular:
        default:
            return stops.back().color;
    }
}
} // namespace detail

template <
    typename TColor, typename TIndexRange, typename TOutputRange,
    typename = std::enable_if_t<ColorType<TColor> && IsBeginEndRange<std::remove_reference_t<TIndexRange>>::value &&
                                IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
size_t sampleNearest(const IPalette<TColor>& palette, TIndexRange&& paletteIndexes, TOutputRange&& outputColors,
                     PaletteSampleOptions<TColor> options);

template <
    typename TColor, typename TIndexRange, typename TOutputRange,
    typename = std::enable_if_t<ColorType<TColor> && IsBeginEndRange<std::remove_reference_t<TIndexRange>>::value &&
                                IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
size_t sampleInterpolated(const IPalette<TColor>& palette, TIndexRange&& paletteIndexes, TOutputRange&& outputColors,
                          PaletteSampleOptions<TColor> options);

template <typename TIndexIt, typename = void> struct HasPositionMetadata : std::false_type
{
};

template <typename TIndexIt>
struct HasPositionMetadata<TIndexIt, std::void_t<decltype(std::declval<const TIndexIt&>().currentPosition()),
                                                 decltype(std::declval<const TIndexIt&>().positionCount())>>
    : std::true_type
{
};

template <typename TIndexIt> bool sampleIsOutOfRange(const TIndexIt& index, WrapMode wrapMode)
{
    if constexpr (HasPositionMetadata<TIndexIt>::value)
    {
        return detail::wrapPositionIsOutOfRange(wrapMode, index.currentPosition(), index.positionCount());
    }

    return false;
}

template <typename TColor, typename TOutputIt>
void writeOutOfRangeSample(TOutputIt& output, PaletteSampleOptions<TColor> options)
{
    *output = detail::applyBrightnessScale(options.outOfRangeColor, options.brightnessScale);
}

template <typename TColor, typename TIndexRange, typename TOutputRange, typename>
size_t sampleNearest(const IPalette<TColor>& palette, TIndexRange&& paletteIndexes, TOutputRange&& outputColors,
                     PaletteSampleOptions<TColor> options)
{
    const auto stops = palette.stops();
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
        return detail::writeScaledSolid<TColor>(stops.front().color, options.brightnessScale, output, outputEnd);
    }

    const size_t maxIndex = detail::maxStopIndex<TColor>(stops);
    size_t written = 0;
    for (; output != outputEnd && index != indexEnd; ++output, ++index)
    {
        if (sampleIsOutOfRange(index, options.wrapMode))
        {
            writeOutOfRangeSample<TColor>(output, options);
            ++written;
            continue;
        }

        const size_t sampleIndex = *index;
        size_t nearestStopIndex = 0;
        size_t nearestDistance = std::numeric_limits<size_t>::max();

        for (size_t stopIndex = 0; stopIndex < stops.size(); ++stopIndex)
        {
            const size_t distance = detail::wrapDistance(options.wrapMode, stops[stopIndex].index, sampleIndex, maxIndex);

            if (distance < nearestDistance)
            {
                nearestDistance = distance;
                nearestStopIndex = stopIndex;
            }
            else if (distance == nearestDistance &&
                     detail::pickNearestTieCandidate(options.tieBreakPolicy, stopIndex, nearestStopIndex))
            {
                nearestStopIndex = stopIndex;
            }
        }

        *output = detail::applyBrightnessScale(stops[nearestStopIndex].color, options.brightnessScale);
        ++written;
    }

    return written;
}

template <typename TColor, typename TIndexRange, typename TOutputRange, typename>
size_t sampleInterpolated(const IPalette<TColor>& palette, TIndexRange&& paletteIndexes, TOutputRange&& outputColors,
                          PaletteSampleOptions<TColor> options)
{
    const auto stops = palette.stops();
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
        return detail::writeScaledSolid<TColor>(stops.front().color, options.brightnessScale, output, outputEnd);
    }

    size_t written = 0;
    const size_t maxIndex = detail::maxStopIndex<TColor>(stops);
    for (; output != outputEnd && index != indexEnd; ++output, ++index)
    {
        if (sampleIsOutOfRange(index, options.wrapMode))
        {
            writeOutOfRangeSample<TColor>(output, options);
            ++written;
            continue;
        }

        const size_t sampleIndex = *index;
        TColor sampled{};

        if (detail::usesClampedBoundarySampling(options.wrapMode))
        {
            if (sampleIndex <= stops.front().index)
            {
                sampled = detail::lowerBoundarySample<TColor>(options.wrapMode, stops);

                *output = detail::applyBrightnessScale(sampled, options.brightnessScale);
                ++written;
                continue;
            }
        }

        const size_t stopIndex = detail::firstStopAtOrAfter<TColor>(stops, sampleIndex);
        if (stopIndex < static_cast<size_t>(stops.size()))
        {
            const auto& left = stops[stopIndex - 1];
            const auto& right = stops[stopIndex];
            const size_t spanWidth = right.index - left.index;

            if (spanWidth == 0)
            {
                sampled = right.color;
            }
            else
            {
                const size_t offset = sampleIndex - left.index;
                const uint8_t progress = static_cast<uint8_t>((offset * 255u) / spanWidth);
                sampled = applyBlendMode<TColor>(options.blendMode, left.color, right.color, progress, sampleIndex,
                                                 options.quantizedLevels);
            }
        }
        else
        {
            if (detail::usesClampedBoundarySampling(options.wrapMode))
            {
                sampled = detail::upperBoundarySample<TColor>(options.wrapMode, stops);
            }
            else
            {
                const auto& left = stops.back();
                const auto& right = stops.front();
                const size_t wrapPeriod = std::max(maxIndex, left.index) + 1;

                const size_t leftIndex = left.index;
                const size_t rightIndex = right.index + wrapPeriod;
                const size_t wrappedSampleIndex =
                    (sampleIndex >= left.index) ? sampleIndex : (sampleIndex + wrapPeriod);

                const size_t spanWidth = rightIndex - leftIndex;
                if (spanWidth == 0)
                {
                    sampled = left.color;
                }
                else
                {
                    const size_t offset = wrappedSampleIndex - leftIndex;
                    const uint8_t progress = static_cast<uint8_t>((offset * 255u) / spanWidth);
                    sampled = applyBlendMode<TColor>(options.blendMode, left.color, right.color, progress, sampleIndex,
                                                     options.quantizedLevels);
                }
            }
        }

        *output = detail::applyBrightnessScale(sampled, options.brightnessScale);
        ++written;
    }

    return written;
}

namespace blend
{
inline constexpr BlendMode Linear = BlendMode::Linear;
inline constexpr BlendMode Nearest = BlendMode::Nearest;
inline constexpr BlendMode Step = BlendMode::Step;
inline constexpr BlendMode HoldMidpoint = BlendMode::HoldMidpoint;
inline constexpr BlendMode Midpoint = BlendMode::HoldMidpoint;
inline constexpr BlendMode MidPoint = BlendMode::HoldMidpoint;
inline constexpr BlendMode Smoothstep = BlendMode::Smoothstep;
inline constexpr BlendMode Cubic = BlendMode::Cubic;
inline constexpr BlendMode Cosine = BlendMode::Cosine;
inline constexpr BlendMode GammaLinear = BlendMode::GammaLinear;
inline constexpr BlendMode Quantized = BlendMode::Quantized;
inline constexpr BlendMode DitheredLinear = BlendMode::DitheredLinear;
} // namespace blend

} // namespace lw::colors::palettes
