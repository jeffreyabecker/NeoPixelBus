#pragma once

#include "factory/Traits.h"
#include "factory/busses/BusDriver.h"

#ifndef NPB_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS
#define NPB_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS 1
#endif

#include "factory/MakeBus.h"
#include "factory/MakeCompositeBus.h"
#include "factory/MakeShader.h"


using lw::factory::makeBus;
using lw::factory::makeShader;
using lw::factory::Bus;
using lw::factory::Shader;
using lw::factory::MosaicBus;
using lw::factory::ConcatBus;
using lw::factory::Gamma;
using lw::factory::CurrentLimiter;
using lw::factory::WhiteBalance;
using lw::factory::NoShader;
using lw::factory::PlatformDefaultOptions;

using lw::factory::descriptors::DotStar;
using lw::factory::descriptors::APA102;
using lw::factory::descriptors::Ws2812x;
using lw::factory::descriptors::Ws2812;
using lw::factory::descriptors::Ws2811;
using lw::factory::descriptors::Ws2805;
using lw::factory::descriptors::Sk6812;
using lw::factory::descriptors::Tm1814;
using lw::factory::descriptors::Tm1914;
using lw::factory::descriptors::Tm1829;
using lw::factory::descriptors::Apa106;
using lw::factory::descriptors::Tx1812;
using lw::factory::descriptors::Gs1903;
using lw::factory::descriptors::Generic800;
using lw::factory::descriptors::Generic400;
using lw::factory::descriptors::Ws2816;
using lw::factory::descriptors::Ws2813;
using lw::factory::descriptors::Ws2814;
using lw::factory::descriptors::Lc8812;
using lw::factory::descriptors::NeoPrint;
using lw::factory::descriptors::Nil;
using lw::factory::descriptors::PlatformDefault;

#if defined(NPB_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS)
using lw::factory::descriptors::NeoSpi;
#endif

#if defined(ARDUINO_ARCH_RP2040)
using lw::factory::descriptors::RpPio;
using lw::factory::descriptors::RpSpi;
using lw::factory::descriptors::RpUart;
#endif

#if defined(ARDUINO_ARCH_ESP32)

using lw::factory::descriptors::Esp32RmtOneWire;
using lw::factory::descriptors::Esp32I2s;
using lw::factory::descriptors::Esp32DmaSpi;
#endif

#if defined(ARDUINO_ARCH_ESP8266)

using lw::factory::descriptors::Esp8266DmaI2s;
using lw::factory::descriptors::Esp8266DmaUart;
#endif
