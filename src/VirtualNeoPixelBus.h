#pragma once

// Resource ownership
#include "virtual/ResourceHandle.h"

// Colors
#include "virtual/colors/Color.h"

// Protocols (includes internal transform details)
#include "virtual/protocols/IProtocol.h"
#include "virtual/protocols/WithShaderProtocol.h"
#include "virtual/protocols/PrintProtocol.h"
#include "virtual/protocols/DotStarProtocol.h"
#include "virtual/protocols/Hd108Protocol.h"
#include "virtual/protocols/Lpd8806Protocol.h"
#include "virtual/protocols/Lpd6803Protocol.h"
#include "virtual/protocols/P9813Protocol.h"
#include "virtual/protocols/Ws2801Protocol.h"
#include "virtual/protocols/Ws2812xProtocol.h"
#include "virtual/protocols/Sm16716Protocol.h"
#include "virtual/protocols/Tlc59711Protocol.h"
#include "virtual/protocols/Tlc5947Protocol.h"
#include "virtual/protocols/PixieProtocol.h"
#include "virtual/protocols/Tm1814Protocol.h"
#include "virtual/protocols/Tm1914Protocol.h"
#include "virtual/protocols/Sm168xProtocol.h"
#include "virtual/transports/OneWireTiming.h"

// Platform one-wire transports (guarded internally by ARDUINO_ARCH_*)
#ifdef ARDUINO_ARCH_RP2040
#include "virtual/transports/rp2040/RpPioOneWireTransport.h"
#include "virtual/transports/rp2040/RpPioSpiTransport.h"
#endif

#ifdef ARDUINO_ARCH_ESP32
#include "virtual/transports/esp32/Esp32RmtOneWireTransport.h"
#endif

#ifdef ARDUINO_ARCH_ESP8266
#endif

#if defined(ARDUINO_ARCH_NRF52840)
#endif

// Shaders
#include "virtual/colors/IShader.h"
#include "virtual/colors/CurrentLimiterShader.h"
#include "virtual/colors/GammaShader.h"
#include "virtual/colors/WhiteBalanceShader.h"
#include "virtual/colors/ShadedTransform.h"

// Buses
#include "virtual/transports/ITransport.h"
#include "virtual/transports/OneWireWrapper.h"

#ifdef ARDUINO_ARCH_ESP32
#include "virtual/transports/esp32/Esp32DmaSpiTransport.h"
#include "virtual/transports/esp32/Esp32I2sTransport.h"
#endif

#ifdef ARDUINO_ARCH_ESP8266
#include "virtual/transports/esp8266/Esp8266DmaTransport.h"
#include "virtual/transports/esp8266/Esp8266UartOneWireTransport.h"
#endif

#if defined(ARDUINO_ARCH_NRF52840)
#include "virtual/transports/nrf52/Nrf52PwmOneWireTransport.h"
#endif

#include "virtual/transports/PrintTransport.h"
#include "virtual/transports/DebugTransport.h"

// Topologies
#include "virtual/topologies/PanelLayout.h"
#include "virtual/topologies/PanelTopology.h"
#include "virtual/topologies/TiledTopology.h"

// Bus
#include "virtual/IPixelBus.h"
#include "virtual/buses/PixelBus.h"
#include "virtual/buses/BusDriver.h"

// Composite buses
#include "virtual/buses/ConcatBus.h"
#include "virtual/buses/SegmentBus.h"
#include "virtual/buses/MosaicBus.h"

// Factory API
#include "virtual/buses/Ws2812xFactory.h"
