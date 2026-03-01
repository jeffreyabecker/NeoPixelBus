#pragma once

#include <algorithm>

#include "Color.h"

namespace lw
{
    class HsbColor
    {
    public:
        constexpr HsbColor() = default;

        constexpr HsbColor(float h, float s, float b)
            : H(h), S(s), B(b)
        {
        }

        constexpr HsbColor(const RgbBasedColor<3, uint8_t> &color)
        {
            const float r = static_cast<float>(color['R']) / 255.0f;
            const float g = static_cast<float>(color['G']) / 255.0f;
            const float b = static_cast<float>(color['B']) / 255.0f;
            rgbToHsb(r, g, b, *this);
        }

        constexpr HsbColor(const RgbBasedColor<3, uint16_t> &color)
        {
            const float r = static_cast<float>(color['R']) / 65535.0f;
            const float g = static_cast<float>(color['G']) / 65535.0f;
            const float b = static_cast<float>(color['B']) / 65535.0f;
            rgbToHsb(r, g, b, *this);
        }

        template <typename THueBlend>
        static constexpr HsbColor LinearBlend(const HsbColor &left,
                                              const HsbColor &right,
                                              float progress)
        {
            return HsbColor(
                THueBlend::HueBlend(left.H, right.H, progress),
                left.S + ((right.S - left.S) * progress),
                left.B + ((right.B - left.B) * progress));
        }

        template <typename THueBlend>
        static constexpr HsbColor BilinearBlend(const HsbColor &c00,
                                                const HsbColor &c01,
                                                const HsbColor &c10,
                                                const HsbColor &c11,
                                                float x,
                                                float y)
        {
            const float v00 = (1.0f - x) * (1.0f - y);
            const float v10 = x * (1.0f - y);
            const float v01 = (1.0f - x) * y;
            const float v11 = x * y;

            return HsbColor(
                THueBlend::HueBlend(
                    THueBlend::HueBlend(c00.H, c10.H, x),
                    THueBlend::HueBlend(c01.H, c11.H, x),
                    y),
                c00.S * v00 + c10.S * v10 + c01.S * v01 + c11.S * v11,
                c00.B * v00 + c10.B * v10 + c01.B * v01 + c11.B * v11);
        }

        float H = 0.0f;
        float S = 0.0f;
        float B = 0.0f;

    private:
        static constexpr void rgbToHsb(float r, float g, float b, HsbColor &color)
        {
            const float max = (r > g && r > b) ? r : ((g > b) ? g : b);
            const float min = (r < g && r < b) ? r : ((g < b) ? g : b);
            const float d = max - min;

            float h = 0.0f;
            const float v = max;
            const float s = (v == 0.0f) ? 0.0f : (d / v);

            if (d != 0.0f)
            {
                if (r == max)
                {
                    h = (g - b) / d + (g < b ? 6.0f : 0.0f);
                }
                else if (g == max)
                {
                    h = (b - r) / d + 2.0f;
                }
                else
                {
                    h = (r - g) / d + 4.0f;
                }

                h /= 6.0f;
            }

            color.H = h;
            color.S = s;
            color.B = v;
        }
    };

    namespace detail::hsb
    {
        constexpr float clamp01(float value)
        {
            return std::clamp(value, 0.0f, 1.0f);
        }
    }

    constexpr Rgb8Color toRgb8(const HsbColor &color)
    {
        float h = detail::hsb::clamp01(color.H);
        const float s = detail::hsb::clamp01(color.S);
        const float v = detail::hsb::clamp01(color.B);

        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;

        if (s == 0.0f)
        {
            r = v;
            g = v;
            b = v;
        }
        else
        {
            if (h < 0.0f)
            {
                h += 1.0f;
            }
            else if (h >= 1.0f)
            {
                h -= 1.0f;
            }

            h *= 6.0f;
            const int i = static_cast<int>(h);
            const float f = h - static_cast<float>(i);
            const float q = v * (1.0f - s * f);
            const float p = v * (1.0f - s);
            const float t = v * (1.0f - s * (1.0f - f));

            switch (i)
            {
            case 0:
                r = v;
                g = t;
                b = p;
                break;
            case 1:
                r = q;
                g = v;
                b = p;
                break;
            case 2:
                r = p;
                g = v;
                b = t;
                break;
            case 3:
                r = p;
                g = q;
                b = v;
                break;
            case 4:
                r = t;
                g = p;
                b = v;
                break;
            default:
                r = v;
                g = p;
                b = q;
                break;
            }
        }

        return Rgb8Color(
                static_cast<uint8_t>(detail::hsb::clamp01(r) * Rgb8Color::MaxComponent),
                static_cast<uint8_t>(detail::hsb::clamp01(g) * Rgb8Color::MaxComponent),
                static_cast<uint8_t>(detail::hsb::clamp01(b) * Rgb8Color::MaxComponent));
    }

    constexpr Rgb16Color toRgb16(const HsbColor &color)
    {
        float h = detail::hsb::clamp01(color.H);
        const float s = detail::hsb::clamp01(color.S);
        const float v = detail::hsb::clamp01(color.B);

        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;

        if (s == 0.0f)
        {
            r = v;
            g = v;
            b = v;
        }
        else
        {
            if (h < 0.0f)
            {
                h += 1.0f;
            }
            else if (h >= 1.0f)
            {
                h -= 1.0f;
            }

            h *= 6.0f;
            const int i = static_cast<int>(h);
            const float f = h - static_cast<float>(i);
            const float q = v * (1.0f - s * f);
            const float p = v * (1.0f - s);
            const float t = v * (1.0f - s * (1.0f - f));

            switch (i)
            {
            case 0:
                r = v;
                g = t;
                b = p;
                break;
            case 1:
                r = q;
                g = v;
                b = p;
                break;
            case 2:
                r = p;
                g = v;
                b = t;
                break;
            case 3:
                r = p;
                g = q;
                b = v;
                break;
            case 4:
                r = t;
                g = p;
                b = v;
                break;
            default:
                r = v;
                g = p;
                b = q;
                break;
            }
        }

        return Rgb16Color(
            static_cast<uint16_t>(detail::hsb::clamp01(r) * Rgb16Color::MaxComponent),
            static_cast<uint16_t>(detail::hsb::clamp01(g) * Rgb16Color::MaxComponent),
            static_cast<uint16_t>(detail::hsb::clamp01(b) * Rgb16Color::MaxComponent));
    }
}

