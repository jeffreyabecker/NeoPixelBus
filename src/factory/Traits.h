#pragma once

#include "factory/traits/ProtocolDescriptorTraits.h"
#include "factory/traits/ProtocolDescriptorTraits.DotStar.h"
#include "factory/traits/ProtocolDescriptorTraits.Ws2812x.h"
#include "factory/traits/ProtocolDescriptorTraits.Tm1814.h"
#include "factory/traits/ProtocolDescriptorTraits.Tm1914.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "factory/traits/TransportDescriptorTraits.PrintTransport.h"
#include "factory/traits/TransportDescriptorTraits.NilTransport.h"
#include "factory/traits/ResolveProtocolSettings.h"
#include "factory/traits/ResolveTransportSettings.h"
#include "factory/traits/TransportDescriptorTraits.RpPioTransport.h"
#include "factory/traits/TransportDescriptorTraits.RpSpiTransport.h"
#include "factory/traits/TransportDescriptorTraits.RpUartTransport.h"
#include "factory/traits/TransportDescriptorTraits.Esp32RmtOneWireTransport.h"
#include "factory/traits/TransportDescriptorTraits.Esp32I2sTransport.h"
#include "factory/traits/TransportDescriptorTraits.Esp32DmaSpiTransport.h"
#include "factory/traits/TransportDescriptorTraits.Esp8266DmaI2sTransport.h"
#include "factory/traits/TransportDescriptorTraits.Esp8266DmaUartTransport.h"
#include "factory/traits/TransportDescriptorTraits.SpiTransport.h"

namespace npb
{
namespace factory
{

#if defined(ARDUINO_ARCH_ESP32)
	using PlatformDefaultOptions = Esp32I2sOptions;
#elif defined(ARDUINO_ARCH_ESP8266)
	using PlatformDefaultOptions = Esp8266DmaI2sOptions;
#elif defined(ARDUINO_ARCH_RP2040)
	using PlatformDefaultOptions = RpPioOptions;
#elif defined(ARDUINO_ARCH_NATIVE) || !defined(ARDUINO)
	using PlatformDefaultOptions = NilOptions;
#else
#if defined(NPB_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS)
	using PlatformDefaultOptions = NeoSpiOptions;
#else
	using PlatformDefaultOptions = NilOptions;
#endif
#endif

} // namespace factory
} // namespace npb
