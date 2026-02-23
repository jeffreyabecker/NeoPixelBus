#pragma once

// Resource ownership
#include "virtual/ResourceHandle.h"

// Colors
#include "virtual/colors/Color.h"

// Protocols (includes internal transform details)
#include "virtual/protocols/IProtocol.h"
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
#include "virtual/protocols/Dmx512Protocol.h"
#include "virtual/protocols/Tm1814Protocol.h"
#include "virtual/protocols/Tm1914Protocol.h"
#include "virtual/protocols/Sm168xProtocol.h"
#include "virtual/transports/OneWireTiming.h"

// Platform one-wire transports (guarded internally by ARDUINO_ARCH_*)
#ifdef ARDUINO_ARCH_RP2040
#include "virtual/transports/RpPioSelfClockingTransport.h"
#endif

#ifdef ARDUINO_ARCH_ESP32
#include "virtual/transports/Esp32RmtSelfClockingTransport.h"
#endif

#ifdef ARDUINO_ARCH_ESP8266
#endif

#if defined(ARDUINO_ARCH_NRF52840)
#endif

// Shaders
#include "virtual/shaders/IShader.h"
#include "virtual/shaders/GammaNullMethod.h"
#include "virtual/shaders/GammaEquationMethod.h"
#include "virtual/shaders/GammaCieLabMethod.h"
#include "virtual/shaders/GammaTableMethod.h"
#include "virtual/shaders/GammaDynamicTableMethod.h"
#include "virtual/shaders/GammaInvertMethod.h"
#include "virtual/shaders/GammaShader.h"
#include "virtual/shaders/CurrentLimiterShader.h"
#include "virtual/shaders/WhiteBalanceShader.h"
#include "virtual/shaders/ShadedTransform.h"

// Buses
#include "virtual/transports/IClockDataTransport.h"
#include "virtual/transports/ISelfClockingTransport.h"
#include "virtual/transports/SelfClockingTransportConfig.h"
#include "virtual/transports/Esp32DmaSpiClockDataTransport.h"
#include "virtual/transports/Esp32I2SClockDataTransport.h"
#include "virtual/transports/Esp8266I2SClockDataTransport.h"
#include "virtual/transports/Esp32I2sSelfClockingTransport.h"
#include "virtual/transports/Esp32I2sParallelSelfClockingTransport.h"
#include "virtual/transports/Esp32LcdParallelSelfClockingTransport.h"
#include "virtual/transports/Esp8266DmaSelfClockingTransport.h"
#include "virtual/transports/Esp8266UartSelfClockingTransport.h"
#include "virtual/transports/Esp8266I2SSelfClockingTransport.h"
#include "virtual/transports/Nrf52PwmSelfClockingTransport.h"
#include "virtual/transports/PrintClockDataTransport.h"
#include "virtual/transports/DebugClockDataTransport.h"
#include "virtual/transports/DebugSelfClockingTransport.h"
// SpiClockDataTransport.h excluded — include directly when SPI is available

// Topologies
#include "virtual/topologies/PanelLayout.h"
#include "virtual/topologies/PanelTopology.h"
#include "virtual/topologies/TiledTopology.h"

// Bus
#include "virtual/IPixelBus.h"
#include "virtual/PixelBus.h"

// Composite buses
#include "virtual/ConcatBus.h"
#include "virtual/SegmentBus.h"
#include "virtual/MosaicBus.h"
