#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <utility>

#include <Arduino.h>

#include "colors/Color.h"
#include "core/IPixelBus.h"
#include "protocols/DebugProtocol.h"
#include "protocols/DotStarProtocol.h"
#include "protocols/Hd108Protocol.h"
#include "protocols/Lpd6803Protocol.h"
#include "protocols/Lpd8806Protocol.h"
#include "protocols/NilProtocol.h"
#include "protocols/P9813Protocol.h"
#include "protocols/PixieProtocol.h"
#include "protocols/Sm16716Protocol.h"
#include "protocols/Sm168xProtocol.h"
#include "protocols/Tlc5947Protocol.h"
#include "protocols/Tlc59711Protocol.h"
#include "protocols/Tm1814Protocol.h"
#include "protocols/Tm1914Protocol.h"
#include "protocols/Ws2801Protocol.h"
#include "protocols/Ws2812xProtocol.h"

namespace npb::factory
{

    template <typename TProtocol>
    struct ProtocolConfig
    {
        using ProtocolType = TProtocol;
        using ColorType = typename TProtocol::ColorType;
        typename TProtocol::SettingsType settings{};
    };

    template <typename TProtocolConfig>
    using BusPointerType = std::unique_ptr<npb::IPixelBus<typename TProtocolConfig::ColorType>>;

    template <typename TColor>
    struct Ws2812x
    {
        using ColorType = TColor;
        const char *colorOrder = ChannelOrder::GRB;
    };

    using Ws2812 = Ws2812x<Rgb8Color>;

    struct Sk6812
    {
        using ColorType = Rgbw8Color;
        const char *colorOrder = ChannelOrder::GRBW;
    };

    struct Ucs8904
    {
        using ColorType = Rgbw16Color;
        const char *colorOrder = ChannelOrder::GRBCW;
    };

    using Ws2812BusPtr = BusPointerType<Ws2812>;
    using Sk6812BusPtr = BusPointerType<Sk6812>;
    using Ucs8904BusPtr = BusPointerType<Ucs8904>;

    template <typename TColor>
    using Nil = ProtocolConfig<NilProtocol<TColor>>;

    template <typename TColor>
    using NilBusPtr = BusPointerType<Nil<TColor>>;

    template <typename TColor>
    using DebugProtocolConfig = ProtocolConfig<DebugProtocol<TColor>>;

    template <typename TColor>
    using DebugProtocolBusPtr = BusPointerType<DebugProtocolConfig<TColor>>;

    template <typename TColor>
    inline DebugProtocolConfig<TColor> debugProtocolOutput(Print &output,
                                                           bool invert = false)
    {
        DebugProtocolConfig<TColor> config{};
        config.settings.output = &output;
        config.settings.invert = invert;
        return config;
    }

    template <typename TColor>
    inline DebugProtocolConfig<TColor> debugProtocolOutput(Print &output,
                                                           IProtocol<TColor> *protocol,
                                                           bool invert = false)
    {
        DebugProtocolConfig<TColor> config{};
        config.settings.output = &output;
        config.settings.invert = invert;
        config.settings.protocol = protocol;
        return config;
    }

    template <typename TColor>
    inline DebugProtocolConfig<TColor> debugProtocolSerial(bool invert = false)
    {
        return debugProtocolOutput<TColor>(Serial, invert);
    }

    struct DotStar
    {
        using ColorType = Rgb8Color;
        const char *colorOrder = ChannelOrder::BGR;
        DotStarMode mode = DotStarMode::FixedBrightness;
    };

    using DotStarBusPtr = BusPointerType<DotStar>;

    template <typename TColor>
    struct Hd108
    {
        using ColorType = TColor;
        const char *colorOrder = ChannelOrder::BGR;
    };

    template <typename TColor>
    using Hd108BusPtr = BusPointerType<Hd108<TColor>>;

    struct Lpd6803
    {
        using ColorType = Rgb8Color;
        const char *colorOrder = ChannelOrder::RGB;
    };

    struct Lpd8806
    {
        using ColorType = Rgb8Color;
        const char *colorOrder = ChannelOrder::GRB;
    };

    using P9813 = ProtocolConfig<P9813Protocol>;

    struct Pixie
    {
        using ColorType = Rgb8Color;
        const char *colorOrder = ChannelOrder::RGB;
    };

    struct Sm16716
    {
        using ColorType = Rgb8Color;
        const char *colorOrder = ChannelOrder::RGB;
    };

    using Lpd6803BusPtr = BusPointerType<Lpd6803>;
    using Lpd8806BusPtr = BusPointerType<Lpd8806>;
    using P9813BusPtr = BusPointerType<P9813>;
    using PixieBusPtr = BusPointerType<Pixie>;
    using Sm16716BusPtr = BusPointerType<Sm16716>;

    template <typename TColor>
    struct Sm168x
    {
        using ColorType = TColor;
        const char *colorOrder = ChannelOrder::RGB;
        Sm168xVariant variant = Sm168xVariant::ThreeChannel;
        std::array<uint8_t, 5> gains = {15, 15, 15, 15, 15};
    };

    template <typename TColor>
    using Sm168xBusPtr = BusPointerType<Sm168x<TColor>>;

    template <typename TColor>
    struct Tlc5947
    {
        using ColorType = TColor;
        int8_t latchPin = PinNotUsed;
        int8_t oePin = PinNotUsed;
        const char *colorOrder = ChannelOrder::RGB;
        Tlc5947PixelStrategy pixelStrategy = Tlc5947PixelStrategy::UseColorChannelCount;
        Tlc5947TailFillStrategy tailFillStrategy = Tlc5947TailFillStrategy::Zero;
    };

    template <typename TColor>
    using Tlc5947BusPtr = BusPointerType<Tlc5947<TColor>>;

    using Tlc59711 = ProtocolConfig<Tlc59711Protocol>;

    struct Tm1814
    {
        using ColorType = Rgbw8Color;
        const char *colorOrder = "WRGB";
        Tm1814CurrentSettings current{};
    };

    struct Tm1914
    {
        using ColorType = Rgb8Color;
        const char *colorOrder = ChannelOrder::GRB;
        Tm1914Mode mode = Tm1914Mode::DinOnly;
    };

    struct Ws2801
    {
        using ColorType = Rgb8Color;
        const char *colorOrder = ChannelOrder::RGB;
    };

    using Tlc59711BusPtr = BusPointerType<Tlc59711>;
    using Tm1814BusPtr = BusPointerType<Tm1814>;
    using Tm1914BusPtr = BusPointerType<Tm1914>;
    using Ws2801BusPtr = BusPointerType<Ws2801>;

    template <typename TColor>
    using Ws2812xRaw = Ws2812x<TColor>;

    template <typename TColor>
    using Ws2812xRawBusPtr = BusPointerType<Ws2812xRaw<TColor>>;

} // namespace npb::factory


