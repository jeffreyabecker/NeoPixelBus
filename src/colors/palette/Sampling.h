#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "colors/palette/Blends.h"

namespace lw
{
    template <typename TWrap = WrapClamp>
    constexpr uint8_t mapPositionToPaletteIndex(size_t pixelIndex,
                                                size_t pixelCount)
    {
        return TWrap::mapPositionToPaletteIndex(pixelIndex, pixelCount);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TColor,
              typename TIndexIt,
              typename TIndexSentinel,
              typename TOutputIt,
              typename TSentinel,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                   TIndexIt index,
                                   TIndexSentinel indexEnd,
                                   TOutputIt output,
                                   TSentinel outputEnd,
                                   PaletteSampleOptions<TColor> options = {})
    {
        return TBlend::template samplePalette<TColor>(stops,
                                                      index,
                                                      indexEnd,
                                                      output,
                                                      outputEnd,
                                                      options);
    }

    template <typename TBlend = BlendLinearContiguous<>,
              typename TColor,
              typename TIndexIt,
              typename TIndexSentinel,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr size_t samplePalette(span<const PaletteStop<TColor>> stops,
                                   TIndexIt index,
                                   TIndexSentinel indexEnd,
                                   span<TColor> outputColors,
                                   PaletteSampleOptions<TColor> options = {})
    {
        return TBlend::template samplePalette<TColor>(stops,
                                                      index,
                                                      indexEnd,
                                                      outputColors,
                                                      options);
    }

} // namespace lw
