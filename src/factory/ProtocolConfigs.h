#pragma once

#include <cstdint>
#include <utility>

#include <Arduino.h>

#include "colors/Color.h"
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
        typename TProtocol::SettingsType settings{};
    };

    template <typename TColor>
    struct Ws2812x
    {
        const char *colorOrder = ChannelOrder::GRB;
    };

    using Ws2812 = Ws2812x<Rgb8Color>;
    using Sk6812 = Ws2812x<Rgbw8Color>;
    using Ucs8904 = Ws2812x<Rgbw16Color>;

    template <typename TColor>
    using Nil = ProtocolConfig<NilProtocol<TColor>>;

    template <typename TColor>
    using DebugProtocolConfig = ProtocolConfig<DebugProtocol<TColor>>;

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
                                                           ResourceHandle<IProtocol<TColor>> protocol,
                                                           bool invert = false)
    {
        DebugProtocolConfig<TColor> config{};
        config.settings.output = &output;
        config.settings.invert = invert;
        config.settings.protocol = std::move(protocol);
        return config;
    }

    template <typename TColor>
    inline DebugProtocolConfig<TColor> debugProtocolSerial(bool invert = false)
    {
        return debugProtocolOutput<TColor>(Serial, invert);
    }

    using DotStar = ProtocolConfig<DotStarProtocol>;

    template <typename TColor>
    using Hd108 = ProtocolConfig<Hd108Protocol<TColor>>;

    using Lpd6803 = ProtocolConfig<Lpd6803Protocol>;
    using Lpd8806 = ProtocolConfig<Lpd8806Protocol>;
    using P9813 = ProtocolConfig<P9813Protocol>;
    using Pixie = ProtocolConfig<PixieProtocol>;
    using Sm16716 = ProtocolConfig<Sm16716Protocol>;

    template <typename TColor>
    using Sm168x = ProtocolConfig<Sm168xProtocol<TColor>>;

    template <typename TColor>
    using Tlc5947 = ProtocolConfig<Tlc5947Protocol<TColor>>;

    using Tlc59711 = ProtocolConfig<Tlc59711Protocol>;
    using Tm1814 = ProtocolConfig<Tm1814Protocol>;
    using Tm1914 = ProtocolConfig<Tm1914Protocol>;
    using Ws2801 = ProtocolConfig<Ws2801Protocol>;

    template <typename TColor>
    using Ws2812xRaw = ProtocolConfig<Ws2812xProtocol<TColor>>;

} // namespace npb::factory


