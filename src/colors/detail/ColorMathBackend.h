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
            for (auto channel : TColor::channelIndexes())
            {
                auto &component = color[channel];
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
            for (auto channel : TColor::channelIndexes())
            {
                auto &component = color[channel];
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
            for (auto channel : TColor::channelIndexes())
            {
                const SignedWide delta = static_cast<SignedWide>(right[channel]) - static_cast<SignedWide>(left[channel]);
                const float value = static_cast<float>(left[channel]) + (static_cast<float>(delta) * progress);
                blended[channel] = static_cast<ComponentType>(value);
            }

            return blended;
        }

        static constexpr TColor linearBlend(const TColor &left, const TColor &right, uint8_t progress)
        {
            using SignedWide = std::conditional_t<(sizeof(ComponentType) <= 2), int32_t, int64_t>;

            TColor blended;
            for (auto channel : TColor::channelIndexes())
            {
                const SignedWide delta = static_cast<SignedWide>(right[channel]) - static_cast<SignedWide>(left[channel]);
                const SignedWide step = ((delta * static_cast<SignedWide>(progress)) + static_cast<SignedWide>(1)) >> 8;
                blended[channel] = static_cast<ComponentType>(static_cast<SignedWide>(left[channel]) + step);
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
            for (auto channel : TColor::channelIndexes())
            {
                const float value = static_cast<float>(c00[channel]) * v00
                    + static_cast<float>(c10[channel]) * v10
                    + static_cast<float>(c01[channel]) * v01
                    + static_cast<float>(c11[channel]) * v11;
                blended[channel] = static_cast<ComponentType>(value);
            }

            return blended;
        }
    };
}

#ifndef LW_COLOR_MATH_BACKEND
#define LW_COLOR_MATH_BACKEND lw::detail::ScalarColorMathBackend
#endif


