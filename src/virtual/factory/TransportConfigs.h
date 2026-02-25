#pragma once

#include "../ResourceHandle.h"
#include "../transports/DebugTransport.h"
#include "../transports/OneWireWrapper.h"
#include "../transports/PrintTransport.h"

#ifdef ARDUINO_ARCH_RP2040
#include "../transports/rp2040/RpPioOneWireTransport.h"
#include "../transports/rp2040/RpPioSpiTransport.h"
#endif

#ifdef ARDUINO_ARCH_ESP32
#include "../transports/esp32/Esp32DmaSpiTransport.h"
#include "../transports/esp32/Esp32I2sTransport.h"
#include "../transports/esp32/Esp32RmtOneWireTransport.h"
#endif

#ifdef ARDUINO_ARCH_ESP8266
#include "../transports/esp8266/Esp8266DmaTransport.h"
#include "../transports/esp8266/Esp8266UartOneWireTransport.h"
#endif

#if defined(ARDUINO_ARCH_NRF52840)
#include "../transports/nrf52/Nrf52PwmOneWireTransport.h"
#endif

namespace npb::factory
{

    template <typename TTransport>
    struct TransportConfig
    {
        using TransportType = TTransport;
        typename TTransport::TransportSettingsType settings{};
    };

    struct Debug
    {
        ResourceHandle<Print> output = nullptr;
        bool invert = false;
    };

    using NilTransportConfig = TransportConfig<NilTransport>;
    using PrintTransportConfig = TransportConfig<PrintTransport>;
    using DebugTransportConfig = TransportConfig<DebugTransport>;
    using DebugOneWireTransportConfig = TransportConfig<DebugOneWireTransport>;

    template <typename TTransport>
    using OneWire = TransportConfig<OneWireTransport<TTransport>>;

#ifdef ARDUINO_ARCH_RP2040
    using RpPioOneWire = TransportConfig<RpPioOneWireTransport>;
    using RpPioSpi = TransportConfig<RpPioSpiTransport>;
#endif

#ifdef ARDUINO_ARCH_ESP32
    using Esp32RmtOneWire = TransportConfig<Esp32RmtOneWireTransport>;
    using Esp32I2s = TransportConfig<Esp32I2sTransport>;
    using Esp32DmaSpi = TransportConfig<Esp32DmaSpiTransport>;
#endif

#ifdef ARDUINO_ARCH_ESP8266
    using Esp8266Dma = TransportConfig<Esp8266DmaTransport>;
    using Esp8266UartOneWire = TransportConfig<Esp8266UartOneWireTransport>;
#endif

#if defined(ARDUINO_ARCH_NRF52840)
    using Nrf52PwmOneWire = TransportConfig<Nrf52PwmOneWireTransport>;
#endif

} // namespace npb::factory
