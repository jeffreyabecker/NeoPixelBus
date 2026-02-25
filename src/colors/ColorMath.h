#pragma once

#include "Color.h"
#include "detail/ColorMathBackend.h"

namespace npb
{
    template <typename TColor>
        requires ColorType<TColor>
    struct ColorMathBackendSelector
    {
        using Type = NEOPIXELBUS_COLOR_MATH_BACKEND<TColor>;
    };

    template <typename TColor>
        requires ColorType<TColor>
    constexpr void darken(TColor &color, typename TColor::ComponentType delta)
    {
        using Backend = typename ColorMathBackendSelector<TColor>::Type;
        Backend::darken(color, delta);
    }

    template <typename TColor>
        requires ColorType<TColor>
    constexpr void lighten(TColor &color, typename TColor::ComponentType delta)
    {
        using Backend = typename ColorMathBackendSelector<TColor>::Type;
        Backend::lighten(color, delta);
    }

    template <typename TColor>
        requires ColorType<TColor>
    constexpr TColor linearBlend(const TColor &left, const TColor &right, float progress)
    {
        using Backend = typename ColorMathBackendSelector<TColor>::Type;
        return Backend::linearBlend(left, right, progress);
    }

    template <typename TColor>
        requires ColorType<TColor>
    constexpr TColor linearBlend(const TColor &left, const TColor &right, uint8_t progress)
    {
        using Backend = typename ColorMathBackendSelector<TColor>::Type;
        return Backend::linearBlend(left, right, progress);
    }

    template <typename TColor>
        requires ColorType<TColor>
    constexpr TColor bilinearBlend(const TColor &c00,
                                   const TColor &c01,
                                   const TColor &c10,
                                   const TColor &c11,
                                   float x,
                                   float y)
    {
        using Backend = typename ColorMathBackendSelector<TColor>::Type;
        return Backend::bilinearBlend(c00, c01, c10, c11, x, y);
    }
}

