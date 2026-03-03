#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "core/Compat.h"
#include "core/IndexIterator.h"
#include "colors/Color.h"
#include "colors/ColorMath.h"

namespace lw
{
    enum class PaletteBlendMode : uint8_t
    {
        Linear,
        Nearest
    };

    enum class PaletteWrapMode : uint8_t
    {
        Clamp,
        Wrap
    };

    template <typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    struct PaletteSampleOptions
    {
        PaletteBlendMode blendMode{PaletteBlendMode::Linear};
        PaletteWrapMode wrapMode{PaletteWrapMode::Clamp};
        typename TColor::ComponentType brightnessScale{std::numeric_limits<typename TColor::ComponentType>::max()};
    };

    template <typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    struct PaletteStop
    {
        uint8_t index{0};
        TColor color{};
    };

    template <typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    class Palette
    {
    public:
        using StopType = PaletteStop<TColor>;

        constexpr Palette() = default;

        constexpr explicit Palette(span<const StopType> stops)
            : _stops(stops)
        {
        }

        constexpr span<const StopType> stops() const
        {
            return _stops;
        }

        constexpr bool empty() const
        {
            return _stops.empty();
        }

        constexpr size_t size() const
        {
            return _stops.size();
        }

    private:
        span<const StopType> _stops{};
    };

    constexpr uint8_t mapPositionToPaletteIndex(size_t pixelIndex,
                                                size_t pixelCount,
                                                PaletteWrapMode wrapMode = PaletteWrapMode::Clamp)
    {
        if (pixelCount == 0)
        {
            return 0;
        }

        if (wrapMode == PaletteWrapMode::Wrap)
        {
            const size_t wrapped = pixelIndex % pixelCount;
            return static_cast<uint8_t>((wrapped * 256ull) / pixelCount);
        }

        if (pixelCount <= 1)
        {
            return 0;
        }

        const size_t clamped = (pixelIndex >= pixelCount) ? (pixelCount - 1) : pixelIndex;
        return static_cast<uint8_t>((clamped * 255ull) / (pixelCount - 1));
    }

    namespace detail
    {
        constexpr uint16_t absDiffU8(uint8_t left, uint8_t right)
        {
            return (left >= right)
                       ? static_cast<uint16_t>(left - right)
                       : static_cast<uint16_t>(right - left);
        }

        constexpr uint16_t circularDiffU8(uint8_t left, uint8_t right)
        {
            const uint16_t direct = absDiffU8(left, right);
            return std::min<uint16_t>(direct, static_cast<uint16_t>(256 - direct));
        }

        template <typename TColor,
                  typename = std::enable_if_t<ColorType<TColor>>>
        constexpr TColor applyBrightnessScale(TColor color,
                                              typename TColor::ComponentType brightnessScale)
        {
            using Component = typename TColor::ComponentType;
            constexpr uint32_t MaxComponent = static_cast<uint32_t>(std::numeric_limits<Component>::max());
            const uint32_t scale = static_cast<uint32_t>(brightnessScale);

            if (scale >= MaxComponent)
            {
                return color;
            }

            for (char channel : TColor::channelIndexes())
            {
                const uint32_t value = static_cast<uint32_t>(color[channel]);
                color[channel] = static_cast<Component>((value * scale) / MaxComponent);
            }

            return color;
        }

        template <typename TColor,
                  typename TOutputIt,
                  typename TSentinel,
                  typename = std::enable_if_t<ColorType<TColor>>>
        constexpr size_t writeZeroed(TOutputIt output,
                                     TSentinel outputEnd)
        {
            size_t written = 0;
            for (; output != outputEnd; ++output)
            {
                *output = TColor{};
                ++written;
            }

            return written;
        }

        template <typename TColor,
                  typename TOutputIt,
                  typename TSentinel,
                  typename = std::enable_if_t<ColorType<TColor>>>
        constexpr size_t writeScaledSolid(TColor color,
                                          typename TColor::ComponentType brightnessScale,
                                          TOutputIt output,
                                          TSentinel outputEnd)
        {
            const TColor scaled = applyBrightnessScale(color, brightnessScale);
            size_t written = 0;
            for (; output != outputEnd; ++output)
            {
                *output = scaled;
                ++written;
            }

            return written;
        }

        template <typename TColor,
                  typename TIndexIt,
                  typename TIndexSentinel,
                  typename TOutputIt,
                  typename TSentinel,
                  typename = std::enable_if_t<ColorType<TColor>>>
        constexpr size_t sampleNearestContiguous(span<const PaletteStop<TColor>> stops,
                                                 TIndexIt index,
                                                 TIndexSentinel indexEnd,
                                                 TOutputIt output,
                                                 TSentinel outputEnd,
                                                 PaletteSampleOptions<TColor> options)
        {
            size_t written = 0;
            for (; output != outputEnd && index != indexEnd; ++output, ++index)
            {
                const uint8_t sampleIndex = *index;
                size_t nearestStopIndex = 0;
                uint16_t nearestDistance = std::numeric_limits<uint16_t>::max();

                for (size_t stopIndex = 0; stopIndex < stops.size(); ++stopIndex)
                {
                    const uint16_t distance = (options.wrapMode == PaletteWrapMode::Wrap)
                                                  ? circularDiffU8(stops[stopIndex].index, sampleIndex)
                                                  : absDiffU8(stops[stopIndex].index, sampleIndex);

                    if (distance < nearestDistance)
                    {
                        nearestDistance = distance;
                        nearestStopIndex = stopIndex;
                    }
                }

                *output = applyBrightnessScale(stops[nearestStopIndex].color, options.brightnessScale);
                ++written;
            }

            return written;
        }

        template <typename TColor,
                  typename TIndexIt,
                  typename TIndexSentinel,
                  typename TOutputIt,
                  typename TSentinel,
                  typename = std::enable_if_t<ColorType<TColor>>>
        constexpr size_t sampleLinearContiguous(span<const PaletteStop<TColor>> stops,
                                                TIndexIt index,
                                                TIndexSentinel indexEnd,
                                                TOutputIt output,
                                                TSentinel outputEnd,
                                                PaletteSampleOptions<TColor> options)
        {
            size_t written = 0;
            for (; output != outputEnd && index != indexEnd; ++output, ++index)
            {
                const uint8_t sampleIndex = *index;
                TColor sampled{};

                if (sampleIndex <= stops.front().index)
                {
                    if (options.wrapMode == PaletteWrapMode::Clamp)
                    {
                        sampled = stops.front().color;
                        *output = applyBrightnessScale(sampled, options.brightnessScale);
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
                    if (options.wrapMode == PaletteWrapMode::Clamp)
                    {
                        sampled = stops.back().color;
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

                *output = applyBrightnessScale(sampled, options.brightnessScale);
                ++written;
            }

            return written;
        }
    } // namespace detail

    template <typename TColor,
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

        if (options.blendMode == PaletteBlendMode::Nearest)
        {
            return detail::sampleNearestContiguous(stops, index, indexEnd, output, outputEnd, options);
        }

        return detail::sampleLinearContiguous(stops, index, indexEnd, output, outputEnd, options);
    }

    template <typename TColor,
              typename TIndexIt,
              typename TIndexSentinel,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                   TIndexIt index,
                                   TIndexSentinel indexEnd,
                                   span<TColor> outputColors,
                                   PaletteSampleOptions<TColor> options = {})
    {
        return samplePalette(stops,
                             index,
                             indexEnd,
                             outputColors.begin(),
                             outputColors.end(),
                             options);
    }

} // namespace lw
