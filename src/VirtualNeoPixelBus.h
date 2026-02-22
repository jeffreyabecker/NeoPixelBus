#pragma once

// Colors
#include "virtual/internal/colors/Color.h"

// Emitters (includes internal transform details)
#include "virtual/internal/emitters/IEmitPixels.h"
#include "virtual/internal/emitters/PrintEmitter.h"
#include "virtual/internal/emitters/ClockDataEmitter.h"

// Shaders
#include "virtual/internal/shaders/IShader.h"
#include "virtual/internal/shaders/GammaNullMethod.h"
#include "virtual/internal/shaders/GammaEquationMethod.h"
#include "virtual/internal/shaders/GammaCieLabMethod.h"
#include "virtual/internal/shaders/GammaTableMethod.h"
#include "virtual/internal/shaders/GammaDynamicTableMethod.h"
#include "virtual/internal/shaders/GammaInvertMethod.h"
#include "virtual/internal/shaders/GammaShader.h"
#include "virtual/internal/shaders/CurrentLimiterShader.h"
#include "virtual/internal/shaders/ShadedTransform.h"

// Buses
#include "virtual/internal/buses/IClockDataBus.h"
#include "virtual/internal/buses/ClockDataProtocol.h"
#include "virtual/internal/buses/DebugClockDataBus.h"
// SpiClockDataBus.h excluded — include directly when SPI is available
#include "virtual/internal/buses/BitBangClockDataBus.h"

// Bus
#include "virtual/internal/IPixelBus.h"
#include "virtual/internal/PixelBus.h"


