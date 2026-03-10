#pragma once

#ifdef ARDUINO_ARCH_ESP8266

#include "transports/AnalogPwmLightDriver.h"

namespace lw::transports::esp8266
{

using Esp8266LedcLightDriverSettings = lw::transports::AnalogPwmLightDriverSettings;

template <typename TColor> using Esp8266LedcLightDriver = lw::transports::AnalogPwmLightDriver<TColor>;

} // namespace lw::transports::esp8266

#endif // ARDUINO_ARCH_ESP8266
