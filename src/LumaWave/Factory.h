#pragma once

#ifndef LW_ENABLE_LEGACY_FACTORY
#define LW_ENABLE_LEGACY_FACTORY 1
#endif

#if !defined(LW_FACTORY_SYSTEM_DISABLED) && LW_ENABLE_LEGACY_FACTORY
#ifndef LW_LEGACY_FACTORY_NO_DEPRECATION_WARNING
#if defined(_MSC_VER)
#pragma message("LumaWave legacy factory surface is deprecated and scheduled for removal. Define LW_ENABLE_LEGACY_FACTORY=0 to proactively disable it.")
#elif defined(__GNUC__) || defined(__clang__)
#warning "LumaWave legacy factory surface is deprecated and scheduled for removal. Define LW_ENABLE_LEGACY_FACTORY=0 to proactively disable it."
#endif
#endif

#include "factory/Factory.h"
#endif
