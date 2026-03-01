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

namespace lw
{
    template <typename TComponent>
    struct InternalStorageComponent
    {
        using type = TComponent;
    };

#if defined(LW_COLOR_UNIFIED_INTERNAL_COMPONENT_SIZE)
    template <>
    struct InternalStorageComponent<uint8_t>
    {
        using type = uint16_t;
    };
#endif

    static constexpr size_t Rgb16DefaultInternalSize = 3 * sizeof(uint16_t);
    static constexpr size_t Rgbw16DefaultInternalSize = 4 * sizeof(uint16_t);
    static constexpr size_t Rgbcw16DefaultInternalSize = 5 * sizeof(uint16_t);

    static constexpr size_t Rgb8DefaultInternalSize = 3 * sizeof(typename InternalStorageComponent<uint8_t>::type);
    static constexpr size_t Rgbw8DefaultInternalSize = 4 * sizeof(typename InternalStorageComponent<uint8_t>::type);
    static constexpr size_t Rgbcw8DefaultInternalSize = 5 * sizeof(typename InternalStorageComponent<uint8_t>::type);



#if defined(LW_COLOR_UNIFIED_8BIT_INTERNAL_SIZE_RGBCW)
    static constexpr size_t Rgb8AliasInternalSize = Rgbcw8DefaultInternalSize;
    static constexpr size_t Rgbw8AliasInternalSize = Rgbcw8DefaultInternalSize;
    static constexpr size_t Rgbcw8AliasInternalSize = Rgbcw8DefaultInternalSize;
#else
    static constexpr size_t Rgb8AliasInternalSize = Rgb8DefaultInternalSize;
    static constexpr size_t Rgbw8AliasInternalSize = Rgbw8DefaultInternalSize;
    static constexpr size_t Rgbcw8AliasInternalSize = Rgbcw8DefaultInternalSize;
#endif

#if defined(LW_COLOR_UNIFIED_16BIT_INTERNAL_SIZE_RGBCW)
    static constexpr size_t Rgb16AliasInternalSize = Rgbcw16DefaultInternalSize;
    static constexpr size_t Rgbw16AliasInternalSize = Rgbcw16DefaultInternalSize;
    static constexpr size_t Rgbcw16AliasInternalSize = Rgbcw16DefaultInternalSize;
#else
    static constexpr size_t Rgb16AliasInternalSize = Rgb16DefaultInternalSize;
    static constexpr size_t Rgbw16AliasInternalSize = Rgbw16DefaultInternalSize;
    static constexpr size_t Rgbcw16AliasInternalSize = Rgbcw16DefaultInternalSize;
#endif

    template <size_t NChannels,
              typename TComponent = uint8_t,
              size_t InternalSize = NChannels * sizeof(typename InternalStorageComponent<TComponent>::type)>
    class RgbBasedColor
    {
    public:
        using ComponentType = TComponent;
        using InternalComponentType = typename InternalStorageComponent<TComponent>::type;

        class ComponentReference
        {
        public:
            explicit constexpr ComponentReference(InternalComponentType &value)
                : _value(value)
            {
            }

            constexpr ComponentReference &operator=(TComponent value)
            {
                _value = static_cast<InternalComponentType>(value);
                return *this;
            }

            constexpr ComponentReference &operator=(const ComponentReference &other)
            {
                _value = other._value;
                return *this;
            }

            constexpr operator TComponent() const
            {
                return static_cast<TComponent>(_value);
            }

        private:
            InternalComponentType &_value;
        };

        static_assert((InternalSize % sizeof(InternalComponentType)) == 0,
                      "RgbBasedColor InternalSize must be a multiple of component size.");
        static_assert(InternalSize >= (NChannels * sizeof(InternalComponentType)),
                      "RgbBasedColor InternalSize must be >= channel storage size.");

        static constexpr size_t ChannelCount = NChannels;
        static constexpr TComponent MaxComponent = std::numeric_limits<TComponent>::max();

        using ChannelIndexIterator = ColorChannelIndexIterator<NChannels>;
        using ChannelIndexRange = ColorChannelIndexRange<NChannels>;

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
            return static_cast<TComponent>(Channels[ColorChannelIndexRange<NChannels>::indexFromChannel(channel)]);
        }

