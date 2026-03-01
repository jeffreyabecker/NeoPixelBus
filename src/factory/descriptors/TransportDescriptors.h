#pragma once

#include "transports/ITransport.h"

namespace lw
{
namespace factory
{
namespace descriptors
{

    struct NeoPrint
    {
        using Capability = lw::AnyTransportTag;
    };

    struct Nil
    {
        using Capability = lw::TransportTag;
    };

    struct NeoSpi
    {
        using Capability = lw::TransportTag;
    };

    struct RpPio
    {
        using Capability = lw::TransportTag;
    };

    struct RpSpi
    {
        using Capability = lw::TransportTag;
    };

    struct RpUart
    {
        using Capability = lw::TransportTag;
    };

    struct Esp32RmtOneWire
    {
        using Capability = lw::OneWireTransportTag;
    };

    struct Esp32I2s
    {
        using Capability = lw::TransportTag;
    };

    struct Esp32DmaSpi
    {
        using Capability = lw::TransportTag;
    };

    struct Esp8266DmaI2s
    {
        using Capability = lw::TransportTag;
    };

    struct Esp8266DmaUart
    {
        using Capability = lw::TransportTag;
    };

#if defined(ARDUINO_ARCH_ESP32)
    using PlatformDefault = Esp32I2s;
#elif defined(ARDUINO_ARCH_ESP8266)
    using PlatformDefault = Esp8266DmaI2s;
#elif defined(ARDUINO_ARCH_RP2040)
    using PlatformDefault = RpPio;
#elif defined(ARDUINO_ARCH_NATIVE) || !defined(ARDUINO) || !defined(NPB_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS)
    using PlatformDefault = Nil;
#else
    using PlatformDefault = NeoSpi;
#endif

} // namespace descriptors
} // namespace factory
} // namespace lw
