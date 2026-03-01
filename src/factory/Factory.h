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
using npb::factory::Gamma;
using npb::factory::CurrentLimiter;
using npb::factory::WhiteBalance;
using npb::factory::NoShader;
using npb::factory::PlatformDefaultOptions;

using npb::factory::descriptors::DotStar;
using npb::factory::descriptors::APA102;
using npb::factory::descriptors::Ws2812x;
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
