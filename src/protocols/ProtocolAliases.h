#pragma once

#include <type_traits>
#include <utility>

#include "colors/Color.h"
#include "protocols/DebugProtocol.h"
#include "protocols/DotStarProtocol.h"
#include "protocols/NilProtocol.h"
#include "protocols/Tm1814Protocol.h"
#include "protocols/Tm1914Protocol.h"
#include "protocols/Ws2812xProtocol.h"
#include "transports/OneWireEncoding.h"

namespace lw
{
namespace protocols
{

    template <typename TProtocolCandidate,
              typename = void>
    struct ResolveProtocolType
    {
        using Type = TProtocolCandidate;
    };

    template <typename TProtocolCandidate>
    struct ResolveProtocolType<TProtocolCandidate,
                               std::void_t<typename TProtocolCandidate::ProtocolType>>
    {
        using Type = typename TProtocolCandidate::ProtocolType;
    };

    template <typename TInterfaceColor = lw::Rgb8Color,
              typename TDefaultChannelOrder = lw::ChannelOrder::BGR,
              typename TStripColor = TInterfaceColor>
    struct DotStar
    {
        using InterfaceColorType = TInterfaceColor;
        using StripColorType = TStripColor;
        using DefaultChannelOrder = TDefaultChannelOrder;

        using EffectiveInterfaceColor = std::conditional_t<lw::ColorAtLeastAsLarge<TInterfaceColor, TStripColor>,
                                                           TInterfaceColor,
                                                           TStripColor>;

        using ProtocolType = lw::Apa102Protocol<EffectiveInterfaceColor, StripColorType>;
        using SettingsType = typename ProtocolType::SettingsType;
        using ColorType = typename ProtocolType::ColorType;

        static SettingsType defaultSettings()
        {
            SettingsType settings{};
            settings.channelOrder = TDefaultChannelOrder::value;
            return normalizeSettings(std::move(settings));
        }

        static SettingsType normalizeSettings(SettingsType settings)
        {
            return SettingsType::template normalizeForColor<ColorType>(std::move(settings),
                                                                       TDefaultChannelOrder::value);
        }
    };

    template <typename TInterfaceColor = lw::Rgb8Color,
              typename TDefaultChannelOrder = lw::ChannelOrder::BGR,
              typename TStripColor = TInterfaceColor>
    using DotStarType = typename DotStar<TInterfaceColor,
                                         TDefaultChannelOrder,
                                         TStripColor>::ProtocolType;

    using APA102 = DotStar<lw::Rgb8Color,
                           lw::ChannelOrder::BGR,
                           lw::Rgb8Color>;

    using APA102Type = typename APA102::ProtocolType;

    template <typename TInterfaceColor = lw::Rgb16Color,
              typename TDefaultChannelOrder = lw::ChannelOrder::BGR,
              typename TStripColor = lw::Rgb16Color>
    struct Hd108
    {
        using InterfaceColorType = TInterfaceColor;
        using StripColorType = TStripColor;
        using DefaultChannelOrder = TDefaultChannelOrder;

        using EffectiveInterfaceColor = std::conditional_t<lw::ColorAtLeastAsLarge<TInterfaceColor, TStripColor>,
                                                           TInterfaceColor,
                                                           TStripColor>;

        using ProtocolType = lw::Hd108Protocol<EffectiveInterfaceColor, StripColorType>;
        using SettingsType = typename ProtocolType::SettingsType;
        using ColorType = typename ProtocolType::ColorType;

        static SettingsType defaultSettings()
        {
            SettingsType settings{};
            settings.channelOrder = TDefaultChannelOrder::value;
            return normalizeSettings(std::move(settings));
        }

        static SettingsType normalizeSettings(SettingsType settings)
        {
            return SettingsType::template normalizeForColor<ColorType>(std::move(settings),
                                                                       TDefaultChannelOrder::value);
        }
    };

    template <typename TInterfaceColor = lw::Rgb16Color,
              typename TDefaultChannelOrder = lw::ChannelOrder::BGR,
              typename TStripColor = lw::Rgb16Color>
    using Hd108Type = typename Hd108<TInterfaceColor,
                                     TDefaultChannelOrder,
                                     TStripColor>::ProtocolType;

    using HD108 = Hd108<lw::Rgb16Color,
                        lw::ChannelOrder::BGR,
                        lw::Rgb16Color>;

    using HD108Type = typename HD108::ProtocolType;

    template <typename TColor = lw::Rgb8Color>
    struct None
    {
        using ColorType = TColor;
        using ProtocolType = lw::NilProtocol<TColor>;
        using SettingsType = typename ProtocolType::SettingsType;

        static SettingsType defaultSettings()
        {
            return SettingsType{};
        }

