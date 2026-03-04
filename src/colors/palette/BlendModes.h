#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "colors/palette/Blends.h"

namespace lw
{
    namespace detail
    {
        struct BlendOpStep
        {
            template <typename TColor>
            static constexpr TColor apply(const TColor &left,
                                          const TColor &,
                                          uint8_t,
                                          uint8_t)
            {
                return left;
            }
        };

        struct BlendOpHoldMidpoint
        {
            template <typename TColor>
            static constexpr TColor apply(const TColor &left,
                                          const TColor &right,
                                          uint8_t progress,
                                          uint8_t)
            {
                return (progress < 128) ? left : right;
            }
        };

        struct BlendOpSmoothstep
        {
            static constexpr uint8_t smoothstep8(uint8_t progress)
            {
                const uint32_t p = progress;
                const uint32_t numerator = p * p * (765u - (2u * p));
                return static_cast<uint8_t>(numerator / 65025u);
            }

            template <typename TColor>
            static constexpr TColor apply(const TColor &left,
                                          const TColor &right,
                                          uint8_t progress,
                                          uint8_t)
            {
                return linearBlend(left, right, smoothstep8(progress));
            }
        };

        struct BlendOpCubic
        {
            static constexpr uint8_t cubicEaseInOut8(uint8_t progress)
            {
                const uint32_t p = progress;
                if (p < 128u)
                {
                    return static_cast<uint8_t>((4u * p * p * p) / 65025u);
                }

                const uint32_t q = 255u - p;
                return static_cast<uint8_t>(255u - ((4u * q * q * q) / 65025u));
            }

            template <typename TColor>
            static constexpr TColor apply(const TColor &left,
                                          const TColor &right,
                                          uint8_t progress,
                                          uint8_t)
            {
                return linearBlend(left, right, cubicEaseInOut8(progress));
            }
        };

        struct BlendOpCosine
        {
            static constexpr uint8_t cosineLike8(uint8_t progress)
            {
                const uint32_t p = progress;
                if (p < 128u)
                {
                    return static_cast<uint8_t>((2u * p * p) / 255u);
                }

                const uint32_t q = 255u - p;
                return static_cast<uint8_t>(255u - ((2u * q * q) / 255u));
            }

            template <typename TColor>
            static constexpr TColor apply(const TColor &left,
                                          const TColor &right,
                                          uint8_t progress,
                                          uint8_t)
            {
                return linearBlend(left, right, cosineLike8(progress));
            }
        };

        struct BlendOpLinear
        {
            template <typename TColor>
            static constexpr TColor apply(const TColor &left,
                                          const TColor &right,
                                          uint8_t progress,
                                          uint8_t)
            {
                return linearBlend(left, right, progress);
            }
        };

        template <typename TColor,
                  typename = std::enable_if_t<ColorType<TColor>>>
        static constexpr uint32_t integerSqrt(uint32_t value)
        {
            uint32_t root = 0;
            uint32_t bit = 1u << 30;

            while (bit > value)
            {
                bit >>= 2;
            }

            while (bit != 0)
            {
                if (value >= root + bit)
                {
                    value -= root + bit;
                    root = (root >> 1) + bit;
                }
                else
                {
                    root >>= 1;
                }
                bit >>= 2;
            }

            return root;
        }

        struct BlendOpGammaLinear
        {
            template <typename TColor>
            static constexpr TColor apply(const TColor &left,
                                          const TColor &right,
                                          uint8_t progress,
                                          uint8_t)
            {
                using Component = typename TColor::ComponentType;
                TColor out{};

                for (char channel : TColor::channelIndexes())
                {
                    const uint32_t leftValue = static_cast<uint32_t>(left[channel]);
                    const uint32_t rightValue = static_cast<uint32_t>(right[channel]);
                    const uint32_t leftLinear = leftValue * leftValue;
                    const uint32_t rightLinear = rightValue * rightValue;

                    const uint32_t linear = leftLinear + ((rightLinear - leftLinear) * progress) / 255u;
                    const uint32_t gamma = integerSqrt<TColor>(linear);
                    out[channel] = static_cast<Component>(gamma);
                }

                return out;
            }
        };

        template <uint8_t TLevels>
        struct BlendOpQuantized
        {
            static_assert(TLevels >= 2, "BlendQuantizedContiguous requires at least 2 levels");

            template <typename TColor>
            static constexpr TColor apply(const TColor &left,
                                          const TColor &right,
                                          uint8_t progress,
                                          uint8_t)
            {
                using Component = typename TColor::ComponentType;
                constexpr uint32_t maxValue = static_cast<uint32_t>(std::numeric_limits<Component>::max());
                constexpr uint32_t step = maxValue / (TLevels - 1u);

                TColor out = linearBlend(left, right, progress);
                for (char channel : TColor::channelIndexes())
                {
                    const uint32_t value = static_cast<uint32_t>(out[channel]);
                    uint32_t quantized = ((value + (step / 2u)) / step) * step;
                    if (quantized > maxValue)
                    {
                        quantized = maxValue;
                    }

                    out[channel] = static_cast<Component>(quantized);
                }

                return out;
            }
        };

