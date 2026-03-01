#pragma once

// Colors
#include "colors/Color.h"
#include "colors/ColorChannelIndexIterator.h"
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
#include "transports/rp2040/RpPioTransport.h"
#include "transports/rp2040/RpSpiTransport.h"
#include "transports/rp2040/RpUartTransport.h"
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
#include "transports/esp8266/Esp8266DmaI2sTransport.h"
#include "transports/esp8266/Esp8266DmaUartTransport.h"
#endif

#if defined(ARDUINO_ARCH_NRF52840)
#include "transports/nrf52/Nrf52PwmOneWireTransport.h"
#endif

#include "transports/PrintTransport.h"
#include "transports/SpiTransport.h"

// Topologies
#include "buses/PanelLayout.h"
#include "buses/Topology.h"
#include "topologies/PanelTopology.h"
#include "topologies/TiledTopology.h"

// Bus
#include "core/BufferHolder.h"
#include "core/IPixelBus.h"
#include "buses/PixelBus.h"
#include "factory/busses/BusDriver.h"
#include "buses/NilBus.h"
#ifndef NPB_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS
#define NPB_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS 1
#endif
#include "factory/MakeBus.h"
#include "factory/MakeCompositeBus.h"
#include "factory/MakeShader.h"

// Composite buses
#include "buses/ConcatBus.h"
#include "buses/SegmentBus.h"
#include "buses/MosaicBus.h"
