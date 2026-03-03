#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "core/Compat.h"
#include "colors/Color.h"
#include "colors/ColorMath.h"

namespace lw
{
    enum class PaletteBlendMode : uint8_t
    {
        Linear,
        Nearest
    };

    enum class PaletteWrapMode : uint8_t
    {
        Clamp,
        Wrap
    };

    template <typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    struct PaletteSampleOptions
    {
        PaletteBlendMode blendMode{PaletteBlendMode::Linear};
        PaletteWrapMode wrapMode{PaletteWrapMode::Clamp};
        typename TColor::ComponentType brightnessScale{std::numeric_limits<typename TColor::ComponentType>::max()};
    };

    template <typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    struct PaletteStop
    {
        uint8_t index{0};
        TColor color{};
    };

    template <typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    class Palette
    {
    public:
        using StopType = PaletteStop<TColor>;

        constexpr Palette() = default;

        constexpr explicit Palette(span<const StopType> stops)
            : _stops(stops)
        {
        }

        constexpr span<const StopType> stops() const
        {
            return _stops;
        }

        constexpr bool empty() const
        {
            return _stops.empty();
        }

        constexpr size_t size() const
        {
            return _stops.size();
        }

    private:
        span<const StopType> _stops{};
    };

    constexpr uint8_t mapPositionToPaletteIndex(size_t pixelIndex,
                                                size_t pixelCount,
                                                PaletteWrapMode wrapMode = PaletteWrapMode::Clamp)
    {
        if (pixelCount == 0)
        {
            return 0;
        }

        if (wrapMode == PaletteWrapMode::Wrap)
        {
            const size_t wrapped = pixelIndex % pixelCount;
            return static_cast<uint8_t>((wrapped * 256ull) / pixelCount);
        }

        if (pixelCount <= 1)
        {
            return 0;
        }

        const size_t clamped = (pixelIndex >= pixelCount) ? (pixelCount - 1) : pixelIndex;
        return static_cast<uint8_t>((clamped * 255ull) / (pixelCount - 1));
    }

    namespace detail
    {
        constexpr uint16_t absDiffU8(uint8_t left, uint8_t right)
        {
            return (left >= right)
                       ? static_cast<uint16_t>(left - right)
                       : static_cast<uint16_t>(right - left);
        }

        constexpr uint16_t circularDiffU8(uint8_t left, uint8_t right)
        {
            const uint16_t direct = absDiffU8(left, right);
            return std::min<uint16_t>(direct, static_cast<uint16_t>(256 - direct));
        }

        template <typename TColor,
                  typename = std::enable_if_t<ColorType<TColor>>>
        constexpr TColor applyBrightnessScale(TColor color,
                                              typename TColor::ComponentType brightnessScale)
        {
            using Component = typename TColor::ComponentType;
            constexpr uint32_t MaxComponent = static_cast<uint32_t>(std::numeric_limits<Component>::max());
            const uint32_t scale = static_cast<uint32_t>(brightnessScale);

            if (scale >= MaxComponent)
            {
                return color;
            }

            for (char channel : TColor::channelIndexes())
            {
                const uint32_t value = static_cast<uint32_t>(color[channel]);
                color[channel] = static_cast<Component>((value * scale) / MaxComponent);
            }

            return color;
        }

        template <typename TColor,
                  typename = std::enable_if_t<ColorType<TColor>>>
        constexpr const PaletteStop<TColor> &nearestStop(span<const PaletteStop<TColor>> stops,
                                                         uint8_t index,
                                                         PaletteWrapMode wrapMode)
        {
            size_t nearestIndex = 0;
            uint16_t nearestDistance = std::numeric_limits<uint16_t>::max();

            for (size_t i = 0; i < stops.size(); ++i)
            {
                const uint16_t distance = (wrapMode == PaletteWrapMode::Wrap)
                                              ? circularDiffU8(stops[i].index, index)
                                              : absDiffU8(stops[i].index, index);

                if (distance < nearestDistance)
                {
                    nearestDistance = distance;
                    nearestIndex = i;
                }
            }

            return stops[nearestIndex];
        }

        template <typename TColor,
                  typename = std::enable_if_t<ColorType<TColor>>>
        constexpr TColor sampleLinear(span<const PaletteStop<TColor>> stops,
                                      uint8_t index,
                                      PaletteWrapMode wrapMode)
        {
            if (index <= stops.front().index)
            {
                if (wrapMode == PaletteWrapMode::Clamp)
                {
                    return stops.front().color;
                }
            }

            for (size_t i = 1; i < stops.size(); ++i)
            {
                if (index <= stops[i].index)
                {
                    const auto &left = stops[i - 1];
                    const auto &right = stops[i];
                    const uint16_t spanWidth = static_cast<uint16_t>(right.index - left.index);

                    if (spanWidth == 0)
                    {
                        return right.color;
                    }

                    const uint16_t offset = static_cast<uint16_t>(index - left.index);
                    const uint8_t progress = static_cast<uint8_t>((offset * 255u) / spanWidth);
                    return linearBlend(left.color, right.color, progress);
                }
            }

            if (wrapMode == PaletteWrapMode::Clamp)
            {
                return stops.back().color;
            }

            const auto &left = stops.back();
            const auto &right = stops.front();

            const uint16_t leftIndex = left.index;
            const uint16_t rightIndex = static_cast<uint16_t>(right.index) + 256u;
            const uint16_t sampleIndex = (index >= left.index)
                                             ? static_cast<uint16_t>(index)
                                             : static_cast<uint16_t>(index) + 256u;

            const uint16_t spanWidth = static_cast<uint16_t>(rightIndex - leftIndex);
            if (spanWidth == 0)
            {
                return left.color;
            }

            const uint16_t offset = static_cast<uint16_t>(sampleIndex - leftIndex);
            const uint8_t progress = static_cast<uint8_t>((offset * 255u) / spanWidth);
            return linearBlend(left.color, right.color, progress);
        }
    } // namespace detail

    template <typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr TColor samplePalette(const Palette<TColor> &palette,
                                   uint8_t index,
                                   PaletteSampleOptions<TColor> options = {})
    {
        const auto stops = palette.stops();
        if (stops.empty())
        {
            return TColor{};
        }

        TColor sampled{};

        if (stops.size() == 1)
        {
            sampled = stops.front().color;
        }
        else if (options.blendMode == PaletteBlendMode::Nearest)
        {
            sampled = detail::nearestStop(stops, index, options.wrapMode).color;
        }
        else
        {
            sampled = detail::sampleLinear(stops, index, options.wrapMode);
        }

        return detail::applyBrightnessScale(sampled, options.brightnessScale);
    }
} // namespace lw
