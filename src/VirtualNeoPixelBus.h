#pragma once

// Resource ownership
#include "core/ResourceHandle.h"

// Colors
#include "colors/Color.h"
#include "colors/HueBlend.h"
#include "colors/HslColor.h"
#include "colors/HsbColor.h"
#include "colors/ColorMath.h"

// Protocols (includes internal transform details)
#include "protocols/IProtocol.h"
#include "protocols/NilProtocol.h"
#include "protocols/WithShaderProtocol.h"
#include "protocols/DebugProtocol.h"
#include "protocols/DotStarProtocol.h"
#include "protocols/Hd108Protocol.h"
#include "protocols/Lpd8806Protocol.h"
#include "protocols/Lpd6803Protocol.h"
#include "protocols/P9813Protocol.h"
#include "protocols/Ws2801Protocol.h"
#include "protocols/Ws2812xProtocol.h"
#include "protocols/Sm16716Protocol.h"
#include "protocols/Tlc59711Protocol.h"
#include "protocols/Tlc5947Protocol.h"
#include "protocols/PixieProtocol.h"
#include "protocols/Tm1814Protocol.h"
#include "protocols/Tm1914Protocol.h"
#include "protocols/Sm168xProtocol.h"
#include "transports/OneWireTiming.h"

// Platform one-wire transports (guarded internally by ARDUINO_ARCH_*)
#ifdef ARDUINO_ARCH_RP2040
#include "transports/rp2040/RpPioOneWireTransport.h"
#include "transports/rp2040/RpPioSpiTransport.h"
#endif

#ifdef ARDUINO_ARCH_ESP32
#include "transports/esp32/Esp32RmtOneWireTransport.h"
#endif

#ifdef ARDUINO_ARCH_ESP8266
#endif

#if defined(ARDUINO_ARCH_NRF52840)
#endif

// Shaders
#include "colors/IShader.h"
#include "colors/NilShader.h"
#include "colors/CurrentLimiterShader.h"
#include "colors/GammaShader.h"
#include "colors/WhiteBalanceShader.h"
#include "colors/AggregateShader.h"

// Buses
#include "transports/ITransport.h"
#include "transports/OneWireWrapper.h"

#ifdef ARDUINO_ARCH_ESP32
#include "transports/esp32/Esp32DmaSpiTransport.h"
#include "transports/esp32/Esp32I2sTransport.h"
#endif

#ifdef ARDUINO_ARCH_ESP8266
#include "transports/esp8266/Esp8266DmaTransport.h"
#include "transports/esp8266/Esp8266UartOneWireTransport.h"
#endif

#if defined(ARDUINO_ARCH_NRF52840)
#include "transports/nrf52/Nrf52PwmOneWireTransport.h"
#endif

#include "transports/PrintTransport.h"
#include "transports/DebugTransport.h"

// Topologies
#include "topologies/PanelLayout.h"
#include "topologies/PanelTopology.h"
#include "topologies/TiledTopology.h"

// Bus
#include "core/IPixelBus.h"
#include "buses/PixelBus.h"
#include "buses/BusDriver.h"
#include "buses/NilBus.h"
#include "factory/MakeBus.h"

// Composite buses
#include "buses/ConcatBus.h"
#include "buses/SegmentBus.h"
#include "buses/MosaicBus.h"

// Factory API re-exports for unqualified consumer usage.
using npb::factory::Bus;

using npb::factory::ProtocolConfig;
using npb::factory::Ws2812x;
using npb::factory::Ws2812;
using npb::factory::Sk6812;
using npb::factory::Ucs8904;
using npb::factory::Nil;
using npb::factory::DebugProtocolConfig;
using npb::factory::debugProtocolOutput;
using npb::factory::debugProtocolSerial;
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
using npb::factory::printOutput;
using npb::factory::printSerial;
using npb::factory::debugOutput;
using npb::factory::debugSerial;
using npb::factory::debugTransportOutput;
using npb::factory::debugTransportSerial;
using npb::factory::debugOneWireOutput;
using npb::factory::debugOneWireSerial;
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
using npb::factory::ChannelMilliamps;
using npb::factory::makeGammaShader;
using npb::factory::makeCurrentLimiterShader;
using npb::factory::makeAggregateShader;
using npb::factory::makeBus;

