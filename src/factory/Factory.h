#pragma once

#include "factory/Traits.h"
#include "factory/busses/BusDriver.h"

#ifndef NPB_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS
#define NPB_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS 1
#endif

#include "factory/MakeBus.h"
#include "factory/MakeCompositeBus.h"
#include "factory/MakeShader.h"


using npb::factory::makeBus;
using npb::factory::makeShader;
using npb::factory::Bus;
using npb::factory::Shader;
using npb::factory::MosaicBus;
using npb::factory::ConcatBus;
using npb::factory::Gamma;
using npb::factory::CurrentLimiter;
using npb::factory::WhiteBalance;
using npb::factory::NoShader;
using npb::factory::PlatformDefaultOptions;

using npb::factory::descriptors::DotStar;
using npb::factory::descriptors::APA102;
using npb::factory::descriptors::Ws2812x;
using npb::factory::descriptors::Ws2812;
using npb::factory::descriptors::Ws2811;
using npb::factory::descriptors::Ws2805;
using npb::factory::descriptors::Sk6812;
using npb::factory::descriptors::Tm1814;
using npb::factory::descriptors::Tm1914;
using npb::factory::descriptors::Tm1829;
using npb::factory::descriptors::Apa106;
using npb::factory::descriptors::Tx1812;
using npb::factory::descriptors::Gs1903;
using npb::factory::descriptors::Generic800;
using npb::factory::descriptors::Generic400;
using npb::factory::descriptors::Ws2816;
using npb::factory::descriptors::Ws2813;
using npb::factory::descriptors::Ws2814;
using npb::factory::descriptors::Lc8812;
using npb::factory::descriptors::NeoPrint;
using npb::factory::descriptors::Nil;
using npb::factory::descriptors::PlatformDefault;

#if defined(NPB_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS)
using npb::factory::descriptors::NeoSpi;
#endif

#if defined(ARDUINO_ARCH_RP2040)
using npb::factory::descriptors::RpPio;
using npb::factory::descriptors::RpSpi;
using npb::factory::descriptors::RpUart;
#endif

#if defined(ARDUINO_ARCH_ESP32)

using npb::factory::descriptors::Esp32RmtOneWire;
using npb::factory::descriptors::Esp32I2s;
using npb::factory::descriptors::Esp32DmaSpi;
#endif

#if defined(ARDUINO_ARCH_ESP8266)

using npb::factory::descriptors::Esp8266DmaI2s;
using npb::factory::descriptors::Esp8266DmaUart;
#endif
