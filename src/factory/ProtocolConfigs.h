#pragma once

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

    using DotStar = ProtocolConfig<DotStarProtocol>;
    using DotStarBusPtr = BusPointerType<DotStar>;

    template <typename TColor>
    using Hd108 = ProtocolConfig<Hd108Protocol<TColor>>;

    template <typename TColor>
    using Hd108BusPtr = BusPointerType<Hd108<TColor>>;

    using Lpd6803 = ProtocolConfig<Lpd6803Protocol>;
    using Lpd8806 = ProtocolConfig<Lpd8806Protocol>;
    using P9813 = ProtocolConfig<P9813Protocol>;
    using Pixie = ProtocolConfig<PixieProtocol>;
    using Sm16716 = ProtocolConfig<Sm16716Protocol>;

    using Lpd6803BusPtr = BusPointerType<Lpd6803>;
    using Lpd8806BusPtr = BusPointerType<Lpd8806>;
    using P9813BusPtr = BusPointerType<P9813>;
    using PixieBusPtr = BusPointerType<Pixie>;
    using Sm16716BusPtr = BusPointerType<Sm16716>;

    template <typename TColor>
    using Sm168x = ProtocolConfig<Sm168xProtocol<TColor>>;

    template <typename TColor>
    using Sm168xBusPtr = BusPointerType<Sm168x<TColor>>;

    template <typename TColor>
    using Tlc5947 = ProtocolConfig<Tlc5947Protocol<TColor>>;

    template <typename TColor>
    using Tlc5947BusPtr = BusPointerType<Tlc5947<TColor>>;

    using Tlc59711 = ProtocolConfig<Tlc59711Protocol>;
    using Tm1814 = ProtocolConfig<Tm1814Protocol>;
    using Tm1914 = ProtocolConfig<Tm1914Protocol>;
    using Ws2801 = ProtocolConfig<Ws2801Protocol>;

    using Tlc59711BusPtr = BusPointerType<Tlc59711>;
    using Tm1814BusPtr = BusPointerType<Tm1814>;
    using Tm1914BusPtr = BusPointerType<Tm1914>;
    using Ws2801BusPtr = BusPointerType<Ws2801>;

    template <typename TColor>
    using Ws2812xRaw = ProtocolConfig<Ws2812xProtocol<TColor>>;

    template <typename TColor>
    using Ws2812xRawBusPtr = BusPointerType<Ws2812xRaw<TColor>>;

} // namespace npb::factory


