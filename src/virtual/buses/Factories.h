#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <utility>

#include "BusDriver.h"
#include "../colors/IShader.h"
#include "../colors/AggregateShader.h"
#include "../colors/CurrentLimiterShader.h"
#include "../colors/GammaShader.h"
#include "../colors/WhiteBalanceShader.h"
#include "../protocols/WithShaderProtocol.h"
#include "../protocols/Ws2812xProtocol.h"
#include "../protocols/DotStarProtocol.h"
#include "../protocols/Ws2801Protocol.h"
#include "../protocols/PixieProtocol.h"
#include "../protocols/Hd108Protocol.h"
#include "../protocols/Lpd8806Protocol.h"
#include "../protocols/Lpd6803Protocol.h"
#include "../protocols/P9813Protocol.h"
#include "../protocols/Sm16716Protocol.h"
#include "../protocols/Sm168xProtocol.h"
#include "../protocols/Tlc59711Protocol.h"
#include "../protocols/Tlc5947Protocol.h"
#include "../protocols/Tm1814Protocol.h"
#include "../protocols/Tm1914Protocol.h"

namespace npb::factory
{

    template <typename TTransport,
              typename TColor = Rgb8Color,
              template <typename> typename TProtocolT = Ws2812xProtocol>
    using Ws2812xOwningPixelBusT = OwningBusDriverPixelBusT<TTransport, TProtocolT<TColor>>;

    template <typename TColor = Rgb8Color>
    using Ws2812xWithShaderProtocolT = WithShader<TColor, Ws2812xProtocol<TColor>>;

    template <typename TColor = Rgb8Color, typename TShader = IShader<TColor>>
    using Ws2812xWithEmbeddedShaderProtocolT = WithEmbeddedShader<TColor, TShader, Ws2812xProtocol<TColor>>;

    template <typename TTransport, typename TColor = Rgb8Color>
        requires TaggedTransportLike<TTransport, OneWireTransportTag>
    Ws2812xOwningPixelBusT<TTransport, TColor> makeWs2812xBus(uint16_t pixelCount,
                                                              const char *channelOrder,
                                                              typename TTransport::TransportConfigType transportConfig)
    {
        Ws2812xProtocolSettings settings{};
        settings.channelOrder = channelOrder;

        return makeOwningDriverPixelBus<TTransport, Ws2812xProtocol<TColor>>(pixelCount,
                                                                             std::move(transportConfig),
                                                                             std::move(settings));
    }

    template <typename TTransport, typename TColor = Rgb8Color>
        requires TaggedTransportLike<TTransport, OneWireTransportTag>
    Ws2812xOwningPixelBusT<TTransport, TColor, Ws2812xWithShaderProtocolT> makeWs2812xBus(uint16_t pixelCount,
                                                                                          const char *channelOrder,
                                                                                          ResourceHandle<IShader<TColor>> shader,
                                                                                          typename TTransport::TransportConfigType transportConfig)
    {
        Ws2812xProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        typename Ws2812xWithShaderProtocolT<TColor>::SettingsType shaderSettings{};
        shaderSettings.shader = std::move(shader);

        return makeOwningDriverPixelBus<TTransport, Ws2812xWithShaderProtocolT<TColor>>(pixelCount,
                                                                                        std::move(transportConfig),
                                                                                        std::move(shaderSettings),
                                                                                        std::move(protocolSettings));
    }

    template <typename TTransport, typename TColor = Rgb8Color, typename TShader>
        requires TaggedTransportLike<TTransport, OneWireTransportTag> &&
                 std::derived_from<std::remove_cvref_t<TShader>, IShader<TColor>>
    OwningBusDriverPixelBusT<TTransport, Ws2812xWithEmbeddedShaderProtocolT<TColor, std::remove_cvref_t<TShader>>> makeWs2812xBus(uint16_t pixelCount,
                                                                                                                                    const char *channelOrder,
                                                                                                                                    TShader &&shader,
                                                                                                                                    typename TTransport::TransportConfigType transportConfig)
    {
        using ShaderType = std::remove_cvref_t<TShader>;
        using ProtocolType = Ws2812xWithEmbeddedShaderProtocolT<TColor, ShaderType>;

        Ws2812xProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        typename ProtocolType::SettingsType shaderSettings{};
        shaderSettings.shader = std::forward<TShader>(shader);

        return makeOwningDriverPixelBus<TTransport, ProtocolType>(pixelCount,
                                                                   std::move(transportConfig),
                                                                   std::move(shaderSettings),
                                                                   std::move(protocolSettings));
    }

    template <typename TTransport, typename TProtocol = DotStarProtocol>
    using DotStarOwningPixelBusT = OwningBusDriverPixelBusT<TTransport, TProtocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag>
    DotStarOwningPixelBusT<TTransport> makeDotStarBus(uint16_t pixelCount,
                                                      const char *channelOrder,
                                                      typename TTransport::TransportConfigType transportConfig,
                                                      DotStarMode mode = DotStarMode::FixedBrightness)
    {
        DotStarProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;
        protocolSettings.mode = mode;

        return makeOwningDriverPixelBus<TTransport, DotStarProtocol>(pixelCount,
                                                                     std::move(transportConfig),
                                                                     std::move(protocolSettings));
    }

    using DotStarWithShaderProtocol = WithShader<Rgb8Color, DotStarProtocol>;
    template <typename TShader = IShader<Rgb8Color>>
    using DotStarWithEmbeddedShaderProtocol = WithEmbeddedShader<Rgb8Color, TShader, DotStarProtocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag>
    DotStarOwningPixelBusT<TTransport, DotStarWithShaderProtocol> makeDotStarBus(uint16_t pixelCount,
                                                                                 const char *channelOrder,
                                                                                 ResourceHandle<IShader<Rgb8Color>> shader,
                                                                                 typename TTransport::TransportConfigType transportConfig,
                                                                                 DotStarMode mode = DotStarMode::FixedBrightness)
    {
        DotStarProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;
        protocolSettings.mode = mode;

        DotStarWithShaderProtocol::SettingsType shaderSettings{};
        shaderSettings.shader = std::move(shader);

        return makeOwningDriverPixelBus<TTransport, DotStarWithShaderProtocol>(pixelCount,
                                                                               std::move(transportConfig),
                                                                               std::move(shaderSettings),
                                                                               std::move(protocolSettings));
    }

