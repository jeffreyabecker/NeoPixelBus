#pragma once

#include <cstdint>
#include <cmath>

namespace npb
{

// Uses the standard gamma equation: pow(value, 1/0.45)
// No lookup table — slower but uses no memory.
struct GammaEquationMethod
{
    static uint8_t correct(uint8_t value)
    {
        return static_cast<uint8_t>(255.0f * powf(value / 255.0f, 1.0f / 0.45f) + 0.5f);
    }
};

// Rudimentary fixed-point approximation of the gamma equation.
// Approximates x^(1/0.45) ~= x^2.25 as x^2 * fourthRoot(x) using Q15 math.
struct GammaQ15EquationMethod
{
    static constexpr int32_t Q15One = 32768;

    static uint8_t correct(uint8_t value)
    {
        int32_t unitQ15 = (static_cast<int32_t>(value) * Q15One + 127) / 255;

        int32_t squareQ15 = mulQ15(unitQ15, unitQ15);
        int32_t rootQ15 = sqrtQ15(unitQ15);
        int32_t fourthRootQ15 = sqrtQ15(rootQ15);
        int32_t correctedQ15 = mulQ15(squareQ15, fourthRootQ15);

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

private:
    static int32_t mulQ15(int32_t aQ15, int32_t bQ15)
    {
        return static_cast<int32_t>((static_cast<int64_t>(aQ15) * bQ15 + (Q15One / 2)) >> 15);
    }

    static int32_t sqrtQ15(int32_t valueQ15)
    {
        if (valueQ15 <= 0)
        {
            return 0;
        }

        uint32_t radicand = static_cast<uint32_t>(valueQ15) * static_cast<uint32_t>(Q15One);
        return static_cast<int32_t>(isqrt32(radicand));
    }

    static uint32_t isqrt32(uint32_t value)
    {
        uint32_t result = 0;
        uint32_t bit = 1uL << 30;

        while (bit > value)
        {
            bit >>= 2;
        }

        while (bit != 0)
        {
            if (value >= result + bit)
            {
                value -= result + bit;
                result = (result >> 1) + bit;
            }
            else
            {
                result >>= 1;
            }
            bit >>= 2;
        }

        return result;
    }
};

} // namespace npb
