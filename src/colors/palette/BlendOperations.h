#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

#include "colors/ColorMath.h"

namespace lw::colors::palettes
{
struct BlendOpLinear
{
    template <typename TColor>
    static constexpr TColor apply(const TColor& left, const TColor& right, uint8_t progress, size_t)
    {
        return lw::linearBlend(left, right, progress);
    }
};

struct BlendOpStep
{
    template <typename TColor> static constexpr TColor apply(const TColor& left, const TColor&, uint8_t, size_t)
    {
        return left;
    }
};

struct BlendOpHoldMidpoint
{
    template <typename TColor>
    static constexpr TColor apply(const TColor& left, const TColor& right, uint8_t progress, size_t)
    {
        return (progress < 128) ? left : right;
    }
};

struct BlendOpSmoothstep
{
    template <typename TColor>
    static constexpr TColor apply(const TColor& left, const TColor& right, uint8_t progress, size_t)
    {
        return lw::linearBlend(left, right, lw::smoothstep8<TColor>(progress));
    }
};

struct BlendOpCubic
{
    template <typename TColor>
    static constexpr TColor apply(const TColor& left, const TColor& right, uint8_t progress, size_t)
    {
        return lw::linearBlend(left, right, lw::cubicEaseInOut8<TColor>(progress));
    }
};

struct BlendOpCosine
{
    template <typename TColor>
    static constexpr TColor apply(const TColor& left, const TColor& right, uint8_t progress, size_t)
    {
        return lw::linearBlend(left, right, lw::cosineLike8<TColor>(progress));
    }
};

struct BlendOpGammaLinear
{
    template <typename TColor>
    static constexpr TColor apply(const TColor& left, const TColor& right, uint8_t progress, size_t)
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
            const uint32_t gamma = lw::integerSqrt<TColor>(linear);
            out[channel] = static_cast<Component>(gamma);
        }

        return out;
    }
};

template <uint8_t TLevels> struct BlendOpQuantized
{
    static_assert(TLevels >= 2, "BlendQuantizedContiguous requires at least 2 levels");

    template <typename TColor>
    static constexpr TColor apply(const TColor& left, const TColor& right, uint8_t progress, size_t)
    {
        using Component = typename TColor::ComponentType;
        constexpr uint32_t maxValue = static_cast<uint32_t>(std::numeric_limits<Component>::max());
        constexpr uint32_t step = maxValue / (TLevels - 1u);

        TColor out = lw::linearBlend(left, right, progress);
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
    static constexpr TColor apply(const TColor& left, const TColor& right, uint8_t progress, size_t sampleIndex)
    {
        using Component = typename TColor::ComponentType;
        constexpr uint32_t maxValue = static_cast<uint32_t>(std::numeric_limits<Component>::max());

        TColor out = lw::linearBlend(left, right, progress);
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

} // namespace lw::colors::palettes