        struct BlendOpDitheredLinear
        {
            template <typename TColor>
            static constexpr TColor apply(const TColor &left,
                                          const TColor &right,
                                          uint8_t progress,
                                          uint8_t sampleIndex)
            {
                using Component = typename TColor::ComponentType;
                constexpr uint32_t maxValue = static_cast<uint32_t>(std::numeric_limits<Component>::max());

                TColor out = linearBlend(left, right, progress);
                uint8_t channelOrdinal = 0;
                for (char channel : TColor::channelIndexes())
                {
                    uint32_t value = static_cast<uint32_t>(out[channel]);
                    const uint8_t noise = static_cast<uint8_t>((sampleIndex * 37u) + (channelOrdinal * 97u));
                    if (value < maxValue && noise < (progress & 0x3Fu))
                    {
                        ++value;
                    }

                    out[channel] = static_cast<Component>(value);
                    ++channelOrdinal;
                }

                return out;
            }
        };

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

    } // namespace detail

    template <typename TWrap = WrapClamp>
    struct BlendStepContiguous
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
            return detail::sampleInterpolated<TWrap, detail::BlendOpStep>(stops,
                                                                           index,
                                                                           indexEnd,
                                                                           output,
                                                                           outputEnd,
                                                                           options);
        }

        template <typename TColor,
                  typename TIndexIt,
                  typename TIndexSentinel,
                  typename = std::enable_if_t<ColorType<TColor>>>
        static constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                              TIndexIt index,
                                              TIndexSentinel indexEnd,
                                              span<TColor> outputColors,
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

    template <typename TWrap = WrapClamp>
    struct BlendHoldMidpointContiguous
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
            return detail::sampleInterpolated<TWrap, detail::BlendOpHoldMidpoint>(stops,
                                                                                   index,
                                                                                   indexEnd,
                                                                                   output,
                                                                                   outputEnd,
                                                                                   options);
        }

        template <typename TColor,
                  typename TIndexIt,
                  typename TIndexSentinel,
                  typename = std::enable_if_t<ColorType<TColor>>>
        static constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                              TIndexIt index,
                                              TIndexSentinel indexEnd,
                                              span<TColor> outputColors,
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

    template <typename TWrap = WrapClamp>
    struct BlendSmoothstepContiguous
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
            return detail::sampleInterpolated<TWrap, detail::BlendOpSmoothstep>(stops,
                                                                                 index,
                                                                                 indexEnd,
                                                                                 output,
                                                                                 outputEnd,
                                                                                 options);
        }

        template <typename TColor,
                  typename TIndexIt,
                  typename TIndexSentinel,
                  typename = std::enable_if_t<ColorType<TColor>>>
        static constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                              TIndexIt index,
                                              TIndexSentinel indexEnd,
                                              span<TColor> outputColors,
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

    template <typename TWrap = WrapClamp>
    struct BlendCubicContiguous
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
            return detail::sampleInterpolated<TWrap, detail::BlendOpCubic>(stops,
                                                                            index,
                                                                            indexEnd,
                                                                            output,
                                                                            outputEnd,
                                                                            options);
        }

        template <typename TColor,
                  typename TIndexIt,
                  typename TIndexSentinel,
                  typename = std::enable_if_t<ColorType<TColor>>>
        static constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                              TIndexIt index,
                                              TIndexSentinel indexEnd,
                                              span<TColor> outputColors,
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

    template <typename TWrap = WrapClamp>
    struct BlendCosineContiguous
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
            return detail::sampleInterpolated<TWrap, detail::BlendOpCosine>(stops,
                                                                             index,
                                                                             indexEnd,
                                                                             output,
                                                                             outputEnd,
                                                                             options);
        }

        template <typename TColor,
                  typename TIndexIt,
                  typename TIndexSentinel,
                  typename = std::enable_if_t<ColorType<TColor>>>
        static constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                              TIndexIt index,
                                              TIndexSentinel indexEnd,
                                              span<TColor> outputColors,
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

    template <typename TWrap = WrapClamp>
    struct BlendGammaLinearContiguous
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
            return detail::sampleInterpolated<TWrap, detail::BlendOpGammaLinear>(stops,
                                                                                  index,
                                                                                  indexEnd,
                                                                                  output,
                                                                                  outputEnd,
                                                                                  options);
        }

        template <typename TColor,
                  typename TIndexIt,
                  typename TIndexSentinel,
                  typename = std::enable_if_t<ColorType<TColor>>>
        static constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                              TIndexIt index,
                                              TIndexSentinel indexEnd,
                                              span<TColor> outputColors,
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
              uint8_t TLevels = 8>
    struct BlendQuantizedContiguous
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
            return detail::sampleInterpolated<TWrap, detail::BlendOpQuantized<TLevels>>(stops,
                                                                                          index,
                                                                                          indexEnd,
                                                                                          output,
                                                                                          outputEnd,
                                                                                          options);
        }

        template <typename TColor,
                  typename TIndexIt,
                  typename TIndexSentinel,
                  typename = std::enable_if_t<ColorType<TColor>>>
        static constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                              TIndexIt index,
                                              TIndexSentinel indexEnd,
                                              span<TColor> outputColors,
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

    template <typename TWrap = WrapClamp>
    struct BlendDitheredLinearContiguous
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
            return detail::sampleInterpolated<TWrap, detail::BlendOpDitheredLinear>(stops,
                                                                                      index,
                                                                                      indexEnd,
                                                                                      output,
                                                                                      outputEnd,
                                                                                      options);
        }

        template <typename TColor,
                  typename TIndexIt,
                  typename TIndexSentinel,
                  typename = std::enable_if_t<ColorType<TColor>>>
        static constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                              TIndexIt index,
                                              TIndexSentinel indexEnd,
                                              span<TColor> outputColors,
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
