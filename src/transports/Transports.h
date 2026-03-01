#pragma once

#include "transports/ITransport.h"
#include "transports/NilTransport.h"
#include "transports/OneWireTiming.h"
#include "transports/OneWireWrapper.h"
#include "transports/PrintTransport.h"
#include "transports/SpiTransport.h"

#ifdef ARDUINO_ARCH_RP2040
#include "transports/rp2040/RpPioTransport.h"
#include "transports/rp2040/RpSpiTransport.h"
#include "transports/rp2040/RpUartTransport.h"
#endif

#ifdef ARDUINO_ARCH_ESP32
#include "transports/esp32/Esp32DmaSpiTransport.h"
#include "transports/esp32/Esp32I2sTransport.h"
#include "transports/esp32/Esp32RmtOneWireTransport.h"
#endif

#ifdef ARDUINO_ARCH_ESP8266
#include "transports/esp8266/Esp8266DmaI2sTransport.h"
#include "transports/esp8266/Esp8266DmaUartTransport.h"
#endif

#if defined(ARDUINO_ARCH_NRF52840)
#include "transports/nrf52/Nrf52PwmOneWireTransport.h"
#endif
