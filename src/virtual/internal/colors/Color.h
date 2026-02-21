#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>

namespace npb
{

class Color
{
public:
    uint16_t R{0};
    uint16_t G{0};
    uint16_t B{0};
    uint16_t WW{0};
    uint16_t CW{0};

    static constexpr size_t ChannelCount = 5;

    constexpr Color() = default;

    constexpr Color(uint16_t r, uint16_t g, uint16_t b,
                    uint16_t ww = 0, uint16_t cw = 0)
        : R{r}, G{g}, B{b}, WW{ww}, CW{cw}
    {
    }

    static constexpr Color fromRgb8(uint8_t r, uint8_t g, uint8_t b)
    {
        return Color(expand8to16(r), expand8to16(g), expand8to16(b));
    }

    static constexpr Color fromRgbw8(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
    {
        return Color(expand8to16(r), expand8to16(g), expand8to16(b),
                     expand8to16(w));
    }

    static constexpr Color fromRgbww8(uint8_t r, uint8_t g, uint8_t b,
                                       uint8_t ww, uint8_t cw)
    {
        return Color(expand8to16(r), expand8to16(g), expand8to16(b),
                     expand8to16(ww), expand8to16(cw));
    }

    constexpr uint16_t operator[](size_t idx) const
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

    uint16_t& operator[](size_t idx)
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

private:
    static constexpr uint16_t expand8to16(uint8_t v)
    {
        return static_cast<uint16_t>((v << 8) | v);
    }
};

} // namespace npb
