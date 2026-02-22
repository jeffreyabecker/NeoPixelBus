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

} // namespace npb
