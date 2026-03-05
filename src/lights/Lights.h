#pragma once

#include "lights/ILightDriver.h"
#include "lights/NilLightDriver.h"
#include "lights/PrintLightDriver.h"
#include "lights/esp32/Esp32LedcLightDriver.h"

#ifdef ARDUINO_ARCH_RP2040
#include "lights/rp2040/RpPwmLightDriver.h"
#endif

#ifdef ARDUINO_ARCH_ESP8266
#include "lights/AnalogPwmLightDriver.h"
#endif

#ifdef ARDUINO_ARCH_ESP32
#include "lights/esp32/Esp32SigmaDeltaLightDriver.h"
#endif