    template <typename TTransport, typename TShader>
        requires TaggedTransportLike<TTransport, TransportTag> &&
                 std::derived_from<std::remove_cvref_t<TShader>, IShader<Rgb8Color>>
    DotStarOwningPixelBusT<TTransport, DotStarWithEmbeddedShaderProtocol<std::remove_cvref_t<TShader>>> makeDotStarBus(uint16_t pixelCount,
                                                                                                                          const char *channelOrder,
                                                                                                                          TShader &&shader,
                                                                                                                          typename TTransport::TransportConfigType transportConfig,
                                                                                                                          DotStarMode mode = DotStarMode::FixedBrightness)
    {
        using ShaderType = std::remove_cvref_t<TShader>;
        using ProtocolType = DotStarWithEmbeddedShaderProtocol<ShaderType>;

        DotStarProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;
        protocolSettings.mode = mode;

        typename ProtocolType::SettingsType shaderSettings{};
        shaderSettings.shader = std::forward<TShader>(shader);

        return makeOwningDriverPixelBus<TTransport, ProtocolType>(pixelCount,
                                                                   std::move(transportConfig),
                                                                   std::move(shaderSettings),
                                                                   std::move(protocolSettings));
    }

    using Ws2801WithShaderProtocol = WithShader<Rgb8Color, Ws2801Protocol>;
    template <typename TShader = IShader<Rgb8Color>>
    using Ws2801WithEmbeddedShaderProtocol = WithEmbeddedShader<Rgb8Color, TShader, Ws2801Protocol>;

    template <typename TTransport, typename TProtocol = Ws2801Protocol>
    using Ws2801OwningPixelBusT = OwningBusDriverPixelBusT<TTransport, TProtocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag>
    Ws2801OwningPixelBusT<TTransport> makeWs2801Bus(uint16_t pixelCount,
                                                    const char *channelOrder,
                                                    typename TTransport::TransportConfigType transportConfig)
    {
        Ws2801ProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        return makeOwningDriverPixelBus<TTransport, Ws2801Protocol>(pixelCount,
                                                                    std::move(transportConfig),
                                                                    std::move(protocolSettings));
    }

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag>
    Ws2801OwningPixelBusT<TTransport, Ws2801WithShaderProtocol> makeWs2801Bus(uint16_t pixelCount,
                                                                              const char *channelOrder,
                                                                              ResourceHandle<IShader<Rgb8Color>> shader,
                                                                              typename TTransport::TransportConfigType transportConfig)
    {
        Ws2801ProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        Ws2801WithShaderProtocol::SettingsType shaderSettings{};
        shaderSettings.shader = std::move(shader);

        return makeOwningDriverPixelBus<TTransport, Ws2801WithShaderProtocol>(pixelCount,
                                                                              std::move(transportConfig),
                                                                              std::move(shaderSettings),
                                                                              std::move(protocolSettings));
    }

    template <typename TTransport, typename TShader>
        requires TaggedTransportLike<TTransport, TransportTag> &&
                 std::derived_from<std::remove_cvref_t<TShader>, IShader<Rgb8Color>>
    Ws2801OwningPixelBusT<TTransport, Ws2801WithEmbeddedShaderProtocol<std::remove_cvref_t<TShader>>> makeWs2801Bus(uint16_t pixelCount,
                                                                                                                        const char *channelOrder,
                                                                                                                        TShader &&shader,
                                                                                                                        typename TTransport::TransportConfigType transportConfig)
    {
        using ShaderType = std::remove_cvref_t<TShader>;
        using ProtocolType = Ws2801WithEmbeddedShaderProtocol<ShaderType>;

        Ws2801ProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        typename ProtocolType::SettingsType shaderSettings{};
        shaderSettings.shader = std::forward<TShader>(shader);

        return makeOwningDriverPixelBus<TTransport, ProtocolType>(pixelCount,
                                                                   std::move(transportConfig),
                                                                   std::move(shaderSettings),
                                                                   std::move(protocolSettings));
    }

    using PixieWithShaderProtocol = WithShader<Rgb8Color, PixieProtocol>;
    template <typename TShader = IShader<Rgb8Color>>
    using PixieWithEmbeddedShaderProtocol = WithEmbeddedShader<Rgb8Color, TShader, PixieProtocol>;

    template <typename TTransport, typename TProtocol = PixieProtocol>
    using PixieOwningPixelBusT = OwningBusDriverPixelBusT<TTransport, TProtocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, OneWireTransportTag>
    PixieOwningPixelBusT<TTransport> makePixieBus(uint16_t pixelCount,
                                                  const char *channelOrder,
                                                  typename TTransport::TransportConfigType transportConfig)
    {
        PixieProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        return makeOwningDriverPixelBus<TTransport, PixieProtocol>(pixelCount,
                                                                   std::move(transportConfig),
                                                                   std::move(protocolSettings));
    }

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, OneWireTransportTag>
    PixieOwningPixelBusT<TTransport, PixieWithShaderProtocol> makePixieBus(uint16_t pixelCount,
                                                                           const char *channelOrder,
                                                                           ResourceHandle<IShader<Rgb8Color>> shader,
                                                                           typename TTransport::TransportConfigType transportConfig)
    {
        PixieProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        PixieWithShaderProtocol::SettingsType shaderSettings{};
        shaderSettings.shader = std::move(shader);

        return makeOwningDriverPixelBus<TTransport, PixieWithShaderProtocol>(pixelCount,
                                                                             std::move(transportConfig),
                                                                             std::move(shaderSettings),
                                                                             std::move(protocolSettings));
    }

