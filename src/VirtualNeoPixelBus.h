#pragma once

// Resource ownership
#include "virtual/ResourceHandle.h"

// Colors
#include "virtual/colors/Color.h"

// Emitters (includes internal transform details)
#include "virtual/emitters/IProtocol.h"
#include "virtual/emitters/PrintProtocol.h"
#include "virtual/emitters/DotStarProtocol.h"
#include "virtual/emitters/Hd108Protocol.h"
#include "virtual/emitters/Lpd8806Protocol.h"
#include "virtual/emitters/Lpd6803Protocol.h"
#include "virtual/emitters/P9813Protocol.h"
#include "virtual/emitters/Ws2801Protocol.h"
#include "virtual/emitters/Sm16716Protocol.h"
#include "virtual/emitters/Tlc59711Protocol.h"
#include "virtual/emitters/Tlc5947Protocol.h"
#include "virtual/emitters/PixieProtocol.h"
#include "virtual/emitters/PixieStreamProtocol.h"
#include "virtual/emitters/Dmx512Protocol.h"
#include "virtual/emitters/OneWireTiming.h"

// Platform one-wire emitters (guarded internally by ARDUINO_ARCH_*)
#ifdef ARDUINO_ARCH_RP2040
#include "virtual/emitters/RpPioOneWireProtocol.h"
#endif

#ifdef ARDUINO_ARCH_ESP32
#include "virtual/emitters/Esp32RmtOneWireProtocol.h"
#include "virtual/emitters/Esp32I2sOneWireProtocol.h"
#include "virtual/emitters/Esp32I2sParallelOneWireProtocol.h"
#include "virtual/emitters/Esp32LcdParallelOneWireProtocol.h"
#endif

#ifdef ARDUINO_ARCH_ESP8266
#include "virtual/emitters/Esp8266DmaOneWireProtocol.h"
#include "virtual/emitters/Esp8266UartOneWireProtocol.h"
#endif

#if defined(ARDUINO_ARCH_NRF52840)
#include "virtual/emitters/Nrf52PwmOneWireProtocol.h"
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
#include "virtual/shaders/ShadedTransform.h"

// Buses
#include "virtual/buses/IClockDataTransport.h"
#include "virtual/buses/ISelfClockingTransport.h"
#include "virtual/buses/SelfClockingTransportConfig.h"
#include "virtual/buses/Esp32DmaSpiClockDataTransport.h"
#include "virtual/buses/Esp32I2SClockDataTransport.h"
#include "virtual/buses/Esp8266I2SClockDataTransport.h"
#include "virtual/buses/Esp8266I2SSelfClockingTransport.h"
#include "virtual/buses/PrintClockDataTransport.h"
#include "virtual/buses/DebugClockDataTransport.h"
#include "virtual/buses/DebugSelfClockingTransport.h"
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
