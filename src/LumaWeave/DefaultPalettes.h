#pragma once
#include "colors/Color.h"
#include "colors/palette/defaults/DefaultPalettes.h"

#ifndef LW_USE_EXPLICIT_NAMESPACES
static constexpr auto& DefaultPalettes = lw::palettes::StaticPalettes<lw::colors::DefaultColorType>::instance();
using Gradients = lw::palettes::gradients;

#endif // LW_USE_EXPLICIT_NAMESPACES