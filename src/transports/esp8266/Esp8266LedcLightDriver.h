#pragma once

#ifdef ARDUINO_ARCH_ESP8266

#include "transports/esp32/Esp32LedcLightDriver.h"

namespace lw::transports::esp8266
{

    using Esp8266LedcLightDriverSettings = lw::transports::esp32::Esp32LedcLightDriverSettings;

    template <typename TColor>
    using Esp8266LedcLightDriver = lw::transports::esp32::Esp32LedcLightDriver<TColor>;

} // namespace lw::transports::esp8266

#endif // ARDUINO_ARCH_ESP8266
