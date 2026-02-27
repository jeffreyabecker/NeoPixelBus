#pragma once

namespace npb
{
namespace factory
{
namespace descriptors
{

    struct PrintTransport
    {
    };

    struct NilTransport
    {
    };

    struct SpiTransport
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

    struct RpPioOneWireTransport
    {
    };

    struct RpPioSpiTransport
    {
    };

    struct Esp32RmtOneWireTransport
    {
    };

    struct Esp32I2sTransport
    {
    };

    struct Esp32DmaSpiTransport
    {
    };

    struct Esp8266DmaI2sTransport
    {
    };

    struct Esp8266DmaUartTransport
    {
    };

    struct Esp8266UartOneWireTransport
    {
    };

} // namespace descriptors
} // namespace factory
} // namespace npb
