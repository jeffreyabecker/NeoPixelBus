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
    template <typename TInterfaceColor = lw::Rgb8Color,
              typename TCapabilityRequirement = lw::TransportTag,
              typename TDefaultChannelOrder = lw::ChannelOrder::BGR,
              typename TStripColor = TInterfaceColor>
    struct DotStar
    {
        using InterfaceColorType = TInterfaceColor;
        using StripColorType = TStripColor;
        using ColorType = InterfaceColorType;
        using CapabilityRequirement = TCapabilityRequirement;
        using DefaultChannelOrder = TDefaultChannelOrder;
    };

    template <typename TInterfaceColor = lw::Rgb8Color,
              typename TCapabilityRequirement = lw::TransportTag,
              typename TDefaultChannelOrder = lw::ChannelOrder::BGR,
              typename TStripColor = TInterfaceColor>
    using DotStarx = DotStar<TInterfaceColor,
                             TCapabilityRequirement,
                             TDefaultChannelOrder,
                             TStripColor>;

    using APA102 = DotStar<lw::Rgb8Color,
                           lw::TransportTag,
                           lw::ChannelOrder::BGR,
                           lw::Rgb8Color>;


    template <typename TInterfaceColor = lw::Color,
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
    struct Ws2812BT : public Ws2812x<TInterfaceColor,
                                     lw::ChannelOrder::GRB,
                                     &timing::Generic800,
                                     lw::Rgb8Color>
    {
    };
    using Ws2812B = Ws2812BT<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ws2812CT : public Ws2812x<TInterfaceColor,
                                     lw::ChannelOrder::GRB,
                                     &timing::Generic800,
                                     lw::Rgb8Color>
    {
    };
    using Ws2812C = Ws2812CT<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ws2811T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::RGB,
                                    &timing::Ws2811,
                                    lw::Rgb8Color>
    {

    };
    using Ws2811 = Ws2811T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ws2811CT : public Ws2812x<TInterfaceColor,
                                     lw::ChannelOrder::RGB,
                                     &timing::Ws2811,
                                     lw::Rgb8Color>
    {
    };
    using Ws2811C = Ws2811CT<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ws2805T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::RGBCW,
                                    &timing::Ws2805,
                                    lw::Rgbcw8Color>
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
                                    lw::ChannelOrder::RGB,
                                    &timing::Ws2813,
                                    lw::Rgb8Color>
    {
    };
    using Ws2813 = Ws2813T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ws2813RgbwT : public Ws2812x<TInterfaceColor,
                                         lw::ChannelOrder::GRBW,
                                         &timing::Ws2813,
                                         lw::Rgbw8Color>
    {
    };
    using Ws2813Rgbw = Ws2813RgbwT<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ws2814T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::RGBW,
                                    &timing::Ws2814,
                                    lw::Rgbw8Color>
    {

    };
    using Ws2814 = Ws2814T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ws2814AT : public Ws2812x<TInterfaceColor,
                                     lw::ChannelOrder::WRGB,
                                     &timing::Ws2814,
                                     lw::Rgbw8Color>
    {
    };
    using Ws2814A = Ws2814AT<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ws2814BT : public Ws2812x<TInterfaceColor,
                                     lw::ChannelOrder::RGBW,
                                     &timing::Ws2814,
                                     lw::Rgbw8Color>
    {
    };
    using Ws2814B = Ws2814BT<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ws2814CT : public Ws2812x<TInterfaceColor,
                                     lw::ChannelOrder::RGBW,
                                     &timing::Ws2814,
                                     lw::Rgbw8Color>
    {
    };
    using Ws2814C = Ws2814CT<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ws2815T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::GRB,
                                    &timing::Generic800,
                                    lw::Rgb8Color>
    {
    };
    using Ws2815 = Ws2815T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ws2815BT : public Ws2812x<TInterfaceColor,
                                     lw::ChannelOrder::GRB,
                                     &timing::Generic800,
                                     lw::Rgb8Color>
    {
    };
    using Ws2815B = Ws2815BT<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ws2818T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::RGB,
                                    &timing::Generic800,
                                    lw::Rgb8Color>
    {
    };
    using Ws2818 = Ws2818T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ws2818BT : public Ws2812x<TInterfaceColor,
                                     lw::ChannelOrder::RGB,
                                     &timing::Generic800,
                                     lw::Rgb8Color>
    {
    };
    using Ws2818B = Ws2818BT<>;

    template <typename TInterfaceColor = lw::Color>
    struct Sk6813T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::GRB,
                                    &timing::Generic800,
                                    lw::Rgb8Color>
    {
    };
    using Sk6813 = Sk6813T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Sk6813HvT : public Ws2812x<TInterfaceColor,
                                      lw::ChannelOrder::GRB,
                                      &timing::Generic800,
                                      lw::Rgb8Color>
    {
    };
    using Sk6813Hv = Sk6813HvT<>;

    template <typename TInterfaceColor = lw::Color>
    struct Apa107T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::GRB,
                                    &timing::Generic800,
                                    lw::Rgb8Color>
    {
    };
    using Apa107 = Apa107T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Hc2912c2020T : public Ws2812x<TInterfaceColor,
                                         lw::ChannelOrder::GRB,
                                         &timing::Generic800,
                                         lw::Rgb8Color>
    {
    };
    using Hc2912c2020 = Hc2912c2020T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Sm16703PT : public Ws2812x<TInterfaceColor,
                                      lw::ChannelOrder::RGB,
                                      &timing::Generic800,
                                      lw::Rgb8Color>
    {
    };
    using Sm16703P = Sm16703PT<>;

    template <typename TInterfaceColor = lw::Color>
    struct Sm16703SpT : public Ws2812x<TInterfaceColor,
                                       lw::ChannelOrder::RGB,
                                       &timing::Generic800,
                                       lw::Rgb8Color>
    {
    };
    using Sm16703Sp = Sm16703SpT<>;

    template <typename TInterfaceColor = lw::Color>
    struct Sm16704T : public Ws2812x<TInterfaceColor,
                                     lw::ChannelOrder::RGBW,
                                     &timing::Generic800,
                                     lw::Rgbw8Color>
    {
    };
    using Sm16704 = Sm16704T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Sm16704PbT : public Ws2812x<TInterfaceColor,
                                       lw::ChannelOrder::RGB,
                                       &timing::Generic800,
                                       lw::Rgb8Color>
    {
    };
    using Sm16704Pb = Sm16704PbT<>;

    template <typename TInterfaceColor = lw::Color>
    struct Gs8208BT : public Ws2812x<TInterfaceColor,
                                     lw::ChannelOrder::RGB,
                                     &timing::Generic800,
                                     lw::Rgb8Color>
    {
    };
    using Gs8208B = Gs8208BT<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ucs1903T : public Ws2812x<TInterfaceColor,
                                     lw::ChannelOrder::RGB,
                                     &timing::Generic800,
                                     lw::Rgb8Color>
    {
    };
    using Ucs1903 = Ucs1903T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ucs2903T : public Ws2812x<TInterfaceColor,
                                     lw::ChannelOrder::RGB,
                                     &timing::Generic800,
                                     lw::Rgb8Color>
    {
    };
    using Ucs2903 = Ucs2903T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ucs2904T : public Ws2812x<TInterfaceColor,
                                     lw::ChannelOrder::RGBW,
                                     &timing::Generic800,
                                     lw::Rgbw8Color>
    {
    };
    using Ucs2904 = Ucs2904T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ucs5603T : public Ws2812x<TInterfaceColor,
                                     lw::ChannelOrder::RGB,
                                     &timing::Generic800,
                                     lw::Rgb16Color>
    {
    };
    using Ucs5603 = Ucs5603T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ucs7604T : public Ws2812x<TInterfaceColor,
                                     lw::ChannelOrder::RGBW,
                                     &timing::Generic800,
                                     lw::Rgbw16Color>
    {
    };
    using Ucs7604 = Ucs7604T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ucs8903T : public Ws2812x<TInterfaceColor,
                                     lw::ChannelOrder::RGB,
                                     &timing::Generic800,
                                     lw::Rgb16Color>
    {
    };
    using Ucs8903 = Ucs8903T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ucs8904T : public Ws2812x<TInterfaceColor,
                                     lw::ChannelOrder::RGBW,
                                     &timing::Generic800,
                                     lw::Rgbw16Color>
    {
    };
    using Ucs8904 = Ucs8904T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Ucs8904BT : public Ws2812x<TInterfaceColor,
                                      lw::ChannelOrder::RGBW,
                                      &timing::Generic800,
                                      lw::Rgbw16Color>
    {
    };
    using Ucs8904B = Ucs8904BT<>;

    template <typename TInterfaceColor = lw::Color>
    struct Tm1903T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::RGB,
                                    &timing::Generic800,
                                    lw::Rgb8Color,
                                    false>
    {
    };
    using Tm1903 = Tm1903T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Lb1908T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::RGB,
                                    &timing::Generic800,
                                    lw::Rgb8Color,
                                    false>
    {
    };
    using Lb1908 = Lb1908T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Tm1803T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::RGB,
                                    &timing::Generic400,
                                    lw::Rgb8Color>
    {
    };
    using Tm1803 = Tm1803T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Tm1804T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::RGB,
                                    &timing::Generic400,
                                    lw::Rgb8Color>
    {
    };
    using Tm1804 = Tm1804T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Tm1809T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::RGB,
                                    &timing::Generic400,
                                    lw::Rgb8Color>
    {
    };
    using Tm1809 = Tm1809T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Cs8812T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::RGB,
                                    &timing::Generic800,
                                    lw::Rgb8Color>
    {
    };
    using Cs8812 = Cs8812T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Gs8206T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::RGB,
                                    &timing::Generic800,
                                    lw::Rgb8Color>
    {
    };
    using Gs8206 = Gs8206T<>;

    template <typename TInterfaceColor = lw::Color>
    struct Gs8208T : public Ws2812x<TInterfaceColor,
                                    lw::ChannelOrder::RGB,
                                    &timing::Generic800,
                                    lw::Rgb8Color>
    {
    };
    using Gs8208 = Gs8208T<>;

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
