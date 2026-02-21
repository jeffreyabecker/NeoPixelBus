#pragma once

#include <cstdint>
#include <cstring>
#include <cmath>

namespace npb
{

// Function pointer type for custom gamma curves.
// Maps [0,255] → [0,255].
using GammaCalcFunction = uint8_t(*)(uint8_t value);

class GammaDynamicTableMethod
{
public:
    // Initialize the lookup table from a custom curve function.
    // The function maps input [0,255] to output [0,255].
    static void initialize(GammaCalcFunction calc)
    {
        for (uint16_t entry = 0; entry < 256; ++entry)
        {
            _table[entry] = calc(static_cast<uint8_t>(entry));
        }
    }

    // Initialize from a float-based curve function.
    // The function maps [0.0, 1.0] → [0.0, 1.0].
    static void initializeFromUnit(float(*calc)(float))
    {
        for (uint16_t entry = 0; entry < 256; ++entry)
        {
            float unit = static_cast<float>(entry) / 255.0f;
            _table[entry] = static_cast<uint8_t>(calc(unit) * 255.0f + 0.5f);
        }
    }

    static uint8_t correct(uint8_t value)
    {
        return _table[value];
    }

private:
    static inline uint8_t _table[256] = {};
};

} // namespace npb
