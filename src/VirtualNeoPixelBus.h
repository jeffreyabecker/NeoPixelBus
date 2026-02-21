#pragma once

// Phase 1 — Color, Transform, Emitter, Bus
#include "virtual/internal/colors/Color.h"
#include "virtual/internal/transforms/ITransformColorToBytes.h"
#include "virtual/internal/transforms/NeoPixelTransform.h"
#include "virtual/internal/emitters/IEmitPixels.h"
#include "virtual/internal/emitters/PrintEmitter.h"
#include "virtual/internal/IPixelBus.h"
#include "virtual/internal/PixelBus.h"

// Phase 2 — Shaders, Gamma Methods, ShadedTransform
#include "virtual/internal/transforms/IShader.h"
#include "virtual/internal/transforms/GammaNullMethod.h"
#include "virtual/internal/transforms/GammaEquationMethod.h"
#include "virtual/internal/transforms/GammaCieLabMethod.h"
#include "virtual/internal/transforms/GammaTableMethod.h"
#include "virtual/internal/transforms/GammaDynamicTableMethod.h"
#include "virtual/internal/transforms/GammaInvertMethod.h"
#include "virtual/internal/transforms/GammaShader.h"
#include "virtual/internal/transforms/CurrentLimiterShader.h"
#include "virtual/internal/transforms/ShadedTransform.h"