        static SettingsType normalizeSettings(SettingsType settings)
        {
            return settings;
        }
    };

    template <typename TColor = lw::Rgb8Color>
    using NoneType = typename None<TColor>::ProtocolType;

    template <typename TWrappedProtocolSpec = None<lw::Rgb8Color>>
    struct Debug
    {
        using WrappedProtocolType = typename ResolveProtocolType<TWrappedProtocolSpec>::Type;
        using ProtocolType = lw::DebugProtocol<WrappedProtocolType>;
        using SettingsType = typename ProtocolType::SettingsType;
        using ColorType = typename ProtocolType::ColorType;

        static SettingsType defaultSettings()
        {
            SettingsType settings{};
            settings.wrapped = normalizeWrappedSettings(settings.wrapped);
            return settings;
        }

        static SettingsType normalizeSettings(SettingsType settings)
        {
            settings.wrapped = normalizeWrappedSettings(std::move(settings.wrapped));
            return settings;
        }

    private:
        using WrappedSettingsType = typename WrappedProtocolType::SettingsType;

        template <typename TWrappedSpec,
                  typename = void>
        struct WrappedSpecHasNormalizeSettings : std::false_type
        {
        };

        template <typename TWrappedSpec>
        struct WrappedSpecHasNormalizeSettings<TWrappedSpec,
                                               std::void_t<decltype(TWrappedSpec::normalizeSettings(std::declval<WrappedSettingsType>()))>> : std::true_type
        {
        };

        static WrappedSettingsType normalizeWrappedSettings(WrappedSettingsType settings)
        {
            if constexpr (WrappedSpecHasNormalizeSettings<TWrappedProtocolSpec>::value)
            {
                return TWrappedProtocolSpec::normalizeSettings(std::move(settings));
            }

            return settings;
        }
    };

    template <typename TWrappedProtocolSpec = None<lw::Rgb8Color>>
    using DebugType = typename Debug<TWrappedProtocolSpec>::ProtocolType;

    template <typename TInterfaceColor = lw::Rgbw8Color>
    struct Tm1814
    {
        using ColorType = TInterfaceColor;
        using ProtocolType = lw::Tm1814ProtocolT<TInterfaceColor>;
        using SettingsType = typename ProtocolType::SettingsType;

        static SettingsType defaultSettings()
        {
            SettingsType settings{};
            return normalizeSettings(std::move(settings));
        }

        static SettingsType normalizeSettings(SettingsType settings)
        {
            return SettingsType::template normalizeForColor<ColorType>(std::move(settings),
                                                                       "WRGB");
        }
    };

    template <typename TInterfaceColor = lw::Rgbw8Color>
    using Tm1814Type = typename Tm1814<TInterfaceColor>::ProtocolType;

    template <typename TInterfaceColor = lw::Rgb8Color>
    struct Tm1914
    {
        using ColorType = TInterfaceColor;
        using ProtocolType = lw::Tm1914ProtocolT<TInterfaceColor>;
        using SettingsType = typename ProtocolType::SettingsType;

        static SettingsType defaultSettings()
        {
            SettingsType settings{};
            return normalizeSettings(std::move(settings));
        }

        static SettingsType normalizeSettings(SettingsType settings)
        {
            return SettingsType::template normalizeForColor<ColorType>(std::move(settings),
                                                                       lw::ChannelOrder::GRB::value);
        }
    };

    template <typename TInterfaceColor = lw::Rgb8Color>
    using Tm1914Type = typename Tm1914<TInterfaceColor>::ProtocolType;

    template <typename TInterfaceColor = lw::Color,
              typename TDefaultChannelOrder = lw::ChannelOrder::GRB,
              const OneWireTiming *TDefaultTiming = &lw::timing::Generic800,
              typename TStripColor = TInterfaceColor,
              bool TIdleHigh = false>
    struct Ws2812x
    {
        using InterfaceColorType = TInterfaceColor;
        using StripColorType = TStripColor;
        using DefaultChannelOrder = TDefaultChannelOrder;

        using EffectiveInterfaceColor = std::conditional_t<lw::ColorAtLeastAsLarge<TInterfaceColor, TStripColor>,
                                                           TInterfaceColor,
                                                           TStripColor>;

        using ProtocolType = lw::Ws2812xProtocol<EffectiveInterfaceColor, StripColorType>;
        using SettingsType = typename ProtocolType::SettingsType;
        using ColorType = typename ProtocolType::ColorType;

        static constexpr OneWireTiming defaultTiming()
        {
            return (TDefaultTiming != nullptr) ? *TDefaultTiming : lw::timing::Ws2812x;
        }

