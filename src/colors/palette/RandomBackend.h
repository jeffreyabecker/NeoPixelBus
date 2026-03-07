#pragma once

#include <cstdint>

namespace lw::colors::palettes::detail::palettegen
{
struct XorShift32RandomBackend
{
    static constexpr uint32_t next(uint32_t& state)
    {
        if (state == 0u)
        {
            state = 0x6D2B79F5u;
        }

        state ^= (state << 13u);
        state ^= (state >> 17u);
        state ^= (state << 5u);
        return state;
    }
};
} // namespace lw::colors::palettes::detail::palettegen
