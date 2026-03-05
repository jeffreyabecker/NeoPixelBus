#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "colors/palette/Blends.h"
#include "colors/palette/Traits.h"

namespace lw
{
    template <typename TBlend = BlendLinearContiguous,
              typename TWrap = WrapClamp,
              typename TPaletteLike,
              typename TIndexRange,
              typename TOutputRange,
              typename = std::enable_if_t<IsPaletteLike<TPaletteLike>::value &&
                                          IsBeginEndRange<std::remove_reference_t<TIndexRange>>::value &&
                                          IsBeginEndRange<std::remove_reference_t<TOutputRange>>::value>>
    constexpr size_t samplePalette(const TPaletteLike &palette,
                                   TIndexRange &&paletteIndexes,
                                   TOutputRange &&outputColors,
                                   PaletteSampleOptions<typename TPaletteLike::StopType::ColorType> options = {})
    {
        return TBlend::template samplePalette<TWrap>(palette,
                                                     std::forward<TIndexRange>(paletteIndexes),
                                                     std::forward<TOutputRange>(outputColors),
                                                     options);
    }

} // namespace lw
