#pragma once
#include "colors/Color.h"
#include "colors/palette/defaults/DefaultPalettes.h"

#ifndef LW_USE_EXPLICIT_NAMESPACES
inline const auto DefaultPalettes = lw::palettes::StaticPalettes<lw::colors::DefaultColorType>();
namespace Gradients = lw::palettes;

#endif // LW_USE_EXPLICIT_NAMESPACES