#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "colors/ColorMath.h"
#include "colors/HsbColor.h"
#include "colors/palette/detail/RandomBackend.h"
#include "colors/palette/Types.h"

namespace lw
{
    namespace detail::palettegen
    {
        struct RandomBackendSelector
        {
            using Type = LW_PALETTE_RANDOM_BACKEND;
        };

        constexpr uint32_t nextRandom(uint32_t &state)
        {
            using Backend = typename RandomBackendSelector::Type;
            return Backend::next(state);
        }

        template <typename TComponent>
        constexpr TComponent randomComponent(uint32_t &state)
        {
            const uint32_t value = nextRandom(state);
            return static_cast<TComponent>(value & static_cast<uint32_t>(std::numeric_limits<TComponent>::max()));
        }

        template <typename TColor,
                  typename = std::enable_if_t<ColorType<TColor>>>
        constexpr TColor randomColor(uint32_t &state)
        {
            TColor color{};
            for (char channel : TColor::channelIndexes())
            {
                color[channel] = randomComponent<typename TColor::ComponentType>(state);
            }

            return color;
        }

        template <typename TColor,
                  size_t TStopCount,
                  typename = std::enable_if_t<ColorType<TColor>>>
        constexpr void assignEvenStopIndexes(std::array<PaletteStop<TColor>, TStopCount> &stops)
        {
            static_assert(TStopCount >= 2, "Palette generators require at least 2 stops");

            for (size_t i = 0; i < TStopCount; ++i)
            {
                stops[i].index = i;
            }
        }
    } // namespace detail::palettegen

    template <typename TColor,
              size_t TStopCount = 16,
              RequireColorChannelsInRange<TColor, 3, 5> = 0>
    class SolidPaletteGenerator
    {
    public:
        using StopType = PaletteStop<TColor>;

        explicit constexpr SolidPaletteGenerator(TColor color = TColor{})
            : _color(color)
        {
            detail::palettegen::assignEvenStopIndexes(_stops);
            rebuild();
        }

        constexpr void setColor(const TColor &color)
        {
            _color = color;
            rebuild();
        }

        constexpr void update()
        {
            rebuild();
        }

        constexpr span<const StopType> stops() const
        {
            return span<const StopType>(_stops.data(), _stops.size());
        }

    private:
        constexpr void rebuild()
        {
            for (size_t i = 0; i < TStopCount; ++i)
            {
                _stops[i].color = _color;
            }
        }

        std::array<StopType, TStopCount> _stops{};
        TColor _color{};
    };

    template <typename TColor,
              size_t TStopCount = 16,
              RequireColorChannelsInRange<TColor, 3, 5> = 0>
    class RainbowPaletteGenerator
    {
    public:
        using StopType = PaletteStop<TColor>;

        constexpr RainbowPaletteGenerator(float saturation = 1.0f,
                                          float brightness = 1.0f,
                                          uint8_t hueOffset = 0)
            : _saturation(saturation),
              _brightness(brightness),
              _hueOffset(hueOffset)
        {
            detail::palettegen::assignEvenStopIndexes(_stops);
            rebuild();
        }

        constexpr void setSaturation(float saturation)
        {
            _saturation = saturation;
            rebuild();
        }

        constexpr void setBrightness(float brightness)
        {
            _brightness = brightness;
            rebuild();
        }

        constexpr void setHueOffset(uint8_t hueOffset)
        {
            _hueOffset = hueOffset;
            rebuild();
        }

        constexpr void update(uint8_t hueStep = 1)
        {
            _hueOffset = static_cast<uint8_t>(_hueOffset + hueStep);
            rebuild();
        }

        constexpr span<const StopType> stops() const
        {
            return span<const StopType>(_stops.data(), _stops.size());
        }

    private:
        constexpr void rebuild()
        {
            for (size_t i = 0; i < TStopCount; ++i)
            {
                const uint8_t hue = static_cast<uint8_t>(_hueOffset + static_cast<uint8_t>((i * 256ull) / TStopCount));
                const float hue01 = static_cast<float>(hue) / 255.0f;
                _stops[i].color = toRgb<TColor>(HsbColor(hue01, _saturation, _brightness));
            }
        }

        std::array<StopType, TStopCount> _stops{};
        float _saturation{1.0f};
        float _brightness{1.0f};
        uint8_t _hueOffset{0};
    };

