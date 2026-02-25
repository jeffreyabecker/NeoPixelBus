#pragma once

// Resource ownership
#include "virtual/ResourceHandle.h"

// Colors
#include "virtual/colors/Color.h"

// Protocols (includes internal transform details)
#include "virtual/protocols/IProtocol.h"
#include "virtual/protocols/NilProtocol.h"
#include "virtual/protocols/WithShaderProtocol.h"
#include "virtual/protocols/DebugProtocol.h"
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
#include "virtual/colors/NilShader.h"
#include "virtual/colors/CurrentLimiterShader.h"
#include "virtual/colors/GammaShader.h"
#include "virtual/colors/WhiteBalanceShader.h"
#include "virtual/colors/AggregateShader.h"

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
#include "virtual/buses/NilBus.h"
#include "virtual/factory/MakeBus.h"

// Composite buses
#include "virtual/buses/ConcatBus.h"
#include "virtual/buses/SegmentBus.h"
#include "virtual/buses/MosaicBus.h"

// Factory API re-exports for unqualified consumer usage.
using npb::factory::Bus;

using npb::factory::ProtocolConfig;
using npb::factory::Ws2812x;
using npb::factory::Ws2812;
using npb::factory::Sk6812;
using npb::factory::Ucs8904;
using npb::factory::Nil;
using npb::factory::DebugProtocolConfig;
using npb::factory::DotStar;
using npb::factory::Hd108;
using npb::factory::Lpd6803;
using npb::factory::Lpd8806;
using npb::factory::P9813;
using npb::factory::Pixie;
using npb::factory::Sm16716;
using npb::factory::Sm168x;
using npb::factory::Tlc5947;
using npb::factory::Tlc59711;
using npb::factory::Tm1814;
using npb::factory::Tm1914;
using npb::factory::Ws2801;
using npb::factory::Ws2812xRaw;

using npb::factory::TransportConfig;
using npb::factory::Debug;
using npb::factory::NilTransportConfig;
using npb::factory::PrintTransportConfig;
using npb::factory::DebugTransportConfig;
using npb::factory::DebugOneWireTransportConfig;
using npb::factory::OneWire;
#ifdef ARDUINO_ARCH_RP2040
using npb::factory::RpPioOneWire;
using npb::factory::RpPioSpi;
#endif
#ifdef ARDUINO_ARCH_ESP32
using npb::factory::Esp32RmtOneWire;
using npb::factory::Esp32I2s;
using npb::factory::Esp32DmaSpi;
#endif
#ifdef ARDUINO_ARCH_ESP8266
using npb::factory::Esp8266Dma;
using npb::factory::Esp8266UartOneWire;
#endif
#if defined(ARDUINO_ARCH_NRF52840)
using npb::factory::Nrf52PwmOneWire;
#endif

using npb::factory::Gamma;
using npb::factory::CurrentLimiterRgb;
using npb::factory::CurrentLimiterRgbw;
using npb::factory::CurrentLimiterRgbcw;
using npb::factory::makeGammaShader;
using npb::factory::makeCurrentLimiterShader;
using npb::factory::makeAggregateShader;
using npb::factory::makeBus;

