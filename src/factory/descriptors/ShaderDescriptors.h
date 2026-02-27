#pragma once

#include "colors/Color.h"

namespace npb
{
namespace factory
{
namespace descriptors
{

    template <typename TColor = npb::Rgb8Color>
    struct Gamma
    {
        using ColorType = TColor;
    };

    template <typename TColor = npb::Rgb8Color>
    struct CurrentLimiter
    {
        using ColorType = TColor;
    };

    template <typename TColor = npb::Rgbw8Color>
    struct WhiteBalance
    {
        using ColorType = TColor;
    };

    template <typename TColor = npb::Rgb8Color>
    struct NilShader
    {
        using ColorType = TColor;
    };

} // namespace descriptors
} // namespace factory
} // namespace npb
