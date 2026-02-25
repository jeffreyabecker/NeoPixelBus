#pragma once

#include <cstdint>
#include <utility>

#include "BusDriver.h"
#include "../colors/IShader.h"
#include "../protocols/WithShaderProtocol.h"
#include "../protocols/DotStarProtocol.h"
#include "../protocols/Ws2801Protocol.h"
#include "../protocols/PixieProtocol.h"

namespace npb::factory
{

    using DotStarWithShaderProtocol = WithShader<Rgb8Color, DotStarProtocol>;

    template <typename TTransport>
    using DotStarOwningShaderBusDriverPixelBusT = OwningBusDriverPixelBusT<TTransport, DotStarWithShaderProtocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag>
    DotStarOwningShaderBusDriverPixelBusT<TTransport> makeDotStarOwningShaderBusDriverPixelBus(uint16_t pixelCount,
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

        return makeOwningBusDriverPixelBus<TTransport, DotStarWithShaderProtocol>(pixelCount,
                                                                                   std::move(transportConfig),
                                                                                   pixelCount,
                                                                                   std::move(shaderSettings),
                                                                                   std::move(protocolSettings));
    }

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag>
    DotStarOwningShaderBusDriverPixelBusT<TTransport> MakeDotStarShaderBusDriverPixelBus(uint16_t pixelCount,
                                                                                           const char *channelOrder,
                                                                                           ResourceHandle<IShader<Rgb8Color>> shader,
                                                                                           typename TTransport::TransportConfigType transportConfig,
                                                                                           DotStarMode mode = DotStarMode::FixedBrightness)
    {
        return makeDotStarOwningShaderBusDriverPixelBus<TTransport>(pixelCount,
                                                                     channelOrder,
                                                                     std::move(shader),
                                                                     std::move(transportConfig),
                                                                     mode);
    }

    using Ws2801WithShaderProtocol = WithShader<Rgb8Color, Ws2801Protocol>;

    template <typename TTransport>
    using Ws2801OwningShaderBusDriverPixelBusT = OwningBusDriverPixelBusT<TTransport, Ws2801WithShaderProtocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag>
    Ws2801OwningShaderBusDriverPixelBusT<TTransport> makeWs2801OwningShaderBusDriverPixelBus(uint16_t pixelCount,
                                                                                               const char *channelOrder,
                                                                                               ResourceHandle<IShader<Rgb8Color>> shader,
                                                                                               typename TTransport::TransportConfigType transportConfig)
    {
        Ws2801ProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        Ws2801WithShaderProtocol::SettingsType shaderSettings{};
        shaderSettings.shader = std::move(shader);

        return makeOwningBusDriverPixelBus<TTransport, Ws2801WithShaderProtocol>(pixelCount,
                                                                                  std::move(transportConfig),
                                                                                  pixelCount,
                                                                                  std::move(shaderSettings),
                                                                                  std::move(protocolSettings));
    }

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, TransportTag>
    Ws2801OwningShaderBusDriverPixelBusT<TTransport> MakeWs2801ShaderBusDriverPixelBus(uint16_t pixelCount,
                                                                                         const char *channelOrder,
                                                                                         ResourceHandle<IShader<Rgb8Color>> shader,
                                                                                         typename TTransport::TransportConfigType transportConfig)
    {
        return makeWs2801OwningShaderBusDriverPixelBus<TTransport>(pixelCount,
                                                                    channelOrder,
                                                                    std::move(shader),
                                                                    std::move(transportConfig));
    }

    using PixieWithShaderProtocol = WithShader<Rgb8Color, PixieProtocol>;

    template <typename TTransport>
    using PixieOwningShaderBusDriverPixelBusT = OwningBusDriverPixelBusT<TTransport, PixieWithShaderProtocol>;

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, OneWireTransportTag>
    PixieOwningShaderBusDriverPixelBusT<TTransport> makePixieOwningShaderBusDriverPixelBus(uint16_t pixelCount,
                                                                                             const char *channelOrder,
                                                                                             ResourceHandle<IShader<Rgb8Color>> shader,
                                                                                             typename TTransport::TransportConfigType transportConfig)
    {
        PixieProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        PixieWithShaderProtocol::SettingsType shaderSettings{};
        shaderSettings.shader = std::move(shader);

        return makeOwningBusDriverPixelBus<TTransport, PixieWithShaderProtocol>(pixelCount,
                                                                                 std::move(transportConfig),
                                                                                 pixelCount,
                                                                                 std::move(shaderSettings),
                                                                                 std::move(protocolSettings));
    }

    template <typename TTransport>
        requires TaggedTransportLike<TTransport, OneWireTransportTag>
    PixieOwningShaderBusDriverPixelBusT<TTransport> MakePixieShaderBusDriverPixelBus(uint16_t pixelCount,
                                                                                       const char *channelOrder,
                                                                                       ResourceHandle<IShader<Rgb8Color>> shader,
                                                                                       typename TTransport::TransportConfigType transportConfig)
    {
        return makePixieOwningShaderBusDriverPixelBus<TTransport>(pixelCount,
                                                                   channelOrder,
                                                                   std::move(shader),
                                                                   std::move(transportConfig));
    }

} // namespace npb::factory
