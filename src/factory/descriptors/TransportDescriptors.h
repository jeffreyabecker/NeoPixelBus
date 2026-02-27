#pragma once

namespace npb
{
namespace factory
{
namespace descriptors
{

    struct NeoPrint
    {
    };

    struct Nil
    {
    };

    struct NeoSpi
    {
    };

    template <typename TTransportDesc>
    struct DebugTransport
    {
    };

    template <typename TTransportDesc>
    struct DebugOneWireTransport
    {
    };

    struct RpPioOneWire
    {
    };

    struct RpPioSpi
    {
    };

    struct Esp32RmtOneWire
    {
    };

    struct Esp32I2s
    {
    };

    struct Esp32DmaSpi
    {
    };

    struct Esp8266DmaI2s
    {
    };

    struct Esp8266DmaUart
    {
    };

} // namespace descriptors
} // namespace factory
} // namespace npb
