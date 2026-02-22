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
#include "virtual/buses/DebugClockDataBus.h"
// SpiClockDataBus.h excluded — include directly when SPI is available

// Bus
#include "virtual/IPixelBus.h"
#include "virtual/PixelBus.h"


