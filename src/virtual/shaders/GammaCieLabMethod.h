#pragma once

#include <cstdint>
#include <cmath>

namespace npb
{

// CIE L* perceptual lightness curve.
// More perceptually uniform than simple power-law gamma.
struct GammaCieLabMethod
{
    static uint8_t correct(uint8_t value)
    {
        float unit = value / 255.0f;
        float corrected;
        if (unit <= 0.08f)
        {
            corrected = unit / 9.033f;
        }
        else
        {
            corrected = powf((unit + 0.16f) / 1.16f, 3.0f);
        }
        return static_cast<uint8_t>(corrected * 255.0f + 0.5f);
    }
};

} // namespace npb