    template <typename TTransport, typename TShader>
        requires TaggedTransportLike<TTransport, OneWireTransportTag> &&
                 std::derived_from<std::remove_cvref_t<TShader>, IShader<Rgb8Color>>
    PixieOwningPixelBusT<TTransport, PixieWithEmbeddedShaderProtocol<std::remove_cvref_t<TShader>>> makePixieBus(uint16_t pixelCount,
                                                                                                                    const char *channelOrder,
                                                                                                                    TShader &&shader,
                                                                                                                    typename TTransport::TransportConfigType transportConfig)
    {
        using ShaderType = std::remove_cvref_t<TShader>;
        using ProtocolType = PixieWithEmbeddedShaderProtocol<ShaderType>;

        PixieProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        typename ProtocolType::SettingsType shaderSettings{};
        shaderSettings.shader = std::forward<TShader>(shader);

        return makeOwningDriverPixelBus<TTransport, ProtocolType>(pixelCount,
                                                                   std::move(transportConfig),
                                                                   std::move(shaderSettings),
                                                                   std::move(protocolSettings));
    }

    template <typename TTransport, typename TProtocol = Lpd8806Protocol>
    using Lpd8806OwningPixelBusT = OwningBusDriverPixelBusT<TTransport, TProtocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag>
    Lpd8806OwningPixelBusT<TTransport> makeLpd8806Bus(uint16_t pixelCount,
                                                      const char *channelOrder,
                                                      typename TTransport::TransportConfigType transportConfig)
    {
        Lpd8806ProtocolSettings settings{};
        settings.channelOrder = channelOrder;

        return makeOwningDriverPixelBus<TTransport, Lpd8806Protocol>(pixelCount,
                                                                     std::move(transportConfig),
                                                                     std::move(settings));
    }

    using Lpd8806WithShaderProtocol = WithShader<Rgb8Color, Lpd8806Protocol>;
    template <typename TShader = IShader<Rgb8Color>>
    using Lpd8806WithEmbeddedShaderProtocol = WithEmbeddedShader<Rgb8Color, TShader, Lpd8806Protocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag>
    Lpd8806OwningPixelBusT<TTransport, Lpd8806WithShaderProtocol> makeLpd8806Bus(uint16_t pixelCount,
                                                                                 const char *channelOrder,
                                                                                 ResourceHandle<IShader<Rgb8Color>> shader,
                                                                                 typename TTransport::TransportConfigType transportConfig)
    {
        Lpd8806ProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        Lpd8806WithShaderProtocol::SettingsType shaderSettings{};
        shaderSettings.shader = std::move(shader);

        return makeOwningDriverPixelBus<TTransport, Lpd8806WithShaderProtocol>(pixelCount,
                                                                               std::move(transportConfig),
                                                                               std::move(shaderSettings),
                                                                               std::move(protocolSettings));
    }

    template <typename TTransport, typename TShader>
        requires TaggedTransportLike<TTransport, TransportTag> &&
                 std::derived_from<std::remove_cvref_t<TShader>, IShader<Rgb8Color>>
    Lpd8806OwningPixelBusT<TTransport, Lpd8806WithEmbeddedShaderProtocol<std::remove_cvref_t<TShader>>> makeLpd8806Bus(uint16_t pixelCount,
                                                                                                                          const char *channelOrder,
                                                                                                                          TShader &&shader,
                                                                                                                          typename TTransport::TransportConfigType transportConfig)
    {
        using ShaderType = std::remove_cvref_t<TShader>;
        using ProtocolType = Lpd8806WithEmbeddedShaderProtocol<ShaderType>;

        Lpd8806ProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        typename ProtocolType::SettingsType shaderSettings{};
        shaderSettings.shader = std::forward<TShader>(shader);

        return makeOwningDriverPixelBus<TTransport, ProtocolType>(pixelCount,
                                                                   std::move(transportConfig),
                                                                   std::move(shaderSettings),
                                                                   std::move(protocolSettings));
    }

    template <typename TTransport, typename TProtocol = Lpd6803Protocol>
    using Lpd6803OwningPixelBusT = OwningBusDriverPixelBusT<TTransport, TProtocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag>
    Lpd6803OwningPixelBusT<TTransport> makeLpd6803Bus(uint16_t pixelCount,
                                                      const char *channelOrder,
                                                      typename TTransport::TransportConfigType transportConfig)
    {
        Lpd6803ProtocolSettings settings{};
        settings.channelOrder = channelOrder;

        return makeOwningDriverPixelBus<TTransport, Lpd6803Protocol>(pixelCount,
                                                                     std::move(transportConfig),
                                                                     std::move(settings));
    }

    using Lpd6803WithShaderProtocol = WithShader<Rgb8Color, Lpd6803Protocol>;
    template <typename TShader = IShader<Rgb8Color>>
    using Lpd6803WithEmbeddedShaderProtocol = WithEmbeddedShader<Rgb8Color, TShader, Lpd6803Protocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag>
    Lpd6803OwningPixelBusT<TTransport, Lpd6803WithShaderProtocol> makeLpd6803Bus(uint16_t pixelCount,
                                                                                 const char *channelOrder,
                                                                                 ResourceHandle<IShader<Rgb8Color>> shader,
                                                                                 typename TTransport::TransportConfigType transportConfig)
    {
        Lpd6803ProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        Lpd6803WithShaderProtocol::SettingsType shaderSettings{};
        shaderSettings.shader = std::move(shader);

        return makeOwningDriverPixelBus<TTransport, Lpd6803WithShaderProtocol>(pixelCount,
                                                                               std::move(transportConfig),
                                                                               std::move(shaderSettings),
                                                                               std::move(protocolSettings));
    }

    template <typename TTransport, typename TShader>
        requires TaggedTransportLike<TTransport, TransportTag> &&
                 std::derived_from<std::remove_cvref_t<TShader>, IShader<Rgb8Color>>
    Lpd6803OwningPixelBusT<TTransport, Lpd6803WithEmbeddedShaderProtocol<std::remove_cvref_t<TShader>>> makeLpd6803Bus(uint16_t pixelCount,
                                                                                                                          const char *channelOrder,
                                                                                                                          TShader &&shader,
                                                                                                                          typename TTransport::TransportConfigType transportConfig)
    {
        using ShaderType = std::remove_cvref_t<TShader>;
        using ProtocolType = Lpd6803WithEmbeddedShaderProtocol<ShaderType>;

        Lpd6803ProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        typename ProtocolType::SettingsType shaderSettings{};
        shaderSettings.shader = std::forward<TShader>(shader);

        return makeOwningDriverPixelBus<TTransport, ProtocolType>(pixelCount,
                                                                   std::move(transportConfig),
                                                                   std::move(shaderSettings),
                                                                   std::move(protocolSettings));
    }

