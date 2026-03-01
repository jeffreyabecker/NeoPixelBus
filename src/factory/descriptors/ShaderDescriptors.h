#pragma once

#include "colors/Color.h"

namespace lw
{
namespace factory
{
namespace descriptors
{

    template <typename TColor = lw::Rgb8Color>
    struct Gamma
    {
        using ColorType = TColor;
    };

    template <typename TColor = lw::Rgb8Color>
    struct CurrentLimiter
    {
        using ColorType = TColor;
    };

    template <typename TColor = lw::Rgbw8Color>
    struct WhiteBalance
    {
        using ColorType = TColor;
    };

    template <typename TColor = lw::Rgb8Color>
    struct NilShader
    {
        using ColorType = TColor;
    };

} // namespace descriptors
} // namespace factory
} // namespace lw
