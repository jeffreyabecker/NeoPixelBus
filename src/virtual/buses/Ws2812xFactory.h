#pragma once

#include <cstdint>
#include <utility>

#include "PixelBus.h"
#include "../protocols/Ws2812xProtocol.h"

namespace npb::factory
{

    template <typename TTransport, typename TColor = Rgb8Color>
    using Ws2812xOwningPixelBusT = OwningPixelBusT<TTransport, Ws2812xProtocol<TColor>>;

    template <typename TTransport, typename TColor = Rgb8Color>
        requires TaggedTransportLike<TTransport, SelfClockingTransportTag>
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
        requires TaggedTransportLike<TTransport, SelfClockingTransportTag>
    Ws2812xOwningPixelBusT<TTransport, TColor> MakeWs2812PixelBus(uint16_t pixelCount,
                                                                   const char *channelOrder,
                                                                   typename TTransport::TransportConfigType transportConfig)
    {
        return makeWs2812xOwningPixelBus<TTransport, TColor>(pixelCount,
                                                              channelOrder,
                                                              std::move(transportConfig));
    }

} // namespace npb::factory