    template <typename TTransport, typename TProtocol = P9813Protocol>
    using P9813OwningPixelBusT = OwningBusDriverPixelBusT<TTransport, TProtocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag>
    P9813OwningPixelBusT<TTransport> makeP9813Bus(uint16_t pixelCount,
                                                  typename TTransport::TransportConfigType transportConfig)
    {
        P9813ProtocolSettings settings{};

        return makeOwningDriverPixelBus<TTransport, P9813Protocol>(pixelCount,
                                                                   std::move(transportConfig),
                                                                   std::move(settings));
    }

    using P9813WithShaderProtocol = WithShader<Rgb8Color, P9813Protocol>;
    template <typename TShader = IShader<Rgb8Color>>
    using P9813WithEmbeddedShaderProtocol = WithEmbeddedShader<Rgb8Color, TShader, P9813Protocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag>
    P9813OwningPixelBusT<TTransport, P9813WithShaderProtocol> makeP9813Bus(uint16_t pixelCount,
                                                                           ResourceHandle<IShader<Rgb8Color>> shader,
                                                                           typename TTransport::TransportConfigType transportConfig)
    {
        P9813ProtocolSettings protocolSettings{};

        P9813WithShaderProtocol::SettingsType shaderSettings{};
        shaderSettings.shader = std::move(shader);

        return makeOwningDriverPixelBus<TTransport, P9813WithShaderProtocol>(pixelCount,
                                                                             std::move(transportConfig),
                                                                             std::move(shaderSettings),
                                                                             std::move(protocolSettings));
    }

    template <typename TTransport, typename TShader>
        requires TaggedTransportLike<TTransport, TransportTag> &&
                 std::derived_from<std::remove_cvref_t<TShader>, IShader<Rgb8Color>>
    P9813OwningPixelBusT<TTransport, P9813WithEmbeddedShaderProtocol<std::remove_cvref_t<TShader>>> makeP9813Bus(uint16_t pixelCount,
                                                                                                                    TShader &&shader,
                                                                                                                    typename TTransport::TransportConfigType transportConfig)
    {
        using ShaderType = std::remove_cvref_t<TShader>;
        using ProtocolType = P9813WithEmbeddedShaderProtocol<ShaderType>;

        P9813ProtocolSettings protocolSettings{};

        typename ProtocolType::SettingsType shaderSettings{};
        shaderSettings.shader = std::forward<TShader>(shader);

        return makeOwningDriverPixelBus<TTransport, ProtocolType>(pixelCount,
                                                                   std::move(transportConfig),
                                                                   std::move(shaderSettings),
                                                                   std::move(protocolSettings));
    }

    template <typename TTransport, typename TProtocol = Sm16716Protocol>
    using Sm16716OwningPixelBusT = OwningBusDriverPixelBusT<TTransport, TProtocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag>
    Sm16716OwningPixelBusT<TTransport> makeSm16716Bus(uint16_t pixelCount,
                                                      const char *channelOrder,
                                                      typename TTransport::TransportConfigType transportConfig)
    {
        Sm16716ProtocolSettings settings{};
        settings.channelOrder = channelOrder;

        return makeOwningDriverPixelBus<TTransport, Sm16716Protocol>(pixelCount,
                                                                     std::move(transportConfig),
                                                                     std::move(settings));
    }

    using Sm16716WithShaderProtocol = WithShader<Rgb8Color, Sm16716Protocol>;
    template <typename TShader = IShader<Rgb8Color>>
    using Sm16716WithEmbeddedShaderProtocol = WithEmbeddedShader<Rgb8Color, TShader, Sm16716Protocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag>
    Sm16716OwningPixelBusT<TTransport, Sm16716WithShaderProtocol> makeSm16716Bus(uint16_t pixelCount,
                                                                                 const char *channelOrder,
                                                                                 ResourceHandle<IShader<Rgb8Color>> shader,
                                                                                 typename TTransport::TransportConfigType transportConfig)
    {
        Sm16716ProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        Sm16716WithShaderProtocol::SettingsType shaderSettings{};
        shaderSettings.shader = std::move(shader);

        return makeOwningDriverPixelBus<TTransport, Sm16716WithShaderProtocol>(pixelCount,
                                                                               std::move(transportConfig),
                                                                               std::move(shaderSettings),
                                                                               std::move(protocolSettings));
    }

    template <typename TTransport, typename TShader>
        requires TaggedTransportLike<TTransport, TransportTag> &&
                 std::derived_from<std::remove_cvref_t<TShader>, IShader<Rgb8Color>>
    Sm16716OwningPixelBusT<TTransport, Sm16716WithEmbeddedShaderProtocol<std::remove_cvref_t<TShader>>> makeSm16716Bus(uint16_t pixelCount,
                                                                                                                          const char *channelOrder,
                                                                                                                          TShader &&shader,
                                                                                                                          typename TTransport::TransportConfigType transportConfig)
    {
        using ShaderType = std::remove_cvref_t<TShader>;
        using ProtocolType = Sm16716WithEmbeddedShaderProtocol<ShaderType>;

        Sm16716ProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        typename ProtocolType::SettingsType shaderSettings{};
        shaderSettings.shader = std::forward<TShader>(shader);

        return makeOwningDriverPixelBus<TTransport, ProtocolType>(pixelCount,
                                                                   std::move(transportConfig),
                                                                   std::move(shaderSettings),
                                                                   std::move(protocolSettings));
    }

