#pragma once

#include <cstdint>
#include <cstddef>
#include <array>

namespace npb
{

class Color
{
public:
    std::array<uint8_t, 5> Channels{};

    static constexpr size_t ChannelCount = 5;

    // Named channel indices
    static constexpr size_t IdxR  = 0;
    static constexpr size_t IdxG  = 1;
    static constexpr size_t IdxB  = 2;
    static constexpr size_t IdxWW = 3;
    static constexpr size_t IdxCW = 4;

    constexpr Color() = default;

    constexpr Color(uint8_t r, uint8_t g, uint8_t b,
                    uint8_t ww = 0, uint8_t cw = 0)
        : Channels{r, g, b, ww, cw}
    {
    }

    constexpr uint8_t operator[](size_t idx) const
    {
        return Channels[idx];
    }

    uint8_t& operator[](size_t idx)
    {
        return Channels[idx];
    }

    constexpr bool operator==(const Color& other) const
    {
        return Channels == other.Channels;
    }
};

} // namespace npb
