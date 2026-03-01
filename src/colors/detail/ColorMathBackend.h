#pragma once

#include <cstdint>
#include <type_traits>

#include "colors/Color.h"

namespace lw::detail
{
    template <typename TColor>
    struct ScalarColorMathBackend
    {
        using ComponentType = typename TColor::ComponentType;

        static constexpr void darken(TColor &color, ComponentType delta)
        {
            for (auto &component : color)
            {
                if (component > delta)
                {
                    component = static_cast<ComponentType>(component - delta);
                }
                else
                {
                    component = static_cast<ComponentType>(0);
                }
            }
        }

        static constexpr void lighten(TColor &color, ComponentType delta)
        {
            for (auto &component : color)
            {
                if (component < static_cast<ComponentType>(TColor::MaxComponent - delta))
                {
                    component = static_cast<ComponentType>(component + delta);
                }
                else
                {
                    component = TColor::MaxComponent;
                }
            }
        }

        static constexpr TColor linearBlend(const TColor &left, const TColor &right, float progress)
        {
            using SignedWide = std::conditional_t<(sizeof(ComponentType) <= 2), int32_t, int64_t>;

            TColor blended;
            auto blendedIt = blended.begin();
            auto leftIt = left.begin();
            auto rightIt = right.begin();
            for (; blendedIt != blended.end(); ++blendedIt, ++leftIt, ++rightIt)
            {
                const SignedWide delta = static_cast<SignedWide>(*rightIt) - static_cast<SignedWide>(*leftIt);
                const float value = static_cast<float>(*leftIt) + (static_cast<float>(delta) * progress);
                *blendedIt = static_cast<ComponentType>(value);
            }

            return blended;
        }

        static constexpr TColor linearBlend(const TColor &left, const TColor &right, uint8_t progress)
        {
            using SignedWide = std::conditional_t<(sizeof(ComponentType) <= 2), int32_t, int64_t>;

            TColor blended;
            auto blendedIt = blended.begin();
            auto leftIt = left.begin();
            auto rightIt = right.begin();
            for (; blendedIt != blended.end(); ++blendedIt, ++leftIt, ++rightIt)
            {
                const SignedWide delta = static_cast<SignedWide>(*rightIt) - static_cast<SignedWide>(*leftIt);
                const SignedWide step = ((delta * static_cast<SignedWide>(progress)) + static_cast<SignedWide>(1)) >> 8;
                *blendedIt = static_cast<ComponentType>(static_cast<SignedWide>(*leftIt) + step);
            }

            return blended;
        }

        static constexpr TColor bilinearBlend(const TColor &c00,
                                              const TColor &c01,
                                              const TColor &c10,
                                              const TColor &c11,
                                              float x,
                                              float y)
        {
            const float v00 = (1.0f - x) * (1.0f - y);
            const float v10 = x * (1.0f - y);
            const float v01 = (1.0f - x) * y;
            const float v11 = x * y;

            TColor blended;
            auto blendedIt = blended.begin();
            auto c00It = c00.begin();
            auto c01It = c01.begin();
            auto c10It = c10.begin();
            auto c11It = c11.begin();
            for (; blendedIt != blended.end(); ++blendedIt, ++c00It, ++c01It, ++c10It, ++c11It)
            {
                const float value = static_cast<float>(*c00It) * v00
                    + static_cast<float>(*c10It) * v10
                    + static_cast<float>(*c01It) * v01
                    + static_cast<float>(*c11It) * v11;
                *blendedIt = static_cast<ComponentType>(value);
            }

            return blended;
        }
    };
}

#ifndef NEOPIXELBUS_COLOR_MATH_BACKEND
#define NEOPIXELBUS_COLOR_MATH_BACKEND lw::detail::ScalarColorMathBackend
#endif


