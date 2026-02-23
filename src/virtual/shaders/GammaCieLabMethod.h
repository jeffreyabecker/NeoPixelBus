#pragma once

#include <cstdint>
#include <array>
#include <cstddef>
#include <cmath>

namespace npb
{

// CIE L* perceptual lightness curve.
// More perceptually uniform than simple power-law gamma.
struct GammaCieLabMethod
{
    static constexpr float LinearThreshold = 0.08f;
    static constexpr float LinearDivisor = 9.033f;
    static constexpr float CubicOffset = 0.16f;
    static constexpr float CubicDivisor = 1.16f;

    static uint8_t correct(uint8_t value)
    {
        float unit = value / 255.0f;
        float corrected;
        if (unit <= LinearThreshold)
        {
            corrected = unit / LinearDivisor;
        }
        else
        {
            float t = (unit + CubicOffset) / CubicDivisor;
            corrected = t * t * t;
        }
        return static_cast<uint8_t>(corrected * 255.0f + 0.5f);
    }
};

struct GammaCieLabQ15Method
{
    static constexpr int32_t Q15One = 32768;

    static constexpr int32_t LinearThresholdQ15 = 2621; // 0.08 * 32768
    static constexpr int32_t LinearReciprocalQ15 = 3628; // (1 / 9.033) * 32768
    static constexpr int32_t CubicOffsetQ15 = 5243; // 0.16 * 32768
    static constexpr int32_t CubicReciprocalQ15 = 28248; // (1 / 1.16) * 32768

    static uint8_t correct(uint8_t value)
    {
        int32_t unitQ15 = (static_cast<int32_t>(value) * Q15One + 127) / 255;
        int32_t correctedQ15;

        if (unitQ15 <= LinearThresholdQ15)
        {
            correctedQ15 = (unitQ15 * LinearReciprocalQ15 + (Q15One / 2)) >> 15;
        }
        else
        {
            int32_t tQ15 = ((unitQ15 + CubicOffsetQ15) * CubicReciprocalQ15 + (Q15One / 2)) >> 15;
            int64_t cubicQ45 = static_cast<int64_t>(tQ15) * tQ15 * tQ15;
            correctedQ15 = static_cast<int32_t>((cubicQ45 + (1LL << 29)) >> 30);
        }

        if (correctedQ15 < 0)
        {
            correctedQ15 = 0;
        }
        else if (correctedQ15 > Q15One)
        {
            correctedQ15 = Q15One;
        }

        int32_t out = (correctedQ15 * 255 + (Q15One / 2)) >> 15;
        if (out < 0)
        {
            return 0;
        }
        if (out > 255)
        {
            return 255;
        }
        return static_cast<uint8_t>(out);
    }
};

struct GammaCieLabLutMethod
{
    static std::array<uint8_t, 256> makeLut()
    {
        std::array<uint8_t, 256> lut{};
        for (std::size_t index = 0; index < lut.size(); index++)
        {
            lut[index] = GammaCieLabMethod::correct(static_cast<uint8_t>(index));
        }
        return lut;
    }

    static const std::array<uint8_t, 256>& lut()
    {
        static const std::array<uint8_t, 256> Lut = makeLut();
        return Lut;
    }

    static uint8_t correct(uint8_t value)
    {
        return lut()[value];
    }
};

} // namespace npb
