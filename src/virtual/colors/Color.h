#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <string>

namespace npb
{

    namespace ChannelOrder
    {
        inline constexpr const char *RGB = "RGB";
        inline constexpr const char *GRB = "GRB";
        inline constexpr const char *BGR = "BGR";

        inline constexpr const char *RGBW = "RGBW";
        inline constexpr const char *GRBW = "GRBW";
        inline constexpr const char *BGRW = "BGRW";

        inline constexpr const char *RGBCW = "RGBCW";
        inline constexpr const char *GRBCW = "GRBCW";
        inline constexpr const char *BGRCW = "BGRCW";

        inline constexpr size_t LengthRGB = std::char_traits<char>::length(RGB);
        inline constexpr size_t LengthGRB = std::char_traits<char>::length(GRB);
        inline constexpr size_t LengthBGR = std::char_traits<char>::length(BGR);

        inline constexpr size_t LengthRGBW = std::char_traits<char>::length(RGBW);
        inline constexpr size_t LengthGRBW = std::char_traits<char>::length(GRBW);
        inline constexpr size_t LengthBGRW = std::char_traits<char>::length(BGRW);

        inline constexpr size_t LengthRGBCW = std::char_traits<char>::length(RGBCW);
        inline constexpr size_t LengthGRBCW = std::char_traits<char>::length(GRBCW);
        inline constexpr size_t LengthBGRCW = std::char_traits<char>::length(BGRCW);
    }

    class Color
    {
    public:
        std::array<uint8_t, 5> Channels{};

        static constexpr size_t ChannelCount = 5;

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

        uint8_t &operator[](size_t idx)
        {
            return Channels[idx];
        }

        constexpr uint8_t operator[](uint8_t idx) const
        {
            return Channels[idx];
        }

        uint8_t &operator[](uint8_t idx)
        {
            return Channels[idx];
        }

        static constexpr size_t indexFromChannel(char channel)
        {
            switch (channel)
            {
            case 'R':
            case 'r':
                return 0;

            case 'G':
            case 'g':
                return 1;

            case 'B':
            case 'b':
                return 2;

            case 'W':
            case 'w':
                return 3;

            case 'C':
            case 'c':
                return 4;

            default:
                return 0;
            }
        }

        constexpr uint8_t operator[](char channel) const
        {
            return Channels[indexFromChannel(channel)];
        }

        uint8_t &operator[](char channel)
        {
            return Channels[indexFromChannel(channel)];
        }

        constexpr bool operator==(const Color &other) const
        {
            return Channels == other.Channels;
        }
    };

} // namespace npb
