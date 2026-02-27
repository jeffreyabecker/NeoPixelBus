#pragma once

#include "colors/Color.h"

namespace npb
{
namespace factory
{
namespace descriptors
{

    struct APA102
    {
    };

    template <typename TColor = npb::Rgb8Color>
    struct DotStar
    {
    };

    template <typename TColor = npb::Rgb8Color>
    struct Ws2812x
    {
    };

} // namespace descriptors
} // namespace factory
} // namespace npb
