#pragma once

#include <cstdint>

namespace npb
{

template<typename T_METHOD>
struct GammaInvertMethod
{
    static constexpr uint8_t correct(uint8_t value)
    {
        return static_cast<uint8_t>(~T_METHOD::correct(value));
    }
};

} // namespace npb