    template <typename TTransport,
              typename TColor = Rgbcw8Color,
              template <typename> typename TProtocolT = Sm168xProtocol>
    using Sm168xOwningPixelBusT = OwningBusDriverPixelBusT<TTransport, TProtocolT<TColor>>;

    template <typename TTransport, typename TColor = Rgbcw8Color>
        requires TaggedTransportLike<TTransport, TransportTag>
    Sm168xOwningPixelBusT<TTransport, TColor> makeSm168xBus(uint16_t pixelCount,
                                                            const char *channelOrder,
                                                            Sm168xVariant variant,
                                                            std::array<uint8_t, 5> gains,
                                                            typename TTransport::TransportConfigType transportConfig)
    {
        Sm168xProtocolSettings settings{};
        settings.channelOrder = channelOrder;
        settings.variant = variant;
        settings.gains = gains;

        return makeOwningDriverPixelBus<TTransport, Sm168xProtocol<TColor>>(pixelCount,
                                                                            std::move(transportConfig),
                                                                            std::move(settings));
    }

    template <typename TColor = Rgbcw8Color>
    using Sm168xWithShaderProtocolT = WithShader<TColor, Sm168xProtocol<TColor>>;

    template <typename TColor = Rgbcw8Color, typename TShader = IShader<TColor>>
    using Sm168xWithEmbeddedShaderProtocolT = WithEmbeddedShader<TColor, TShader, Sm168xProtocol<TColor>>;

    template <typename TTransport, typename TColor = Rgbcw8Color>
        requires TaggedTransportLike<TTransport, TransportTag>
    Sm168xOwningPixelBusT<TTransport, TColor, Sm168xWithShaderProtocolT> makeSm168xBus(uint16_t pixelCount,
                                                                                       const char *channelOrder,
                                                                                       Sm168xVariant variant,
                                                                                       std::array<uint8_t, 5> gains,
                                                                                       ResourceHandle<IShader<TColor>> shader,
                                                                                       typename TTransport::TransportConfigType transportConfig)
    {
        Sm168xProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;
        protocolSettings.variant = variant;
        protocolSettings.gains = gains;

        typename Sm168xWithShaderProtocolT<TColor>::SettingsType shaderSettings{};
        shaderSettings.shader = std::move(shader);

        return makeOwningDriverPixelBus<TTransport, Sm168xWithShaderProtocolT<TColor>>(pixelCount,
                                                                                       std::move(transportConfig),
                                                                                       std::move(shaderSettings),
                                                                                       std::move(protocolSettings));
    }

    template <typename TTransport, typename TColor = Rgbcw8Color, typename TShader>
        requires TaggedTransportLike<TTransport, TransportTag> &&
                 std::derived_from<std::remove_cvref_t<TShader>, IShader<TColor>>
    OwningBusDriverPixelBusT<TTransport, Sm168xWithEmbeddedShaderProtocolT<TColor, std::remove_cvref_t<TShader>>> makeSm168xBus(uint16_t pixelCount,
                                                                                                                                 const char *channelOrder,
                                                                                                                                 Sm168xVariant variant,
                                                                                                                                 std::array<uint8_t, 5> gains,
                                                                                                                                 TShader &&shader,
                                                                                                                                 typename TTransport::TransportConfigType transportConfig)
    {
        using ShaderType = std::remove_cvref_t<TShader>;
        using ProtocolType = Sm168xWithEmbeddedShaderProtocolT<TColor, ShaderType>;

        Sm168xProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;
        protocolSettings.variant = variant;
        protocolSettings.gains = gains;

        typename ProtocolType::SettingsType shaderSettings{};
        shaderSettings.shader = std::forward<TShader>(shader);

        return makeOwningDriverPixelBus<TTransport, ProtocolType>(pixelCount,
                                                                   std::move(transportConfig),
                                                                   std::move(shaderSettings),
                                                                   std::move(protocolSettings));
    }

    template <typename TTransport, typename TProtocol = Tlc59711Protocol>
    using Tlc59711OwningPixelBusT = OwningBusDriverPixelBusT<TTransport, TProtocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag>
    Tlc59711OwningPixelBusT<TTransport> makeTlc59711Bus(uint16_t pixelCount,
                                                        typename TTransport::TransportConfigType transportConfig,
                                                        Tlc59711Config config = {})
    {
        Tlc59711ProtocolSettings settings{};
        settings.config = config;

        return makeOwningDriverPixelBus<TTransport, Tlc59711Protocol>(pixelCount,
                                                                      std::move(transportConfig),
                                                                      std::move(settings));
    }

    using Tlc59711WithShaderProtocol = WithShader<Rgb8Color, Tlc59711Protocol>;
    template <typename TShader = IShader<Rgb8Color>>
    using Tlc59711WithEmbeddedShaderProtocol = WithEmbeddedShader<Rgb8Color, TShader, Tlc59711Protocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag>
    Tlc59711OwningPixelBusT<TTransport, Tlc59711WithShaderProtocol> makeTlc59711Bus(uint16_t pixelCount,
                                                                                    ResourceHandle<IShader<Rgb8Color>> shader,
                                                                                    typename TTransport::TransportConfigType transportConfig,
                                                                                    Tlc59711Config config = {})
    {
        Tlc59711ProtocolSettings protocolSettings{};
        protocolSettings.config = config;

        Tlc59711WithShaderProtocol::SettingsType shaderSettings{};
        shaderSettings.shader = std::move(shader);

        return makeOwningDriverPixelBus<TTransport, Tlc59711WithShaderProtocol>(pixelCount,
                                                                                std::move(transportConfig),
                                                                                std::move(shaderSettings),
                                                                                std::move(protocolSettings));
    }

