#pragma once

#include "colors/Color.h"
#include "transports/ITransport.h"
#include "transports/OneWireTiming.h"

namespace lw
{
namespace factory
{
namespace descriptors
{

    template <typename TColor = lw::Rgb8Color,
              typename TCapabilityRequirement = lw::TransportTag,
              typename TDefaultChannelOrder = lw::ChannelOrder::BGR>
    struct DotStar
    {
        using ColorType = TColor;
        using CapabilityRequirement = TCapabilityRequirement;
        using DefaultChannelOrder = TDefaultChannelOrder;
    };

    using APA102 = DotStar<lw::Rgb8Color,
                           lw::TransportTag,
                           lw::ChannelOrder::BGR>;

    template <typename TInterfaceColor = lw::Rgb8Color,
              typename TDefaultChannelOrder = lw::ChannelOrder::GRB,
              const OneWireTiming *TDefaultTiming = &timing::Generic800,
              typename TStripColor = TInterfaceColor,
              bool TIdleHigh = false>
    struct Ws2812x
    {
        using InterfaceColorType = TInterfaceColor;
        using StripColorType = TStripColor;
        using ColorType = InterfaceColorType;
        using CapabilityRequirement = lw::OneWireTransportTag;
        using DefaultChannelOrder = TDefaultChannelOrder;
        static constexpr const OneWireTiming *DefaultTiming = TDefaultTiming;
        static constexpr bool IdleHigh = TIdleHigh;
    };

    
    template <typename TInterfaceColor = lw::Color>
    struct Ws2812T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::GRB,
                                    &timing::Generic800,
                                    lw::Rgb8Color>
    {

    };
    using Ws2812 = Ws2812T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ws2811T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::GRB,
                                    &timing::Generic800,
                                    lw::Rgb8Color>
    {

    };
    using Ws2811 = Ws2811T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ws2805T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::GRB,
                                    &timing::Generic800,
                                    lw::Rgb8Color>
    {

    };
    using Ws2805 = Ws2805T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Sk6812T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::GRB,
                                    &timing::Sk6812,
                                    lw::Rgb8Color>
    {

    };
    using Sk6812 = Sk6812T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Tm1814T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::GRB,
                                    &timing::Generic800,
                                    lw::Rgb8Color>
    {

    };
    using Tm1814 = Tm1814T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Tm1914T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::GRB,
                                    &timing::Generic800,
                                    lw::Rgb8Color>
    {

    };
    using Tm1914 = Tm1914T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Tm1829T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::GRB,
                                    &timing::Generic800,
                                    lw::Rgb8Color>
    {

    };
    using Tm1829 = Tm1829T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Apa106T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::GRB,
                                    &timing::Apa106,
                                    lw::Rgb8Color>
    {

    };
    using Apa106 = Apa106T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Tx1812T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::GRB,
                                    &timing::Tx1812,
                                    lw::Rgb8Color>
    {

    };
    using Tx1812 = Tx1812T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Gs1903T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::GRB,
                                    &timing::Gs1903,
                                    lw::Rgb8Color>
    {
    };
    using Gs1903 = Gs1903T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Generic800T : public Ws2812x<TInterfaceColor,
                                        lw::ChannelOrder::GRB,
                                        &timing::Generic800,
                                        lw::Rgb8Color>
    {
    };
    using Generic800 = Generic800T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Generic400T : public Ws2812x<TInterfaceColor,
                                        lw::ChannelOrder::GRB,
                                        &timing::Generic400,
                                        lw::Rgb8Color>
    {
    };
    using Generic400 = Generic400T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ws2816T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::GRB,
                                    &timing::Ws2816,
                                    lw::Rgb8Color>
    {
    };
    using Ws2816 = Ws2816T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ws2813T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::GRB,
                                    &timing::Ws2813,
                                    lw::Rgb8Color>
    {
    };
    using Ws2813 = Ws2813T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ws2814T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::GRB,
                                    &timing::Ws2814,
                                    lw::Rgb8Color>
    {

    };
    using Ws2814 = Ws2814T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Lc8812T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::GRB,
                                    &timing::Lc8812,
                                    lw::Rgb8Color>
    {
    };
    using Lc8812 = Lc8812T<>;

} // namespace descriptors
} // namespace factory
} // namespace lw
