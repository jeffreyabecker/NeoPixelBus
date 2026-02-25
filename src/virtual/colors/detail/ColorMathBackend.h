#pragma once

#include <cstdint>
#include <type_traits>

#include "../Color.h"

namespace npb::detail
{
    template <typename TColor>
    struct ScalarColorMathBackend
    {
        using ComponentType = typename TColor::ComponentType;

        static constexpr void darken(TColor &color, ComponentType delta)
        {
            for (size_t idx = 0; idx < TColor::ChannelCount; ++idx)
            {
                if (color[idx] > delta)
                {
                    color[idx] = static_cast<ComponentType>(color[idx] - delta);
                }
                else
                {
                    color[idx] = static_cast<ComponentType>(0);
                }
            }
        }

        static constexpr void lighten(TColor &color, ComponentType delta)
        {
            for (size_t idx = 0; idx < TColor::ChannelCount; ++idx)
            {
                if (color[idx] < static_cast<ComponentType>(TColor::MaxComponent - delta))
                {
                    color[idx] = static_cast<ComponentType>(color[idx] + delta);
                }
                else
                {
                    color[idx] = TColor::MaxComponent;
                }
            }
        }

        static constexpr TColor linearBlend(const TColor &left, const TColor &right, float progress)
        {
            using SignedWide = std::conditional_t<(sizeof(ComponentType) <= 2), int32_t, int64_t>;

            TColor blended;
            for (size_t idx = 0; idx < TColor::ChannelCount; ++idx)
            {
                const SignedWide delta = static_cast<SignedWide>(right[idx]) - static_cast<SignedWide>(left[idx]);
                const float value = static_cast<float>(left[idx]) + (static_cast<float>(delta) * progress);
                blended[idx] = static_cast<ComponentType>(value);
            }

            return blended;
        }

        static constexpr TColor linearBlend(const TColor &left, const TColor &right, uint8_t progress)
        {
            using SignedWide = std::conditional_t<(sizeof(ComponentType) <= 2), int32_t, int64_t>;

            TColor blended;
            for (size_t idx = 0; idx < TColor::ChannelCount; ++idx)
            {
                const SignedWide delta = static_cast<SignedWide>(right[idx]) - static_cast<SignedWide>(left[idx]);
                const SignedWide step = ((delta * static_cast<SignedWide>(progress)) + static_cast<SignedWide>(1)) >> 8;
                blended[idx] = static_cast<ComponentType>(static_cast<SignedWide>(left[idx]) + step);
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
            for (size_t idx = 0; idx < TColor::ChannelCount; ++idx)
            {
                const float value = static_cast<float>(c00[idx]) * v00
                    + static_cast<float>(c10[idx]) * v10
                    + static_cast<float>(c01[idx]) * v01
                    + static_cast<float>(c11[idx]) * v11;
                blended[idx] = static_cast<ComponentType>(value);
            }

            return blended;
        }
    };
}

#ifndef NEOPIXELBUS_COLOR_MATH_BACKEND
#define NEOPIXELBUS_COLOR_MATH_BACKEND npb::detail::ScalarColorMathBackend
#endif
