#pragma once

// Resource ownership
#include "virtual/ResourceHandle.h"

// Colors
#include "virtual/colors/Color.h"

// Emitters (includes internal transform details)
#include "virtual/emitters/IEmitPixels.h"
#include "virtual/emitters/PrintEmitter.h"
#include "virtual/emitters/DotStarEmitter.h"
#include "virtual/emitters/Hd108Emitter.h"
#include "virtual/emitters/Lpd8806Emitter.h"
#include "virtual/emitters/Lpd6803Emitter.h"
#include "virtual/emitters/P9813Emitter.h"
#include "virtual/emitters/Ws2801Emitter.h"
#include "virtual/emitters/Sm16716Emitter.h"
#include "virtual/emitters/Tlc59711Emitter.h"
#include "virtual/emitters/Tlc5947Emitter.h"
#include "virtual/emitters/PixieStreamEmitter.h"
#include "virtual/emitters/OneWireTiming.h"

// Platform one-wire emitters (guarded internally by ARDUINO_ARCH_*)
#ifdef ARDUINO_ARCH_RP2040
#include "virtual/emitters/RpPioOneWireEmitter.h"
#endif

#ifdef ARDUINO_ARCH_ESP32
#include "virtual/emitters/Esp32RmtOneWireEmitter.h"
#include "virtual/emitters/Esp32I2sOneWireEmitter.h"
#include "virtual/emitters/Esp32I2sParallelOneWireEmitter.h"
#include "virtual/emitters/Esp32LcdParallelOneWireEmitter.h"
#endif

#ifdef ARDUINO_ARCH_ESP8266
#include "virtual/emitters/Esp8266DmaOneWireEmitter.h"
#include "virtual/emitters/Esp8266UartOneWireEmitter.h"
#endif

#if defined(ARDUINO_ARCH_NRF52840)
#include "virtual/emitters/Nrf52PwmOneWireEmitter.h"
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
#include "virtual/buses/DebugClockDataTransport.h"
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