    template <typename TTransport, typename TShader>
        requires TaggedTransportLike<TTransport, TransportTag> &&
                 std::derived_from<std::remove_cvref_t<TShader>, IShader<Rgb8Color>>
    Tlc59711OwningPixelBusT<TTransport, Tlc59711WithEmbeddedShaderProtocol<std::remove_cvref_t<TShader>>> makeTlc59711Bus(uint16_t pixelCount,
                                                                                                                             TShader &&shader,
                                                                                                                             typename TTransport::TransportConfigType transportConfig,
                                                                                                                             Tlc59711Config config = {})
    {
        using ShaderType = std::remove_cvref_t<TShader>;
        using ProtocolType = Tlc59711WithEmbeddedShaderProtocol<ShaderType>;

        Tlc59711ProtocolSettings protocolSettings{};
        protocolSettings.config = config;

        typename ProtocolType::SettingsType shaderSettings{};
        shaderSettings.shader = std::forward<TShader>(shader);

        return makeOwningDriverPixelBus<TTransport, ProtocolType>(pixelCount,
                                                                   std::move(transportConfig),
                                                                   std::move(shaderSettings),
                                                                   std::move(protocolSettings));
    }

    template <typename TTransport,
              typename TColor = Rgb16Color,
              template <typename> typename TProtocolT = Tlc5947Protocol>
    using Tlc5947OwningPixelBusT = OwningBusDriverPixelBusT<TTransport, TProtocolT<TColor>>;

    template <typename TTransport, typename TColor = Rgb16Color>
        requires TaggedTransportLike<TTransport, TransportTag>
    Tlc5947OwningPixelBusT<TTransport, TColor> makeTlc5947Bus(uint16_t pixelCount,
                                                              const char *channelOrder,
                                                              int8_t latchPin,
                                                              typename TTransport::TransportConfigType transportConfig,
                                                              int8_t oePin = PinNotUsed,
                                                              Tlc5947PixelStrategy pixelStrategy = Tlc5947PixelStrategy::UseColorChannelCount,
                                                              Tlc5947TailFillStrategy tailFillStrategy = Tlc5947TailFillStrategy::Zero)
    {
        Tlc5947ProtocolSettings settings{};
        settings.channelOrder = channelOrder;
        settings.latchPin = latchPin;
        settings.oePin = oePin;
        settings.pixelStrategy = pixelStrategy;
        settings.tailFillStrategy = tailFillStrategy;

        return makeOwningDriverPixelBus<TTransport, Tlc5947Protocol<TColor>>(pixelCount,
                                                                             std::move(transportConfig),
                                                                             std::move(settings));
    }

    template <typename TColor = Rgb16Color>
    using Tlc5947WithShaderProtocolT = WithShader<TColor, Tlc5947Protocol<TColor>>;

    template <typename TColor = Rgb16Color, typename TShader = IShader<TColor>>
    using Tlc5947WithEmbeddedShaderProtocolT = WithEmbeddedShader<TColor, TShader, Tlc5947Protocol<TColor>>;

    template <typename TTransport, typename TColor = Rgb16Color>
        requires TaggedTransportLike<TTransport, TransportTag>
    Tlc5947OwningPixelBusT<TTransport, TColor, Tlc5947WithShaderProtocolT> makeTlc5947Bus(uint16_t pixelCount,
                                                                                          const char *channelOrder,
                                                                                          int8_t latchPin,
                                                                                          ResourceHandle<IShader<TColor>> shader,
                                                                                          typename TTransport::TransportConfigType transportConfig,
                                                                                          int8_t oePin = PinNotUsed,
                                                                                          Tlc5947PixelStrategy pixelStrategy = Tlc5947PixelStrategy::UseColorChannelCount,
                                                                                          Tlc5947TailFillStrategy tailFillStrategy = Tlc5947TailFillStrategy::Zero)
    {
        Tlc5947ProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;
        protocolSettings.latchPin = latchPin;
        protocolSettings.oePin = oePin;
        protocolSettings.pixelStrategy = pixelStrategy;
        protocolSettings.tailFillStrategy = tailFillStrategy;

        typename Tlc5947WithShaderProtocolT<TColor>::SettingsType shaderSettings{};
        shaderSettings.shader = std::move(shader);

        return makeOwningDriverPixelBus<TTransport, Tlc5947WithShaderProtocolT<TColor>>(pixelCount,
                                                                                        std::move(transportConfig),
                                                                                        std::move(shaderSettings),
                                                                                        std::move(protocolSettings));
    }

    template <typename TTransport, typename TColor = Rgb16Color, typename TShader>
        requires TaggedTransportLike<TTransport, TransportTag> &&
                 std::derived_from<std::remove_cvref_t<TShader>, IShader<TColor>>
    OwningBusDriverPixelBusT<TTransport, Tlc5947WithEmbeddedShaderProtocolT<TColor, std::remove_cvref_t<TShader>>> makeTlc5947Bus(uint16_t pixelCount,
                                                                                                                                    const char *channelOrder,
                                                                                                                                    int8_t latchPin,
                                                                                                                                    TShader &&shader,
                                                                                                                                    typename TTransport::TransportConfigType transportConfig,
                                                                                                                                    int8_t oePin = PinNotUsed,
                                                                                                                                    Tlc5947PixelStrategy pixelStrategy = Tlc5947PixelStrategy::UseColorChannelCount,
                                                                                                                                    Tlc5947TailFillStrategy tailFillStrategy = Tlc5947TailFillStrategy::Zero)
    {
        using ShaderType = std::remove_cvref_t<TShader>;
        using ProtocolType = Tlc5947WithEmbeddedShaderProtocolT<TColor, ShaderType>;

        Tlc5947ProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;
        protocolSettings.latchPin = latchPin;
        protocolSettings.oePin = oePin;
        protocolSettings.pixelStrategy = pixelStrategy;
        protocolSettings.tailFillStrategy = tailFillStrategy;

        typename ProtocolType::SettingsType shaderSettings{};
        shaderSettings.shader = std::forward<TShader>(shader);

        return makeOwningDriverPixelBus<TTransport, ProtocolType>(pixelCount,
                                                                   std::move(transportConfig),
                                                                   std::move(shaderSettings),
                                                                   std::move(protocolSettings));
    }

