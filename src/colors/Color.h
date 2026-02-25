#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <concepts>
#include <limits>
#include <span>
#include <string>
#include <type_traits>

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

    template <size_t NChannels, typename TComponent = uint8_t>
    class RgbBasedColor
    {
    public:
        std::array<TComponent, NChannels> Channels{};

        static constexpr size_t ChannelCount = NChannels;
        static constexpr TComponent MaxComponent = std::numeric_limits<TComponent>::max();

        using ComponentType = TComponent;

        constexpr RgbBasedColor() = default;

        template <typename... Args>
            requires((sizeof...(Args) <= NChannels) && (std::convertible_to<Args, TComponent> && ...))
        constexpr RgbBasedColor(Args... args)
            : Channels{}
        {
            constexpr size_t ArgCount = sizeof...(Args);
            const std::array<TComponent, ArgCount> values{static_cast<TComponent>(args)...};

            for (size_t idx = 0; idx < ArgCount; ++idx)
            {
                Channels[idx] = values[idx];
            }
        }

        template <typename TIndex>
            requires(std::integral<TIndex> && !std::same_as<std::remove_cv_t<TIndex>, char>)
        constexpr TComponent operator[](TIndex idx) const
        {
            return Channels[static_cast<size_t>(idx)];
        }

        template <typename TIndex>
            requires(std::integral<TIndex> && !std::same_as<std::remove_cv_t<TIndex>, char>)
        TComponent &operator[](TIndex idx)
        {
            return Channels[static_cast<size_t>(idx)];
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
                return (NChannels > 3) ? 3 : 0;

            case 'C':
            case 'c':
                return (NChannels > 4) ? 4 : 0;

            default:
                return 0;
            }
        }

        constexpr TComponent operator[](char channel) const
        {
            return Channels[indexFromChannel(channel)];
        }

        TComponent &operator[](char channel)
        {
            return Channels[indexFromChannel(channel)];
        }

        constexpr bool operator==(const RgbBasedColor &other) const
        {
            return Channels == other.Channels;
        }

        static RgbBasedColor parseHex(const char *str, const char *colorOrder = nullptr)
        {
            RgbBasedColor result{};
            if (str == nullptr)
            {
                return result;
            }

            if (colorOrder == nullptr)
            {
                colorOrder = defaultColorOrder();
            }

            if (colorOrder == nullptr)
            {
                return result;
            }

            const char *cursor = str;
            if (*cursor == '#')
            {
                ++cursor;
            }
            else if (cursor[0] == '0' && (cursor[1] == 'x' || cursor[1] == 'X'))
            {
                cursor += 2;
            }

            constexpr size_t DigitsPerComponent = sizeof(TComponent) * 2;

            for (size_t logicalChannel = 0; logicalChannel < NChannels; ++logicalChannel)
            {
                const char channelTag = colorOrder[logicalChannel];
                if (channelTag == '\0' || !isSupportedChannelTag(channelTag))
                {
                    return RgbBasedColor{};
                }

                const size_t channelIndex = indexFromChannel(channelTag);

                if (channelIndex >= NChannels)
                {
                    return RgbBasedColor{};
                }

                TComponent value = 0;
                for (size_t digit = 0; digit < DigitsPerComponent; ++digit)
                {
                    while (isHexSeparator(*cursor))
                    {
                        ++cursor;
                    }

                    const int nibble = hexNibble(*cursor);
                    if (nibble < 0)
                    {
                        return RgbBasedColor{};
                    }

                    value = static_cast<TComponent>((value << 4) | static_cast<TComponent>(nibble));
                    ++cursor;
                }

                result[channelIndex] = value;
            }

            return result;
        }

        void fillHex(std::span<uint8_t> resultBuffer,
                     const char *colorOrder = nullptr,
                     const char *prefix = nullptr) const
        {
            if (resultBuffer.empty())
            {
                return;
            }

            for (size_t idx = 0; idx < resultBuffer.size(); ++idx)
            {
                resultBuffer[idx] = 0;
            }

            if (colorOrder == nullptr)
            {
                colorOrder = defaultColorOrder();
            }

            if (colorOrder == nullptr)
            {
                return;
            }

            size_t out = 0;

            if (prefix != nullptr)
            {
                for (size_t idx = 0; prefix[idx] != '\0' && out < resultBuffer.size(); ++idx)
                {
                    resultBuffer[out++] = static_cast<uint8_t>(prefix[idx]);
                }
            }

            constexpr size_t DigitsPerComponent = sizeof(TComponent) * 2;
            for (size_t logicalChannel = 0; logicalChannel < NChannels; ++logicalChannel)
            {
                const char channelTag = colorOrder[logicalChannel];
                if (channelTag == '\0' || !isSupportedChannelTag(channelTag))
                {
                    return;
                }

                const size_t channelIndex = indexFromChannel(channelTag);
                if (channelIndex >= NChannels)
                {
                    return;
                }

                const TComponent value = Channels[channelIndex];
                for (size_t digit = 0; digit < DigitsPerComponent; ++digit)
                {
                    if (out >= resultBuffer.size())
                    {
                        return;
                    }

                    const size_t nibbleShift = (DigitsPerComponent - 1 - digit) * 4;
                    const uint8_t nibble = static_cast<uint8_t>((value >> nibbleShift) & static_cast<TComponent>(0x0F));
                    resultBuffer[out++] = static_cast<uint8_t>(hexChar(nibble));
                }
            }
        }

    private:
        static constexpr char hexChar(uint8_t nibble)
        {
            return (nibble < 10) ? static_cast<char>('0' + nibble) : static_cast<char>('A' + (nibble - 10));
        }

        static constexpr bool isHexSeparator(char value)
        {
            return value == ' ' || value == '_' || value == ':' || value == '-';
        }

        static constexpr int hexNibble(char value)
        {
            if (value >= '0' && value <= '9')
            {
                return value - '0';
            }

            if (value >= 'a' && value <= 'f')
            {
                return 10 + (value - 'a');
            }

            if (value >= 'A' && value <= 'F')
            {
                return 10 + (value - 'A');
            }

            return -1;
        }

        static constexpr bool isSupportedChannelTag(char channel)
        {
            switch (channel)
            {
            case 'R':
            case 'r':
            case 'G':
            case 'g':
            case 'B':
            case 'b':
                return true;

            case 'W':
            case 'w':
                return NChannels >= 4;

            case 'C':
            case 'c':
                return NChannels >= 5;

            default:
                return false;
            }
        }

        static constexpr const char *defaultColorOrder()
        {
            if constexpr (NChannels >= 5)
            {
                return ChannelOrder::RGBCW;
            }

            if constexpr (NChannels == 4)
            {
                return ChannelOrder::RGBW;
            }

            return ChannelOrder::RGB;
        }
    };

    using Rgb8Color = RgbBasedColor<3, uint8_t>;
    using Rgbw8Color = RgbBasedColor<4, uint8_t>;
    using Rgbcw8Color = RgbBasedColor<5, uint8_t>;

    using Rgb16Color = RgbBasedColor<3, uint16_t>;
    using Rgbw16Color = RgbBasedColor<4, uint16_t>;
    using Rgbcw16Color = RgbBasedColor<5, uint16_t>;

    template <typename TColor>
    concept ColorType = requires {
        typename TColor::ComponentType;
        { TColor::ChannelCount } -> std::convertible_to<size_t>;
    };

    template <typename TColor, size_t NChannels>
    concept ColorChannelsExactly = ColorType<TColor> && (TColor::ChannelCount == NChannels);

    template <typename TColor, size_t MinChannels>
    concept ColorChannelsAtLeast = ColorType<TColor> && (TColor::ChannelCount >= MinChannels);

    template <typename TColor, size_t MaxChannels>
    concept ColorChannelsAtMost = ColorType<TColor> && (TColor::ChannelCount <= MaxChannels);

    template <typename TColor, size_t MinChannels, size_t MaxChannels>
    concept ColorChannelsInRange = ColorType<TColor> && (TColor::ChannelCount >= MinChannels) && (TColor::ChannelCount <= MaxChannels);

    template <typename TColor, typename TComponent>
    concept ColorComponentTypeIs = ColorType<TColor> && std::same_as<typename TColor::ComponentType, TComponent>;

    template <typename TColor, size_t BitDepth>
    concept ColorComponentBitDepth = ColorType<TColor> && ((sizeof(typename TColor::ComponentType) * 8) == BitDepth);

    template <typename TColor, size_t NChannels>
    using RequireColorChannelsExactly = std::enable_if_t<ColorChannelsExactly<TColor, NChannels>, int>;

    template <typename TColor, size_t MinChannels, size_t MaxChannels>
    using RequireColorChannelsInRange = std::enable_if_t<ColorChannelsInRange<TColor, MinChannels, MaxChannels>, int>;

    template <typename TColor, size_t BitDepth>
    using RequireColorComponentBitDepth = std::enable_if_t<ColorComponentBitDepth<TColor, BitDepth>, int>;

    template <size_t N>
    constexpr RgbBasedColor<N, uint16_t> widen(const RgbBasedColor<N, uint8_t> &src)
    {
        RgbBasedColor<N, uint16_t> result;
        for (size_t ch = 0; ch < N; ++ch)
        {
            result[ch] = (static_cast<uint16_t>(src[ch]) << 8) | src[ch];
        }
        return result;
    }

    template <size_t N>
    constexpr RgbBasedColor<N, uint8_t> narrow(const RgbBasedColor<N, uint16_t> &src)
    {
        RgbBasedColor<N, uint8_t> result;
        for (size_t ch = 0; ch < N; ++ch)
        {
            result[ch] = static_cast<uint8_t>(src[ch] >> 8);
        }
        return result;
    }

    template <size_t N, size_t M, typename T,
              typename std::enable_if<(N > M), int>::type = 0>
    constexpr RgbBasedColor<N, T> expand(const RgbBasedColor<M, T> &src)
    {
        RgbBasedColor<N, T> result;
        for (size_t ch = 0; ch < M; ++ch)
        {
            result[ch] = src[ch];
        }
        return result;
    }

    template <size_t N, size_t M, typename T,
              typename std::enable_if<(N < M), int>::type = 0>
    constexpr RgbBasedColor<N, T> compress(const RgbBasedColor<M, T> &src)
    {
        RgbBasedColor<N, T> result;
        for (size_t ch = 0; ch < N; ++ch)
        {
            result[ch] = src[ch];
        }
        return result;
    }

} // namespace npb

