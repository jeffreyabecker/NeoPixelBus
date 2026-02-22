#pragma once

// Colors
#include "virtual/colors/Color.h"

// Emitters (includes internal transform details)
#include "virtual/emitters/IEmitPixels.h"
#include "virtual/emitters/PrintEmitter.h"
#include "virtual/emitters/ClockDataEmitter.h"

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
#include "virtual/buses/IClockDataBus.h"
#include "virtual/buses/ClockDataProtocol.h"
#include "virtual/buses/DebugClockDataBus.h"
// SpiClockDataBus.h excluded — include directly when SPI is available
#include "virtual/buses/BitBangClockDataBus.h"

// Bus
#include "virtual/IPixelBus.h"
#include "virtual/PixelBus.h"