    template <typename TTransport, typename TProtocol = Tm1814Protocol>
    using Tm1814OwningPixelBusT = OwningBusDriverPixelBusT<TTransport, TProtocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, OneWireTransportTag>
    Tm1814OwningPixelBusT<TTransport> makeTm1814Bus(uint16_t pixelCount,
                                                    const char *channelOrder,
                                                    Tm1814CurrentSettings current,
                                                    typename TTransport::TransportConfigType transportConfig)
    {
        Tm1814ProtocolSettings settings{};
        settings.channelOrder = channelOrder;
        settings.current = current;

        return makeOwningDriverPixelBus<TTransport, Tm1814Protocol>(pixelCount,
                                                                    std::move(transportConfig),
                                                                    std::move(settings));
    }

    using Tm1814WithShaderProtocol = WithShader<Rgbw8Color, Tm1814Protocol>;
    template <typename TShader = IShader<Rgbw8Color>>
    using Tm1814WithEmbeddedShaderProtocol = WithEmbeddedShader<Rgbw8Color, TShader, Tm1814Protocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, OneWireTransportTag>
    Tm1814OwningPixelBusT<TTransport, Tm1814WithShaderProtocol> makeTm1814Bus(uint16_t pixelCount,
                                                                              const char *channelOrder,
                                                                              Tm1814CurrentSettings current,
                                                                              ResourceHandle<IShader<Rgbw8Color>> shader,
                                                                              typename TTransport::TransportConfigType transportConfig)
    {
        Tm1814ProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;
        protocolSettings.current = current;

        Tm1814WithShaderProtocol::SettingsType shaderSettings{};
        shaderSettings.shader = std::move(shader);

        return makeOwningDriverPixelBus<TTransport, Tm1814WithShaderProtocol>(pixelCount,
                                                                              std::move(transportConfig),
                                                                              std::move(shaderSettings),
                                                                              std::move(protocolSettings));
    }

    template <typename TTransport, typename TShader>
        requires TaggedTransportLike<TTransport, OneWireTransportTag> &&
                 std::derived_from<std::remove_cvref_t<TShader>, IShader<Rgbw8Color>>
    Tm1814OwningPixelBusT<TTransport, Tm1814WithEmbeddedShaderProtocol<std::remove_cvref_t<TShader>>> makeTm1814Bus(uint16_t pixelCount,
                                                                                                                        const char *channelOrder,
                                                                                                                        Tm1814CurrentSettings current,
                                                                                                                        TShader &&shader,
                                                                                                                        typename TTransport::TransportConfigType transportConfig)
    {
        using ShaderType = std::remove_cvref_t<TShader>;
        using ProtocolType = Tm1814WithEmbeddedShaderProtocol<ShaderType>;

        Tm1814ProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;
        protocolSettings.current = current;

        typename ProtocolType::SettingsType shaderSettings{};
        shaderSettings.shader = std::forward<TShader>(shader);

        return makeOwningDriverPixelBus<TTransport, ProtocolType>(pixelCount,
                                                                   std::move(transportConfig),
                                                                   std::move(shaderSettings),
                                                                   std::move(protocolSettings));
    }

    template <typename TTransport, typename TProtocol = Tm1914Protocol>
    using Tm1914OwningPixelBusT = OwningBusDriverPixelBusT<TTransport, TProtocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, OneWireTransportTag>
    Tm1914OwningPixelBusT<TTransport> makeTm1914Bus(uint16_t pixelCount,
                                                    const char *channelOrder,
                                                    Tm1914Mode mode,
                                                    typename TTransport::TransportConfigType transportConfig)
    {
        Tm1914ProtocolSettings settings{};
        settings.channelOrder = channelOrder;
        settings.mode = mode;

        return makeOwningDriverPixelBus<TTransport, Tm1914Protocol>(pixelCount,
                                                                    std::move(transportConfig),
                                                                    std::move(settings));
    }

    using Tm1914WithShaderProtocol = WithShader<Rgb8Color, Tm1914Protocol>;
    template <typename TShader = IShader<Rgb8Color>>
    using Tm1914WithEmbeddedShaderProtocol = WithEmbeddedShader<Rgb8Color, TShader, Tm1914Protocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, OneWireTransportTag>
    Tm1914OwningPixelBusT<TTransport, Tm1914WithShaderProtocol> makeTm1914Bus(uint16_t pixelCount,
                                                                              const char *channelOrder,
                                                                              Tm1914Mode mode,
                                                                              ResourceHandle<IShader<Rgb8Color>> shader,
                                                                              typename TTransport::TransportConfigType transportConfig)
    {
        Tm1914ProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;
        protocolSettings.mode = mode;

        Tm1914WithShaderProtocol::SettingsType shaderSettings{};
        shaderSettings.shader = std::move(shader);

        return makeOwningDriverPixelBus<TTransport, Tm1914WithShaderProtocol>(pixelCount,
                                                                              std::move(transportConfig),
                                                                              std::move(shaderSettings),
                                                                              std::move(protocolSettings));
    }

    template <typename TTransport, typename TShader>
        requires TaggedTransportLike<TTransport, OneWireTransportTag> &&
                 std::derived_from<std::remove_cvref_t<TShader>, IShader<Rgb8Color>>
    Tm1914OwningPixelBusT<TTransport, Tm1914WithEmbeddedShaderProtocol<std::remove_cvref_t<TShader>>> makeTm1914Bus(uint16_t pixelCount,
                                                                                                                        const char *channelOrder,
                                                                                                                        Tm1914Mode mode,
                                                                                                                        TShader &&shader,
                                                                                                                        typename TTransport::TransportConfigType transportConfig)
    {
        using ShaderType = std::remove_cvref_t<TShader>;
        using ProtocolType = Tm1914WithEmbeddedShaderProtocol<ShaderType>;

        Tm1914ProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;
        protocolSettings.mode = mode;

        typename ProtocolType::SettingsType shaderSettings{};
        shaderSettings.shader = std::forward<TShader>(shader);

        return makeOwningDriverPixelBus<TTransport, ProtocolType>(pixelCount,
                                                                   std::move(transportConfig),
                                                                   std::move(shaderSettings),
                                                                   std::move(protocolSettings));
    }

    template <typename TTransport,
              typename TColor = Rgb16Color,
              template <typename> typename TProtocolT = Hd108Protocol>
    using Hd108OwningPixelBusT = OwningBusDriverPixelBusT<TTransport, TProtocolT<TColor>>;

