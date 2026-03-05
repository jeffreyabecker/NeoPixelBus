#pragma once

#ifndef LW_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS
#define LW_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS 0
#endif

#ifndef LW_MAIN_HEADER_ENABLE_GLOBAL_NAMESPACE_IMPORTS
#define LW_MAIN_HEADER_ENABLE_GLOBAL_NAMESPACE_IMPORTS 1
#endif

#include "buses/MakePixelBus.h"
#include "protocols/ProtocolAliases.h"

#if LW_MAIN_HEADER_ENABLE_GLOBAL_NAMESPACE_IMPORTS

using lw::factory::makePixelBus;
using lw::protocols::Ws2812x;
using lw::protocols::Ws2812xType;
using lw::protocols::DotStar;
using lw::protocols::DotStarType;
using lw::protocols::APA102;
using lw::protocols::APA102Type;
using lw::protocols::Hd108;
using lw::protocols::Hd108Type;
using lw::protocols::HD108;
using lw::protocols::HD108Type;
using lw::protocols::None;
using lw::protocols::NoneType;
using lw::protocols::Debug;
using lw::protocols::DebugType;
using lw::protocols::Tm1814;
using lw::protocols::Tm1814Type;
using lw::protocols::Tm1914;
using lw::protocols::Tm1914Type;
using lw::protocols::Ws2812;
using lw::protocols::Ws2812Type;
using lw::protocols::Ws2811;
using lw::protocols::Ws2811Type;
using lw::protocols::Ws2805;
using lw::protocols::Ws2805Type;
using lw::protocols::Sk6812;
using lw::protocols::Sk6812Type;
using lw::protocols::Tm1829;
using lw::protocols::Tm1829Type;
using lw::protocols::Ws2814;
using lw::protocols::Ws2814Type;

#endif
