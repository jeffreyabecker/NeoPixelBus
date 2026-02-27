#pragma once

#include "transports/ITransport.h"

namespace npb
{
namespace factory
{
namespace descriptors
{

    struct NeoPrint
    {
        using Capability = npb::AnyTransportTag;
    };

    struct Nil
    {
        using Capability = npb::TransportTag;
    };

    struct NeoSpi
    {
        using Capability = npb::TransportTag;
    };

    struct RpPio
    {
        using Capability = npb::TransportTag;
    };

    struct RpSpi
    {
        using Capability = npb::TransportTag;
    };

    struct RpUart
    {
        using Capability = npb::TransportTag;
    };

    struct Esp32RmtOneWire
    {
        using Capability = npb::OneWireTransportTag;
    };

    struct Esp32I2s
    {
        using Capability = npb::TransportTag;
    };

    struct Esp32DmaSpi
    {
        using Capability = npb::TransportTag;
    };

    struct Esp8266DmaI2s
    {
        using Capability = npb::TransportTag;
    };

    struct Esp8266DmaUart
    {
        using Capability = npb::TransportTag;
    };

} // namespace descriptors
} // namespace factory
} // namespace npb
