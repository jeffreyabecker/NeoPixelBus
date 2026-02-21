#pragma once

#include <cstdint>

namespace npb
{

struct GammaNullMethod
{
    static constexpr uint8_t correct(uint8_t value)
    {
        return value;
    }
};

} // namespace npb