        static SettingsType defaultSettings()
        {
            SettingsType settings{};
            settings.channelOrder = TDefaultChannelOrder::value;
            settings.timing = defaultTiming();
            return normalizeSettings(std::move(settings));
        }

        static SettingsType normalizeSettings(SettingsType settings)
        {
            settings.idleHigh = static_cast<bool>(TIdleHigh);
            return SettingsType::template normalizeForColor<ColorType>(std::move(settings),
                                                                       TDefaultChannelOrder::value);
        }
    };

    template <typename TInterfaceColor = lw::Color,
              typename TDefaultChannelOrder = lw::ChannelOrder::GRB,
              const OneWireTiming *TDefaultTiming = &lw::timing::Generic800,
              typename TStripColor = TInterfaceColor,
              bool TIdleHigh = false>
    using Ws2812xType = typename Ws2812x<TInterfaceColor,
                                         TDefaultChannelOrder,
                                         TDefaultTiming,
                                         TStripColor,
                                         TIdleHigh>::ProtocolType;

    template <typename TInterfaceColor = lw::Color>
    using Ws2812 = Ws2812x<TInterfaceColor,
                           lw::ChannelOrder::GRB,
                           &lw::timing::Generic800,
                           lw::Rgb8Color,
                           false>;

    template <typename TInterfaceColor = lw::Color>
    using Ws2812Type = typename Ws2812<TInterfaceColor>::ProtocolType;

    template <typename TInterfaceColor = lw::Color>
    using Ws2811 = Ws2812x<TInterfaceColor,
                           lw::ChannelOrder::RGB,
                           &lw::timing::Ws2811,
                           lw::Rgb8Color,
                           false>;

    template <typename TInterfaceColor = lw::Color>
    using Ws2811Type = typename Ws2811<TInterfaceColor>::ProtocolType;

    template <typename TInterfaceColor = lw::Color>
    using Ws2805 = Ws2812x<TInterfaceColor,
                           lw::ChannelOrder::RGBCW,
                           &lw::timing::Ws2805,
                           lw::Rgbcw8Color,
                           false>;

    template <typename TInterfaceColor = lw::Color>
    using Ws2805Type = typename Ws2805<TInterfaceColor>::ProtocolType;

    template <typename TInterfaceColor = lw::Color>
    using Sk6812 = Ws2812x<TInterfaceColor,
                           lw::ChannelOrder::GRB,
                           &lw::timing::Sk6812,
                           lw::Rgb8Color,
                           false>;

    template <typename TInterfaceColor = lw::Color>
    using Sk6812Type = typename Sk6812<TInterfaceColor>::ProtocolType;

    template <typename TInterfaceColor = lw::Color>
    using Tm1829 = Ws2812x<TInterfaceColor,
                           lw::ChannelOrder::RGB,
                           &lw::timing::Tm1829,
                           lw::Rgb8Color,
                           true>;

    template <typename TInterfaceColor = lw::Color>
    using Tm1829Type = typename Tm1829<TInterfaceColor>::ProtocolType;

    template <typename TInterfaceColor = lw::Color>
    using Ws2814 = Ws2812x<TInterfaceColor,
                           lw::ChannelOrder::RGBW,
                           &lw::timing::Ws2814,
                           lw::Rgbw8Color,
                           false>;

    template <typename TInterfaceColor = lw::Color>
    using Ws2814Type = typename Ws2814<TInterfaceColor>::ProtocolType;

} // namespace protocols
namespace factory
{
namespace protocols
{

    using lw::protocols::ResolveProtocolType;

    using lw::protocols::DotStar;
    using lw::protocols::DotStarType;
    using lw::protocols::APA102;
    using lw::protocols::APA102Type;

    using lw::protocols::Hd108;
    using lw::protocols::Hd108Type;
    using lw::protocols::HD108;
    using lw::protocols::HD108Type;

    using lw::protocols::None;
    using lw::protocols::NoneType;
    using lw::protocols::Debug;
    using lw::protocols::DebugType;

    using lw::protocols::Tm1814;
    using lw::protocols::Tm1814Type;
    using lw::protocols::Tm1914;
    using lw::protocols::Tm1914Type;

    using lw::protocols::Ws2812x;
    using lw::protocols::Ws2812xType;
    using lw::protocols::Ws2812;
    using lw::protocols::Ws2812Type;
    using lw::protocols::Ws2811;
    using lw::protocols::Ws2811Type;
    using lw::protocols::Ws2805;
    using lw::protocols::Ws2805Type;
    using lw::protocols::Sk6812;
    using lw::protocols::Sk6812Type;
    using lw::protocols::Tm1829;
    using lw::protocols::Tm1829Type;
    using lw::protocols::Ws2814;
    using lw::protocols::Ws2814Type;

} // namespace protocols
} // namespace factory
} // namespace lw
