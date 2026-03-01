#pragma once

#include <cstdint>
#include <type_traits>

#include "protocols/IProtocol.h"
#include "transports/ITransport.h"

namespace lw
{

    template <typename TProtocol, typename = void>
    struct BusDriverProtocolLikeImpl : std::false_type
    {
    };

    template <typename TProtocol>
    struct BusDriverProtocolLikeImpl<TProtocol,
                                     std::void_t<typename TProtocol::ColorType,
                                                 typename TProtocol::TransportCategory>>
        : std::integral_constant<bool,
                                 std::is_base_of<IProtocol<typename TProtocol::ColorType>, TProtocol>::value>
    {
    };

    template <typename TProtocol>
    static constexpr bool BusDriverProtocolLike = BusDriverProtocolLikeImpl<TProtocol>::value;

    template <typename TProtocol, typename TTransport>
    static constexpr bool BusDriverProtocolSettingsConstructible =
        ProtocolPixelSettingsConstructible<TProtocol> ||
        std::is_constructible<TProtocol,
                              uint16_t,
                              typename TProtocol::SettingsType,
                              TTransport &>::value;

    template <typename TProtocol, typename TTransport>
    static constexpr bool BusDriverProtocolTransportCompatible =
        BusDriverProtocolLike<TProtocol> &&
        TransportLike<TTransport> &&
        TransportCategoryCompatible<typename TProtocol::TransportCategory,
                                    typename TTransport::TransportCategory>;

} // namespace lw
