#pragma once

#include <cstdint>
#include <type_traits>

#include "protocols/IProtocol.h"
#include "transports/ITransport.h"

namespace lw
{

    template <typename TProtocol, typename = void>
    struct ProtocolLikeImpl : std::false_type
    {
    };

    template <typename TProtocol>
    struct ProtocolLikeImpl<TProtocol,
                            std::void_t<typename TProtocol::ColorType,
                                        typename TProtocol::SettingsType>>
        : std::true_type
    {
    };

    template <typename TProtocol>
    static constexpr bool ProtocolLike = ProtocolLikeImpl<TProtocol>::value;

    template <typename TProtocol, typename TTransport>
    static constexpr bool ProtocolSettingsConstructibleWithTransport =
        ProtocolPixelSettingsConstructible<TProtocol> ||
        std::is_constructible<TProtocol,
                              PixelCount,
                              typename TProtocol::SettingsType,
                              TTransport &>::value;

    template <typename TProtocol, typename TTransport>
    static constexpr bool ProtocolTransportCompatible =
        ProtocolLike<TProtocol> &&
        TransportLike<TTransport>;

} // namespace lw