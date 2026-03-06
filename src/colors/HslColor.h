#pragma once

#include <algorithm>

#include "Color.h"

namespace lw::colors
{
    class HslColor
    {
    public:
        constexpr HslColor() = default;

        constexpr HslColor(float h, float s, float l)
            : H(h), S(s), L(l)
        {
        }

        template <typename TComponent, size_t InternalSize,
                  typename std::enable_if<std::is_integral<TComponent>::value, int>::type = 0>
        constexpr HslColor(const RgbBasedColor<3, TComponent, InternalSize> &color)
        {
            const float scale = 1.0f / static_cast<float>(RgbBasedColor<3, TComponent, InternalSize>::MaxComponent);
            const float r = static_cast<float>(color['R']) * scale;
            const float g = static_cast<float>(color['G']) * scale;
            const float b = static_cast<float>(color['B']) * scale;
            rgbToHsl(r, g, b, *this);
        }

        template <typename THueBlend>
        static constexpr HslColor LinearBlend(const HslColor &left,
                                              const HslColor &right,
                                              float progress)
        {
            return HslColor(
                THueBlend::HueBlend(left.H, right.H, progress),
                left.S + ((right.S - left.S) * progress),
                left.L + ((right.L - left.L) * progress));
        }

        template <typename THueBlend>
        static constexpr HslColor BilinearBlend(const HslColor &c00,
                                                const HslColor &c01,
                                                const HslColor &c10,
                                                const HslColor &c11,
                                                float x,
                                                float y)
        {
            const float v00 = (1.0f - x) * (1.0f - y);
            const float v10 = x * (1.0f - y);
            const float v01 = (1.0f - x) * y;
            const float v11 = x * y;

            return HslColor(
                THueBlend::HueBlend(
                    THueBlend::HueBlend(c00.H, c10.H, x),
                    THueBlend::HueBlend(c01.H, c11.H, x),
                    y),
                c00.S * v00 + c10.S * v10 + c01.S * v01 + c11.S * v11,
                c00.L * v00 + c10.L * v10 + c01.L * v01 + c11.L * v11);
        }

        float H = 0.0f;
        float S = 0.0f;
        float L = 0.0f;

    private:
        static constexpr void rgbToHsl(float r, float g, float b, HslColor &color)
        {
            const float max = (r > g && r > b) ? r : ((g > b) ? g : b);
            const float min = (r < g && r < b) ? r : ((g < b) ? g : b);

            const float l = (max + min) / 2.0f;
            if (max == min)
            {
                color.H = 0.0f;
                color.S = 0.0f;
                color.L = l;
                return;
            }

            const float d = max - min;
            const float s = (l > 0.5f) ? (d / (2.0f - (max + min))) : (d / (max + min));

            float h = 0.0f;
            if (r > g && r > b)
            {
                h = (g - b) / d + (g < b ? 6.0f : 0.0f);
            }
            else if (g > b)
            {
                h = (b - r) / d + 2.0f;
            }
            else
            {
                h = (r - g) / d + 4.0f;
            }
            h /= 6.0f;

            color.H = h;
            color.S = s;
            color.L = l;
        }
    };

    namespace detail::hsl
    {
        constexpr float calcHslComponent(float p, float q, float t)
        {
            if (t < 0.0f)
            {
                t += 1.0f;
            }

            if (t > 1.0f)
            {
                t -= 1.0f;
            }

            if (t < (1.0f / 6.0f))
            {
                return p + ((q - p) * 6.0f * t);
            }

            if (t < 0.5f)
            {
                return q;
            }

            if (t < (2.0f / 3.0f))
            {
                return p + ((q - p) * ((2.0f / 3.0f) - t) * 6.0f);
            }

            return p;
        }

        constexpr float clamp01(float value)
        {
            return std::clamp(value, 0.0f, 1.0f);
        }
    }

    template <typename TColor, RequireColorChannelsExactly<TColor, 3> = 0>
    constexpr TColor toRgb(const HslColor &color)
    {
        const float h = detail::hsl::clamp01(color.H);
        const float s = detail::hsl::clamp01(color.S);
        const float l = detail::hsl::clamp01(color.L);

        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;

        if (s == 0.0f || l == 0.0f)
        {
            r = l;
            g = l;
            b = l;
        }
        else
        {
            const float q = l < 0.5f ? (l * (1.0f + s)) : (l + s - (l * s));
            const float p = 2.0f * l - q;
            r = detail::hsl::calcHslComponent(p, q, h + (1.0f / 3.0f));
            g = detail::hsl::calcHslComponent(p, q, h);
            b = detail::hsl::calcHslComponent(p, q, h - (1.0f / 3.0f));
        }

        return TColor(
            static_cast<typename TColor::ComponentType>(detail::hsl::clamp01(r) * TColor::MaxComponent),
            static_cast<typename TColor::ComponentType>(detail::hsl::clamp01(g) * TColor::MaxComponent),
            static_cast<typename TColor::ComponentType>(detail::hsl::clamp01(b) * TColor::MaxComponent));
    }
} // namespace lw::colors

namespace lw
{

using HslColor = colors::HslColor;
using colors::toRgb;

} // namespace lw

