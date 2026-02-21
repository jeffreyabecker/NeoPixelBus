/*-------------------------------------------------------------------------
NeoMethods includes all the classes that describe pulse/data sending methods using
bitbang, SPI, or other platform specific hardware peripheral support.  

Written by Michael C. Miller.

I invest time and resources providing this open source code,
please support me by donating (see https://github.com/Makuna/NeoPixelBus)

-------------------------------------------------------------------------
This file is part of the Makuna/NeoPixelBus library.

NeoPixelBus is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

NeoPixelBus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with NeoPixel.  If not, see
<http://www.gnu.org/licenses/>.
-------------------------------------------------------------------------*/
#pragma once

// helper constants for method speeds and invert
#include "methods/common/NeoBits.h"

// Generic Two Wire (clk and data) methods
//
#if __has_include(<SPI.h>)
#include "methods/common/DotStarGenericMethod.h"
#include "methods/common/Lpd8806GenericMethod.h"
#include "methods/common/Lpd6803GenericMethod.h"
#include "methods/common/Ws2801GenericMethod.h"
#include "methods/common/P9813GenericMethod.h"
#include "methods/common/Tlc5947GenericMethod.h"
#include "methods/common/Tlc59711GenericMethod.h"
#include "methods/common/Sm16716GenericMethod.h"
#include "methods/common/Mbi6033GenericMethod.h"
#include "methods/common/Hd108GenericMethod.h"
#endif

//Adafruit Pixie via UART, not platform specific
//
#include "methods/common/PixieStreamMethod.h"

// Platform specific and One Wire (data) methods
//
#if defined(ARDUINO_ARCH_ESP8266)

#include "methods/platform/esp8266/NeoEsp8266DmaMethod.h"
#include "methods/platform/esp8266/NeoEsp8266I2sDmx512Method.h"
#include "methods/platform/esp8266/NeoEsp8266UartMethod.h"
#include "methods/platform/esp8266/NeoEspBitBangMethod.h"

#elif defined(ARDUINO_ARCH_ESP32)

#if !defined(CONFIG_IDF_TARGET_ESP32C6) && !defined(CONFIG_IDF_TARGET_ESP32H2)
#include "methods/platform/esp32/NeoEsp32I2sMethod.h"
#include "methods/platform/esp32/NeoEsp32RmtMethod.h"
#include "methods/platform/esp32/DotStarEsp32DmaSpiMethod.h"
#include "methods/platform/esp32/NeoEsp32I2sXMethod.h"
#include "methods/platform/esp32/NeoEsp32LcdXMethod.h"


#endif
#include "methods/platform/esp8266/NeoEspBitBangMethod.h"

#elif defined(ARDUINO_ARCH_NRF52840) // must be before __arm__

#include "methods/platform/nrf52/NeoNrf52xMethod.h"

#elif defined(ARDUINO_ARCH_RP2040) // must be before __arm__

#include "methods/platform/rp2040/NeoRp2040x4Method.h"

#elif defined(__arm__) // must be before ARDUINO_ARCH_AVR due to Teensy incorrectly having it set

#include "methods/platform/arm/NeoArmMethod.h"

#elif defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_MEGAAVR)

#include "methods/platform/avr/NeoAvrMethod.h"

#else
#error "Platform Currently Not Supported, please add an Issue at Github/Makuna/NeoPixelBus"
#endif
