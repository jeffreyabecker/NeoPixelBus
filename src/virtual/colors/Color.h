#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>

namespace npb
{

class Color
{
public:
    uint8_t R{0};
    uint8_t G{0};
    uint8_t B{0};
    uint8_t WW{0};
    uint8_t CW{0};

    static constexpr size_t ChannelCount = 5;

    constexpr Color() = default;

    constexpr Color(uint8_t r, uint8_t g, uint8_t b,
                    uint8_t ww = 0, uint8_t cw = 0)
        : R{r}, G{g}, B{b}, WW{ww}, CW{cw}
    {
    }

    constexpr uint8_t operator[](size_t idx) const
    {
        assert(idx < ChannelCount);
        switch (idx)
        {
            case 0: return R;
            case 1: return G;
            case 2: return B;
            case 3: return WW;
            case 4: return CW;
            default: return 0;  // unreachable
        }
    }

    uint8_t& operator[](size_t idx)
    {
        assert(idx < ChannelCount);
        switch (idx)
        {
            case 0: return R;
            case 1: return G;
            case 2: return B;
            case 3: return WW;
            case 4: return CW;
            default: return R;  // unreachable
        }
    }

    constexpr bool operator==(const Color&) const = default;
};

} // namespace npb
