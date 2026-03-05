#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "colors/ColorMath.h"
#include "colors/palette/Blends.h"
#include "colors/palette/Sampling.h"

namespace lw
{
    constexpr uint8_t mapTransitionProgressToBlend(size_t transitionProgress,
                                                   size_t transitionDuration)
    {
        if (transitionDuration == 0)
        {
            return 255;
        }

        const size_t clamped = (transitionProgress >= transitionDuration)
                                   ? transitionDuration
                                   : transitionProgress;
        return static_cast<uint8_t>((clamped * 255ull) / transitionDuration);
    }

    template <typename TBlend = BlendLinearContiguous,
              typename TWrap = WrapClamp,
              typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr TColor samplePaletteTransition(span<const PaletteStop<TColor>> fromStops,
                                             span<const PaletteStop<TColor>> toStops,
                                             size_t paletteIndex,
                                             size_t transitionProgress,
                                             size_t transitionDuration,
                                             PaletteSampleOptions<TColor> options = {})
    {
        const Palette<TColor> fromPalette(fromStops);
        const Palette<TColor> toPalette(toStops);
        IndexRange paletteIndexes(paletteIndex, 1, 1);
        std::array<TColor, 1> fromSampled{};
        std::array<TColor, 1> toSampled{};

        samplePalette<TBlend, TWrap>(fromPalette,
                                     paletteIndexes,
                                     fromSampled,
                                     options);
        samplePalette<TBlend, TWrap>(toPalette,
                                     paletteIndexes,
                                     toSampled,
                                     options);

        const TColor fromColor = fromSampled[0];
        const TColor toColor = toSampled[0];
        const uint8_t blendProgress = mapTransitionProgressToBlend(transitionProgress,
                                                                   transitionDuration);
        return linearBlend(fromColor,
                           toColor,
                           blendProgress);
    }

} // namespace lw
