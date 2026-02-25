#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <concepts>
#include <limits>
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
    class BasicColor
    {
    public:
        std::array<TComponent, NChannels> Channels{};

        static constexpr size_t ChannelCount = NChannels;
        static constexpr TComponent MaxComponent = std::numeric_limits<TComponent>::max();

        using ComponentType = TComponent;

        constexpr BasicColor() = default;

        template <typename... Args>
            requires ((sizeof...(Args) <= NChannels) && (std::convertible_to<Args, TComponent> && ...))
        constexpr BasicColor(Args... args)
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
            requires (std::integral<TIndex> && !std::same_as<std::remove_cv_t<TIndex>, char>)
        constexpr TComponent operator[](TIndex idx) const
        {
            return Channels[static_cast<size_t>(idx)];
        }

        template <typename TIndex>
            requires (std::integral<TIndex> && !std::same_as<std::remove_cv_t<TIndex>, char>)
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

        constexpr bool operator==(const BasicColor &other) const
        {
            return Channels == other.Channels;
        }
    };

    using Rgb8Color = BasicColor<3, uint8_t>;
    using Rgbw8Color = BasicColor<4, uint8_t>;
    using Rgbcw8Color = BasicColor<5, uint8_t>;

    using Rgb16Color = BasicColor<3, uint16_t>;
    using Rgbw16Color = BasicColor<4, uint16_t>;
    using Rgbcw16Color = BasicColor<5, uint16_t>;

    template<typename TColor>
    concept ColorType = requires
    {
        typename TColor::ComponentType;
        { TColor::ChannelCount } -> std::convertible_to<size_t>;
    };

    template<typename TColor, size_t NChannels>
    concept ColorChannelsExactly = ColorType<TColor> && (TColor::ChannelCount == NChannels);

    template<typename TColor, size_t MinChannels>
    concept ColorChannelsAtLeast = ColorType<TColor> && (TColor::ChannelCount >= MinChannels);

    template<typename TColor, size_t MaxChannels>
    concept ColorChannelsAtMost = ColorType<TColor> && (TColor::ChannelCount <= MaxChannels);

    template<typename TColor, size_t MinChannels, size_t MaxChannels>
    concept ColorChannelsInRange = ColorType<TColor>
        && (TColor::ChannelCount >= MinChannels)
        && (TColor::ChannelCount <= MaxChannels);

    template<typename TColor, typename TComponent>
    concept ColorComponentTypeIs = ColorType<TColor>
        && std::same_as<typename TColor::ComponentType, TComponent>;

    template<typename TColor, size_t BitDepth>
    concept ColorComponentBitDepth = ColorType<TColor>
        && ((sizeof(typename TColor::ComponentType) * 8) == BitDepth);

    template<typename TColor, size_t NChannels>
    using RequireColorChannelsExactly = std::enable_if_t<ColorChannelsExactly<TColor, NChannels>, int>;

    template<typename TColor, size_t MinChannels, size_t MaxChannels>
    using RequireColorChannelsInRange = std::enable_if_t<ColorChannelsInRange<TColor, MinChannels, MaxChannels>, int>;

    template<typename TColor, size_t BitDepth>
    using RequireColorComponentBitDepth = std::enable_if_t<ColorComponentBitDepth<TColor, BitDepth>, int>;

    template <size_t N>
    constexpr BasicColor<N, uint16_t> widen(const BasicColor<N, uint8_t> &src)
    {
        BasicColor<N, uint16_t> result;
        for (size_t ch = 0; ch < N; ++ch)
        {
            result[ch] = (static_cast<uint16_t>(src[ch]) << 8) | src[ch];
        }
        return result;
    }

    template <size_t N>
    constexpr BasicColor<N, uint8_t> narrow(const BasicColor<N, uint16_t> &src)
    {
        BasicColor<N, uint8_t> result;
        for (size_t ch = 0; ch < N; ++ch)
        {
            result[ch] = static_cast<uint8_t>(src[ch] >> 8);
        }
        return result;
    }

    template <size_t N, size_t M, typename T,
              typename std::enable_if<(N > M), int>::type = 0>
    constexpr BasicColor<N, T> expand(const BasicColor<M, T> &src)
    {
        BasicColor<N, T> result;
        for (size_t ch = 0; ch < M; ++ch)
        {
            result[ch] = src[ch];
        }
        return result;
    }

    template <size_t N, size_t M, typename T,
              typename std::enable_if<(N < M), int>::type = 0>
    constexpr BasicColor<N, T> compress(const BasicColor<M, T> &src)
    {
        BasicColor<N, T> result;
        for (size_t ch = 0; ch < N; ++ch)
        {
            result[ch] = src[ch];
        }
        return result;
    }

} // namespace npb