    template <typename TTransport, typename TColor = Rgb16Color>
        requires TaggedTransportLike<TTransport, TransportTag>
    Hd108OwningPixelBusT<TTransport, TColor> makeHd108Bus(uint16_t pixelCount,
                                                          const char *channelOrder,
                                                          typename TTransport::TransportConfigType transportConfig)
    {
        Hd108ProtocolSettings settings{};
        settings.channelOrder = channelOrder;

        return makeOwningDriverPixelBus<TTransport, Hd108Protocol<TColor>>(pixelCount,
                                                                           std::move(transportConfig),
                                                                           std::move(settings));
    }

    template <typename TColor = Rgb16Color>
    using Hd108WithShaderProtocolT = WithShader<TColor, Hd108Protocol<TColor>>;

    template <typename TColor = Rgb16Color, typename TShader = IShader<TColor>>
    using Hd108WithEmbeddedShaderProtocolT = WithEmbeddedShader<TColor, TShader, Hd108Protocol<TColor>>;

    template <typename TTransport, typename TColor = Rgb16Color>
        requires TaggedTransportLike<TTransport, TransportTag>
    Hd108OwningPixelBusT<TTransport, TColor, Hd108WithShaderProtocolT> makeHd108Bus(uint16_t pixelCount,
                                                                                    const char *channelOrder,
                                                                                    ResourceHandle<IShader<TColor>> shader,
                                                                                    typename TTransport::TransportConfigType transportConfig)
    {
        Hd108ProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        typename Hd108WithShaderProtocolT<TColor>::SettingsType shaderSettings{};
        shaderSettings.shader = std::move(shader);

        return makeOwningDriverPixelBus<TTransport, Hd108WithShaderProtocolT<TColor>>(pixelCount,
                                                                                      std::move(transportConfig),
                                                                                      std::move(shaderSettings),
                                                                                      std::move(protocolSettings));
    }

    template <typename TTransport, typename TColor = Rgb16Color, typename TShader>
        requires TaggedTransportLike<TTransport, TransportTag> &&
                 std::derived_from<std::remove_cvref_t<TShader>, IShader<TColor>>
    OwningBusDriverPixelBusT<TTransport, Hd108WithEmbeddedShaderProtocolT<TColor, std::remove_cvref_t<TShader>>> makeHd108Bus(uint16_t pixelCount,
                                                                                                                               const char *channelOrder,
                                                                                                                               TShader &&shader,
                                                                                                                               typename TTransport::TransportConfigType transportConfig)
    {
        using ShaderType = std::remove_cvref_t<TShader>;
        using ProtocolType = Hd108WithEmbeddedShaderProtocolT<TColor, ShaderType>;

        Hd108ProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        typename ProtocolType::SettingsType shaderSettings{};
        shaderSettings.shader = std::forward<TShader>(shader);

        return makeOwningDriverPixelBus<TTransport, ProtocolType>(pixelCount,
                                                                   std::move(transportConfig),
                                                                   std::move(shaderSettings),
                                                                   std::move(protocolSettings));
    }

    template <typename TColor>
    CurrentLimiterShader<TColor> makeCurrentLimiterShader(typename CurrentLimiterShader<TColor>::SettingsType settings = {})
    {
        return CurrentLimiterShader<TColor>(std::move(settings));
    }

    template <typename TColor>
    ResourceHandle<IShader<TColor>> makeOwnedCurrentLimiterShader(typename CurrentLimiterShader<TColor>::SettingsType settings = {})
    {
        return ResourceHandle<IShader<TColor>>(std::make_unique<CurrentLimiterShader<TColor>>(std::move(settings)));
    }

    template <typename TColor>
        requires ColorComponentTypeIs<TColor, uint8_t>
    GammaShader<TColor> makeGammaShader(typename GammaShader<TColor>::SettingsType settings = {})
    {
        return GammaShader<TColor>(std::move(settings));
    }

    template <typename TColor>
        requires ColorComponentTypeIs<TColor, uint8_t>
    ResourceHandle<IShader<TColor>> makeOwnedGammaShader(typename GammaShader<TColor>::SettingsType settings = {})
    {
        return ResourceHandle<IShader<TColor>>(std::make_unique<GammaShader<TColor>>(std::move(settings)));
    }

    template <typename TColor>
        requires ColorChannelsAtLeast<TColor, 4>
    WhiteBalanceShader<TColor> makeWhiteBalanceShader(typename WhiteBalanceShader<TColor>::SettingsType settings = {})
    {
        return WhiteBalanceShader<TColor>(std::move(settings));
    }

    template <typename TColor>
        requires ColorChannelsAtLeast<TColor, 4>
    ResourceHandle<IShader<TColor>> makeOwnedWhiteBalanceShader(typename WhiteBalanceShader<TColor>::SettingsType settings = {})
    {
        return ResourceHandle<IShader<TColor>>(std::make_unique<WhiteBalanceShader<TColor>>(std::move(settings)));
    }

    template <typename TColor, typename... TShaders>
        requires(sizeof...(TShaders) > 0 && (std::derived_from<std::remove_cvref_t<TShaders>, IShader<TColor>> && ...))
    OwningAggregateShaderT<TColor, std::remove_cvref_t<TShaders>...> makeAggregateShader(TShaders &&...shaders)
    {
        return OwningAggregateShaderT<TColor, std::remove_cvref_t<TShaders>...>(std::forward<TShaders>(shaders)...);
    }

    template <typename TColor, typename... TShaders>
        requires(sizeof...(TShaders) > 0 && (std::derived_from<std::remove_cvref_t<TShaders>, IShader<TColor>> && ...))
    ResourceHandle<IShader<TColor>> makeOwnedAggregateShader(TShaders &&...shaders)
    {
        using OwningShaderType = OwningAggregateShaderT<TColor, std::remove_cvref_t<TShaders>...>;
        return ResourceHandle<IShader<TColor>>(std::make_unique<OwningShaderType>(std::forward<TShaders>(shaders)...));
    }

} // namespace npb::factory
