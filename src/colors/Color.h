#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <limits>
#include <type_traits>

#include "core/Compat.h"
#include "colors/ChannelOrder.h"
#include "colors/ColorChannelIndexIterator.h"
#include "colors/ColorHexCodec.h"

namespace npb
{

    template <size_t NChannels, typename TComponent = uint8_t, size_t InternalSize = NChannels * sizeof(TComponent)>
    class RgbBasedColor
    {
    public:
        static constexpr size_t ChannelCount = NChannels;
        static constexpr TComponent MaxComponent = std::numeric_limits<TComponent>::max();

        using ComponentType = TComponent;
        using ChannelIndexIterator = ColorChannelIndexIterator<NChannels>;

        constexpr RgbBasedColor() = default;

        template <typename... Args,
                  typename = std::enable_if_t<(sizeof...(Args) <= NChannels) &&
                                              std::conjunction<std::is_convertible<Args, TComponent>...>::value>>
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

        constexpr TComponent operator[](char channel) const
        {
            return Channels[ColorChannelIndexRange<NChannels>::indexFromChannel(channel)];
        }

        TComponent &operator[](char channel)
        {
            return Channels[ColorChannelIndexRange<NChannels>::indexFromChannel(channel)];
        }

        constexpr TComponent *begin()
        {
            return Channels.data();
        }

        constexpr const TComponent *begin() const
        {
            return Channels.data();
        }

        constexpr const TComponent *cbegin() const
        {
            return Channels.data();
        }

        constexpr TComponent *end()
        {
            return Channels.data() + NChannels;
        }

        constexpr const TComponent *end() const
        {
            return Channels.data() + NChannels;
        }

        constexpr const TComponent *cend() const
        {
            return Channels.data() + NChannels;
        }

        constexpr bool operator==(const RgbBasedColor &other) const
        {
            return Channels == other.Channels;
        }

    private:
        std::array<TComponent, InternalSize / sizeof(TComponent)> Channels; // no {} here so we're trivially constructable

    };

    using Rgb8Color = RgbBasedColor<3, uint8_t>;
    using Rgbw8Color = RgbBasedColor<4, uint8_t>;
    using Rgbcw8Color = RgbBasedColor<5, uint8_t>;

    using Rgb16Color = RgbBasedColor<3, uint16_t>;
    using Rgbw16Color = RgbBasedColor<4, uint16_t>;
    using Rgbcw16Color = RgbBasedColor<5, uint16_t>;

    template <typename TColor, typename = void>
    struct ColorTypeImpl : std::false_type
    {
    };

    template <typename TColor>
    struct ColorTypeImpl<TColor,
                         std::void_t<typename TColor::ComponentType,
                                     decltype(TColor::ChannelCount)>>
        : std::integral_constant<bool,
                                 std::is_convertible<decltype(TColor::ChannelCount), size_t>::value>
    {
    };

    template <typename TColor>
    static constexpr bool ColorType = ColorTypeImpl<TColor>::value;

    template <typename TColor, size_t NChannels>
    static constexpr bool ColorChannelsExactly = ColorType<TColor> && (TColor::ChannelCount == NChannels);

    template <typename TColor, size_t MinChannels>
    static constexpr bool ColorChannelsAtLeast = ColorType<TColor> && (TColor::ChannelCount >= MinChannels);

    template <typename TColor, size_t MaxChannels>
    static constexpr bool ColorChannelsAtMost = ColorType<TColor> && (TColor::ChannelCount <= MaxChannels);

    template <typename TColor, size_t MinChannels, size_t MaxChannels>
    static constexpr bool ColorChannelsInRange = ColorType<TColor> && (TColor::ChannelCount >= MinChannels) && (TColor::ChannelCount <= MaxChannels);

    template <typename TColor, typename TComponent>
    static constexpr bool ColorComponentTypeIs = ColorType<TColor> && std::is_same<typename TColor::ComponentType, TComponent>::value;

    template <typename TColor, size_t BitDepth>
    static constexpr bool ColorComponentBitDepth = ColorType<TColor> && ((sizeof(typename TColor::ComponentType) * 8) == BitDepth);

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
        auto resultIt = result.begin();
        for (auto srcIt = src.begin(); srcIt != src.end(); ++srcIt, ++resultIt)
        {
            *resultIt = static_cast<uint16_t>((static_cast<uint16_t>(*srcIt) << 8) | *srcIt);
        }
        return result;
    }

    template <size_t N>
    constexpr RgbBasedColor<N, uint8_t> narrow(const RgbBasedColor<N, uint16_t> &src)
    {
        RgbBasedColor<N, uint8_t> result;
        auto resultIt = result.begin();
        for (auto srcIt = src.begin(); srcIt != src.end(); ++srcIt, ++resultIt)
        {
            *resultIt = static_cast<uint8_t>(*srcIt >> 8);
        }
        return result;
    }

    template <size_t N, size_t M, typename T,
              typename std::enable_if<(N > M), int>::type = 0>
    constexpr RgbBasedColor<N, T> expand(const RgbBasedColor<M, T> &src)
    {
        RgbBasedColor<N, T> result{};
        auto resultIt = result.begin();
        for (auto srcIt = src.begin(); srcIt != src.end(); ++srcIt, ++resultIt)
        {
            *resultIt = *srcIt;
        }
        return result;
    }

    template <size_t N, size_t M, typename T,
              typename std::enable_if<(N < M), int>::type = 0>
    constexpr RgbBasedColor<N, T> compress(const RgbBasedColor<M, T> &src)
    {
        RgbBasedColor<N, T> result;
        auto srcIt = src.begin();
        for (auto resultIt = result.begin(); resultIt != result.end(); ++resultIt, ++srcIt)
        {
            *resultIt = *srcIt;
        }
        return result;
    }

} // namespace npb