        template <typename T = InternalComponentType>
        std::enable_if_t<std::is_same<T, TComponent>::value, TComponent &> operator[](char channel)
        {
            return Channels[ColorChannelIndexRange<NChannels>::indexFromChannel(channel)];
        }

        template <typename T = InternalComponentType>
        std::enable_if_t<!std::is_same<T, TComponent>::value, ComponentReference> operator[](char channel)
        {
            return ComponentReference(Channels[ColorChannelIndexRange<NChannels>::indexFromChannel(channel)]);
        }

        static constexpr ChannelIndexRange channelIndexes()
        {
            return makeColorChannelIndexRange<NChannels>();
        }

        static constexpr size_t channelIndexFromTag(char channel)
        {
            return ColorChannelIndexRange<NChannels>::indexFromChannel(channel);
        }

        constexpr TComponent channelAtIndex(size_t index) const
        {
            return static_cast<TComponent>(Channels[index]);
        }

        template <typename T = InternalComponentType>
        std::enable_if_t<std::is_same<T, TComponent>::value, TComponent &> channelAtIndex(size_t index)
        {
            return Channels[index];
        }

        template <typename T = InternalComponentType>
        std::enable_if_t<!std::is_same<T, TComponent>::value, ComponentReference> channelAtIndex(size_t index)
        {
            return ComponentReference(Channels[index]);
        }

        constexpr bool operator==(const RgbBasedColor &other) const
        {
            return Channels == other.Channels;
        }

    private:
        std::array<InternalComponentType, InternalSize / sizeof(InternalComponentType)> Channels; // no {} here so we're trivially constructable

    };

    using Rgb8Color = RgbBasedColor<3, uint8_t, Rgb8AliasInternalSize>;
    using Rgbw8Color = RgbBasedColor<4, uint8_t, Rgbw8AliasInternalSize>;
    using Rgbcw8Color = RgbBasedColor<5, uint8_t, Rgbcw8AliasInternalSize>;



    using Rgb16Color = RgbBasedColor<3, uint16_t, Rgb16AliasInternalSize>;
    using Rgbw16Color = RgbBasedColor<4, uint16_t, Rgbw16AliasInternalSize>;
    using Rgbcw16Color = RgbBasedColor<5, uint16_t, Rgbcw16AliasInternalSize>;

    using Color = Rgbcw8Color;

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

    template <size_t N, size_t InternalSize>
    constexpr RgbBasedColor<N, uint16_t> widen(const RgbBasedColor<N, uint8_t, InternalSize> &src)
    {
        RgbBasedColor<N, uint16_t> result;
        for (auto channel : RgbBasedColor<N, uint8_t, InternalSize>::channelIndexes())
        {
            const uint8_t value = src[channel];
            result[channel] = static_cast<uint16_t>((static_cast<uint16_t>(value) << 8) | value);
        }
        return result;
    }

    template <size_t N, size_t InternalSize>
    constexpr RgbBasedColor<N, uint8_t> narrow(const RgbBasedColor<N, uint16_t, InternalSize> &src)
    {
        RgbBasedColor<N, uint8_t> result;
        for (auto channel : RgbBasedColor<N, uint16_t, InternalSize>::channelIndexes())
        {
            result[channel] = static_cast<uint8_t>(src[channel] >> 8);
        }
        return result;
    }

    template <size_t N, size_t M, typename T, size_t SrcInternalSize,
              typename std::enable_if<(N > M), int>::type = 0>
    constexpr RgbBasedColor<N, T> expand(const RgbBasedColor<M, T, SrcInternalSize> &src)
    {
        RgbBasedColor<N, T> result{};
        for (auto channel : RgbBasedColor<M, T, SrcInternalSize>::channelIndexes())
        {
            result[channel] = src[channel];
        }
        return result;
    }

    template <size_t N, size_t M, typename T, size_t SrcInternalSize,
              typename std::enable_if<(N < M), int>::type = 0>
    constexpr RgbBasedColor<N, T> compress(const RgbBasedColor<M, T, SrcInternalSize> &src)
    {
        RgbBasedColor<N, T> result;
        for (auto channel : RgbBasedColor<N, T>::channelIndexes())
        {
            result[channel] = src[channel];
        }
        return result;
    }

} // namespace lw
