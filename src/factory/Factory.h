#pragma once

#include "buses/composite/CompositeBusConfig.h"

#ifndef LW_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS
#define LW_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS 1
#endif

#ifndef LW_FACTORY_ENABLE_STATIC
#define LW_FACTORY_ENABLE_STATIC 1
#endif

#ifndef LW_FACTORY_ENABLE_DYNAMIC_BUILDER
#define LW_FACTORY_ENABLE_DYNAMIC_BUILDER 1
#endif

#ifndef LW_FACTORY_ENABLE_INI
#define LW_FACTORY_ENABLE_INI 1
#endif

#if LW_ENABLE_COMPOSITE_BUS
#define LW_FACTORY_ENABLE_STATIC_BUS 1
#define LW_FACTORY_ENABLE_DYNAMIC_BUILDER_API LW_FACTORY_ENABLE_DYNAMIC_BUILDER
#else
#define LW_FACTORY_ENABLE_STATIC_BUS 0
#define LW_FACTORY_ENABLE_DYNAMIC_BUILDER_API 0
#endif

#if !LW_FACTORY_ENABLE_DYNAMIC_BUILDER_API
#undef LW_FACTORY_ENABLE_INI
#define LW_FACTORY_ENABLE_INI 0
#endif

#if LW_FACTORY_ENABLE_DYNAMIC_BUILDER_API && LW_FACTORY_ENABLE_INI
#define LW_FACTORY_ENABLE_DYNAMIC_INI_API 1
#else
#define LW_FACTORY_ENABLE_DYNAMIC_INI_API 0
#endif

// Current dynamic factory surface is INI-backed, so it depends on both builder and INI APIs.
#if LW_FACTORY_ENABLE_DYNAMIC_INI_API
#define LW_FACTORY_ENABLE_DYNAMIC_FACTORY_API 1
#else
#define LW_FACTORY_ENABLE_DYNAMIC_FACTORY_API 0
#endif

#ifndef LW_MAIN_HEADER_ENABLE_GLOBAL_NAMESPACE_IMPORTS
#define LW_MAIN_HEADER_ENABLE_GLOBAL_NAMESPACE_IMPORTS 1
#endif

#include "factory/Traits.h"

#if LW_FACTORY_ENABLE_STATIC_BUS
#include "factory/MakeBus.h"
#include "factory/MakeCompositeBus.h"
#endif

#if LW_FACTORY_ENABLE_STATIC
#include "factory/MakeShader.h"
#endif

#if LW_FACTORY_ENABLE_DYNAMIC_FACTORY_API
#include "factory/MakeDynamicBus.h"
#endif

#if LW_FACTORY_ENABLE_DYNAMIC_BUILDER_API
#include "factory/DynamicBusBuilder.h"
#endif

#if LW_FACTORY_ENABLE_DYNAMIC_INI_API
#include "factory/BuildDynamicBusBuilderFromIni.h"
#include "factory/IniReader.h"
#endif

#if LW_MAIN_HEADER_ENABLE_GLOBAL_NAMESPACE_IMPORTS

#if LW_FACTORY_ENABLE_STATIC_BUS || LW_FACTORY_ENABLE_DYNAMIC_FACTORY_API
using lw::factory::makeBus;
using lw::factory::tryMakeBus;
#endif

#if LW_FACTORY_ENABLE_STATIC
using lw::factory::makeShader;
using lw::factory::Shader;
using lw::factory::Gamma;
using lw::factory::CurrentLimiter;
using lw::factory::WhiteBalance;
using lw::factory::NoShader;
using lw::factory::PlatformDefaultOptions;
using lw::factory::DotStarOptions;
using lw::factory::DebugOptions;
using lw::factory::NoneOptions;
using lw::factory::Ws2812xOptions;
using lw::factory::NilOptions;
using lw::factory::NeoPrintOptions;
using lw::factory::GammaOptions;

#if LW_FACTORY_ENABLE_STATIC_BUS
using lw::factory::getFactory;
using lw::factory::Bus;
using lw::factory::CompositeBus;

using lw::factory::descriptors::DotStar;
using lw::factory::descriptors::Debug;
using lw::factory::descriptors::None;
using lw::factory::descriptors::APA102;
using lw::factory::descriptors::Hd108;
using lw::factory::descriptors::HD108;
using lw::factory::descriptors::Ws2812x;
using lw::factory::descriptors::Ws2812;
using lw::factory::descriptors::Ws2811;
using lw::factory::descriptors::Ws2805;
using lw::factory::descriptors::Sk6812;
using lw::factory::descriptors::Apa106;
using lw::factory::descriptors::Tx1812;
using lw::factory::descriptors::Gs1903;
using lw::factory::descriptors::Generic800;
using lw::factory::descriptors::Generic400;
using lw::factory::descriptors::Ws2816;
using lw::factory::descriptors::Ws2813;
using lw::factory::descriptors::Ws2814;
using lw::factory::descriptors::Lc8812;
using lw::factory::descriptors::Ucs8903;
using lw::factory::descriptors::Ucs8904;
using lw::factory::descriptors::NeoPrint;
using lw::factory::descriptors::Nil;
using lw::factory::descriptors::PlatformDefault;

#if LW_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS
using lw::factory::descriptors::NeoSpi;
#endif

#if defined(ARDUINO_ARCH_RP2040)
using lw::factory::descriptors::RpPio;
using lw::factory::descriptors::RpSpi;
using lw::factory::descriptors::RpUart;
using lw::factory::RpPioOptions;
using lw::factory::RpSpiOptions;
using lw::factory::RpUartOptions;
#endif

#if defined(ARDUINO_ARCH_ESP32)

using lw::factory::descriptors::Esp32Rmt;
using lw::factory::descriptors::Esp32I2s;
using lw::factory::descriptors::Esp32DmaSpi;
using lw::factory::Esp32RmtOptions;
using lw::factory::Esp32I2sOptions;
using lw::factory::Esp32DmaSpiOptions;
#endif

#if defined(ARDUINO_ARCH_ESP8266)

using lw::factory::descriptors::Esp8266DmaI2s;
using lw::factory::descriptors::Esp8266DmaUart;
using lw::factory::Esp8266DmaI2sOptions;
using lw::factory::Esp8266DmaUartOptions;
#endif

#endif

#endif

#if LW_FACTORY_ENABLE_DYNAMIC_FACTORY_API
using lw::factory::makeDynamicBus;
using lw::factory::tryMakeDynamicBus;
using lw::factory::makeDynamicAggregateBus;
using lw::factory::tryMakeDynamicAggregateBus;
#endif

#if LW_FACTORY_ENABLE_DYNAMIC_BUILDER_API
using lw::factory::DynamicBusBuilder;
using lw::factory::DynamicBusBuilderError;
using lw::factory::DynamicBusBuilderResult;
#endif

#if LW_FACTORY_ENABLE_DYNAMIC_INI_API
using lw::factory::DynamicBusBuilderIniError;
using lw::factory::buildDynamicBusBuilderFromIni;
using lw::factory::tryBuildDynamicBusBuilderFromIni;
using lw::factory::IniReader;
using lw::factory::IniSection;
#endif

#endif