    template <typename TColor,
              size_t TStopCount = 8,
              typename = std::enable_if_t<ColorType<TColor>>>
    class RandomSmoothPaletteGenerator
    {
    public:
        using StopType = PaletteStop<TColor>;

        explicit constexpr RandomSmoothPaletteGenerator(uint32_t seed = 0xC0FFEE11u,
                                                        uint8_t progressStep = 12)
            : _rngState(seed),
              _progressStep(progressStep)
        {
            detail::palettegen::assignEvenStopIndexes(_stops);

            for (size_t i = 0; i < TStopCount; ++i)
            {
                _sourceColors[i] = detail::palettegen::randomColor<TColor>(_rngState);
                _targetColors[i] = detail::palettegen::randomColor<TColor>(_rngState);
            }

            rebuild();
        }

        constexpr void setSeed(uint32_t seed)
        {
            _rngState = seed;
        }

        constexpr void update(uint8_t progressStep = 0)
        {
            const uint8_t step = (progressStep == 0) ? _progressStep : progressStep;
            uint16_t nextProgress = static_cast<uint16_t>(_progress) + step;

            while (nextProgress >= 255u)
            {
                nextProgress = static_cast<uint16_t>(nextProgress - 255u);
                for (size_t i = 0; i < TStopCount; ++i)
                {
                    _sourceColors[i] = _targetColors[i];
                    _targetColors[i] = detail::palettegen::randomColor<TColor>(_rngState);
                }
            }

            _progress = static_cast<uint8_t>(nextProgress);
            rebuild();
        }

        constexpr span<const StopType> stops() const
        {
            return span<const StopType>(_stops.data(), _stops.size());
        }

    private:
        constexpr void rebuild()
        {
            for (size_t i = 0; i < TStopCount; ++i)
            {
                _stops[i].color = linearBlend(_sourceColors[i], _targetColors[i], _progress);
            }
        }

        std::array<StopType, TStopCount> _stops{};
        std::array<TColor, TStopCount> _sourceColors{};
        std::array<TColor, TStopCount> _targetColors{};
        uint32_t _rngState{0xC0FFEE11u};
        uint8_t _progress{0};
        uint8_t _progressStep{12};
    };

    template <typename TColor,
              size_t TStopCount = 8,
              typename = std::enable_if_t<ColorType<TColor>>>
    class RandomCyclePaletteGenerator
    {
    public:
        using StopType = PaletteStop<TColor>;

        explicit constexpr RandomCyclePaletteGenerator(uint32_t seed = 0x13579BDFu,
                                                       uint8_t cycleStep = 8)
            : _rngState(seed),
              _cycleStep(cycleStep)
        {
            detail::palettegen::assignEvenStopIndexes(_stops);
            for (size_t i = 0; i < TStopCount; ++i)
            {
                _colors[i] = detail::palettegen::randomColor<TColor>(_rngState);
            }

            rebuild();
        }

        constexpr void setSeed(uint32_t seed)
        {
            _rngState = seed;
        }

        constexpr void update(uint8_t cycleStep = 0)
        {
            const uint8_t step = (cycleStep == 0) ? _cycleStep : cycleStep;
            uint16_t nextPhase = static_cast<uint16_t>(_phase) + step;

            while (nextPhase >= 255u)
            {
                nextPhase = static_cast<uint16_t>(nextPhase - 255u);
                rotateCycle();
            }

            _phase = static_cast<uint8_t>(nextPhase);
            rebuild();
        }

        constexpr span<const StopType> stops() const
        {
            return span<const StopType>(_stops.data(), _stops.size());
        }

    private:
        constexpr void rotateCycle()
        {
            for (size_t i = 0; i + 1 < TStopCount; ++i)
            {
                _colors[i] = _colors[i + 1];
            }

            _colors[TStopCount - 1] = detail::palettegen::randomColor<TColor>(_rngState);
        }

        constexpr void rebuild()
        {
            for (size_t i = 0; i < TStopCount; ++i)
            {
                const size_t next = (i + 1u) % TStopCount;
                _stops[i].color = linearBlend(_colors[i], _colors[next], _phase);
            }
        }

        std::array<StopType, TStopCount> _stops{};
        std::array<TColor, TStopCount> _colors{};
        uint32_t _rngState{0x13579BDFu};
        uint8_t _phase{0};
        uint8_t _cycleStep{8};
    };

} // namespace lw
