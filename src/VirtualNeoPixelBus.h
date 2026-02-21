#pragma once

// Phase 1 — Color, Transform, Emitter, Bus
#include "virtual/internal/colors/Color.h"
#include "virtual/internal/transforms/ITransformColorToBytes.h"
#include "virtual/internal/transforms/ColorOrderTransform.h"
#include "virtual/internal/emitters/IEmitPixels.h"
#include "virtual/internal/emitters/PrintEmitter.h"
#include "virtual/internal/IPixelBus.h"
#include "virtual/internal/PixelBus.h"

// Phase 2 — Shaders, Gamma Methods, ShadedTransform
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

// Phase 3 — Two-Wire Buses, ClockDataEmitter, DotStarTransform
#include "virtual/internal/buses/IClockDataBus.h"
#include "virtual/internal/buses/ClockDataProtocol.h"
#include "virtual/internal/buses/DebugClockDataBus.h"
// SpiClockDataBus.h excluded — include directly when SPI is available
#include "virtual/internal/buses/BitBangClockDataBus.h"
#include "virtual/internal/emitters/ClockDataEmitter.h"
#include "virtual/internal/transforms/DotStarTransform.h"


