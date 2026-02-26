#pragma once

#ifdef ARDUINO_ARCH_ESP8266

#include <cstdint>

#include "transports/ITransport.h"
#include "transports/OneWireWrapper.h"
#include "transports/OneWireTiming.h"
#include "transports/esp8266/Esp8266DmaUartTransport.h"

namespace npb
{

    struct Esp8266UartOneWireTransportSettings
    {
        uint8_t uartNumber = 1;
        bool invert = false;
        OneWireTiming timing = timing::Ws2812x;
    };

    class Esp8266UartOneWireTransport : public OneWireTransport<Esp8266DmaUartTransport>
    {
    public:
        using BaseType = OneWireTransport<Esp8266DmaUartTransport>;
        using TransportSettingsType = Esp8266UartOneWireTransportSettings;
        using TransportCategory = OneWireTransportTag;

        explicit Esp8266UartOneWireTransport(Esp8266UartOneWireTransportSettings config)
            : BaseType(makeBaseSettings(config))
        {
        }

    private:
        static typename BaseType::TransportSettingsType makeBaseSettings(const Esp8266UartOneWireTransportSettings &config)
        {
            typename BaseType::TransportSettingsType base{};
            base.uartNumber = config.uartNumber;
            base.invert = config.invert;
            base.baudRate = static_cast<uint32_t>(config.timing.bitRateHz()) * 4U;
            base.clockRateHz = base.baudRate;
            base.timing = config.timing;
            return base;
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_ESP8266


