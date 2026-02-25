#pragma once

#include <cstdint>
#include <utility>

#include "PixelBus.h"
#include "BusDriver.h"
#include "../protocols/Ws2812xProtocol.h"
#include "../protocols/WithShaderProtocol.h"
#include "../colors/IShader.h"

namespace npb::factory
{

    template <typename TTransport, typename TColor = Rgb8Color>
    using Ws2812xOwningPixelBusT = OwningPixelBusT<TTransport, Ws2812xProtocol<TColor>>;

    template <typename TTransport, typename TColor = Rgb8Color>
    using Ws2812xOwningBusDriverPixelBusT = OwningBusDriverPixelBusT<TTransport, Ws2812xProtocol<TColor>>;

    template <typename TColor = Rgb8Color>
    using Ws2812xWithShaderProtocolT = WithShader<TColor, Ws2812xProtocol<TColor>>;

    template <typename TTransport, typename TColor = Rgb8Color>
    using Ws2812xOwningShaderBusDriverPixelBusT = OwningBusDriverPixelBusT<TTransport, Ws2812xWithShaderProtocolT<TColor>>;

    template <typename TTransport, typename TColor = Rgb8Color>
        requires TaggedTransportLike<TTransport, OneWireTransportTag>
    Ws2812xOwningPixelBusT<TTransport, TColor> makeWs2812xOwningPixelBus(uint16_t pixelCount,
                                                                          const char *channelOrder,
                                                                          typename TTransport::TransportConfigType transportConfig)
    {
        Ws2812xProtocolSettings settings{};
        settings.channelOrder = channelOrder;

        return makeOwningPixelBus<TTransport, Ws2812xProtocol<TColor>>(pixelCount,
                                                                        std::move(transportConfig),
                                                                        pixelCount,
                                                                        std::move(settings));
    }

    template <typename TTransport, typename TColor = Rgb8Color>
        requires TaggedTransportLike<TTransport, OneWireTransportTag>
    Ws2812xOwningPixelBusT<TTransport, TColor> MakeWs2812PixelBus(uint16_t pixelCount,
                                                                   const char *channelOrder,
                                                                   typename TTransport::TransportConfigType transportConfig)
    {
        return makeWs2812xOwningPixelBus<TTransport, TColor>(pixelCount,
                                                              channelOrder,
                                                              std::move(transportConfig));
    }

    template <typename TTransport, typename TColor = Rgb8Color>
        requires TaggedTransportLike<TTransport, OneWireTransportTag>
    Ws2812xOwningBusDriverPixelBusT<TTransport, TColor> makeWs2812xOwningBusDriverPixelBus(uint16_t pixelCount,
                                                                                             const char *channelOrder,
                                                                                             typename TTransport::TransportConfigType transportConfig)
    {
        Ws2812xProtocolSettings settings{};
        settings.channelOrder = channelOrder;

        return makeOwningBusDriverPixelBus<TTransport, Ws2812xProtocol<TColor>>(pixelCount,
                                                                                  std::move(transportConfig),
                                                                                  pixelCount,
                                                                                  std::move(settings));
    }

    template <typename TTransport, typename TColor = Rgb8Color>
        requires TaggedTransportLike<TTransport, OneWireTransportTag>
    Ws2812xOwningBusDriverPixelBusT<TTransport, TColor> MakeWs2812BusDriverPixelBus(uint16_t pixelCount,
                                                                                      const char *channelOrder,
                                                                                      typename TTransport::TransportConfigType transportConfig)
    {
        return makeWs2812xOwningBusDriverPixelBus<TTransport, TColor>(pixelCount,
                                                                       channelOrder,
                                                                       std::move(transportConfig));
    }

    template <typename TTransport, typename TColor = Rgb8Color>
        requires TaggedTransportLike<TTransport, OneWireTransportTag>
    Ws2812xOwningShaderBusDriverPixelBusT<TTransport, TColor> makeWs2812xOwningShaderBusDriverPixelBus(uint16_t pixelCount,
                                                                                                          const char *channelOrder,
                                                                                                          ResourceHandle<IShader<TColor>> shader,
                                                                                                          typename TTransport::TransportConfigType transportConfig)
    {
        Ws2812xProtocolSettings protocolSettings{};
        protocolSettings.channelOrder = channelOrder;

        typename Ws2812xWithShaderProtocolT<TColor>::SettingsType shaderSettings{};
        shaderSettings.shader = std::move(shader);

        return makeOwningBusDriverPixelBus<TTransport, Ws2812xWithShaderProtocolT<TColor>>(pixelCount,
                                                                                             std::move(transportConfig),
                                                                                             pixelCount,
                                                                                             std::move(shaderSettings),
                                                                                             std::move(protocolSettings));
    }

    template <typename TTransport, typename TColor = Rgb8Color>
        requires TaggedTransportLike<TTransport, OneWireTransportTag>
    Ws2812xOwningShaderBusDriverPixelBusT<TTransport, TColor> MakeWs2812ShaderBusDriverPixelBus(uint16_t pixelCount,
                                                                                                   const char *channelOrder,
                                                                                                   ResourceHandle<IShader<TColor>> shader,
                                                                                                   typename TTransport::TransportConfigType transportConfig)
    {
        return makeWs2812xOwningShaderBusDriverPixelBus<TTransport, TColor>(pixelCount,
                                                                              channelOrder,
                                                                              std::move(shader),
                                                                              std::move(transportConfig));
    }

} // namespace npb::factory
