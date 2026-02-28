#pragma once

#include <cstdint>
#include <cstddef>
#include <array>

#include "Color.h"
#include "IShader.h"

namespace npb
{

    struct CurrentLimiterChannelMilliamps
    {
        uint16_t R = 0;
        uint16_t G = 0;
        uint16_t B = 0;
        uint16_t W = 0;
        uint16_t C = 0;
    };

    template<typename TColor>
    struct CurrentLimiterShaderSettings
    {
        static constexpr uint16_t DefaultControllerMilliamps = 100;
        static constexpr uint16_t DefaultStandbyMilliampsPerPixel = 1;

        uint32_t maxMilliamps = 0;
        CurrentLimiterChannelMilliamps milliampsPerChannel{};
        uint16_t controllerMilliamps = DefaultControllerMilliamps;
        uint16_t standbyMilliampsPerPixel = DefaultStandbyMilliampsPerPixel;
        bool rgbwDerating = true;
    };

    template<typename TColor>
    class CurrentLimiterShader : public IShader<TColor>
    {
    public:
        using ColorType = TColor;
        using SettingsType = CurrentLimiterShaderSettings<TColor>;

        static constexpr uint16_t DefaultControllerMilliamps = SettingsType::DefaultControllerMilliamps;
        static constexpr uint16_t DefaultStandbyMilliampsPerPixel = SettingsType::DefaultStandbyMilliampsPerPixel;

        explicit CurrentLimiterShader(SettingsType settings)
            : _maxMilliamps{settings.maxMilliamps}
            , _controllerMilliamps{settings.controllerMilliamps}
            , _standbyMilliampsPerPixel{settings.standbyMilliampsPerPixel}
            , _rgbwDerating{settings.rgbwDerating}
            , _milliampsPerChannel{settings.milliampsPerChannel}
        {
        }

        void apply(span<TColor> colors) override
        {
            if (_maxMilliamps == 0)
            {
                _lastEstimatedMilliamps = 0;
                return;
            }

            const uint64_t maxComponent = static_cast<uint64_t>(TColor::MaxComponent);
            if (maxComponent == 0)
            {
                return;
            }

            uint64_t weightedDraw = estimateWeightedDraw(colors);

            uint64_t estimatedMilliamps = (weightedDraw / maxComponent)
                + _controllerMilliamps
                + static_cast<uint64_t>(_standbyMilliampsPerPixel) * colors.size();

            _lastEstimatedMilliamps = static_cast<uint32_t>(estimatedMilliamps);

            if (_maxMilliamps <= _controllerMilliamps)
            {
                scaleAll(colors, 0);
                _lastEstimatedMilliamps = _controllerMilliamps
                    + static_cast<uint32_t>(_standbyMilliampsPerPixel) * static_cast<uint32_t>(colors.size());
                return;
            }

            uint64_t budgetForPixels = _maxMilliamps - _controllerMilliamps;
            const uint64_t standbyDraw = static_cast<uint64_t>(_standbyMilliampsPerPixel) * colors.size();
            if (budgetForPixels > standbyDraw)
            {
                budgetForPixels -= standbyDraw;
            }
            else
            {
                budgetForPixels = 0;
            }

            const uint64_t pixelMilliamps = weightedDraw / maxComponent;
            if (pixelMilliamps <= budgetForPixels)
            {
                return;
            }

            uint32_t scale = 0;
            if (pixelMilliamps > 0)
            {
                scale = static_cast<uint32_t>((budgetForPixels * 255ULL) / pixelMilliamps);
                if (scale > 255)
                {
                    scale = 255;
                }
            }

            scaleAll(colors, scale);

            const uint64_t limitedPixelMilliamps = (pixelMilliamps * scale) / 255ULL;
            _lastEstimatedMilliamps = static_cast<uint32_t>(limitedPixelMilliamps + _controllerMilliamps + standbyDraw);
        }

        uint32_t lastEstimatedMilliamps() const
        {
            return _lastEstimatedMilliamps;
        }

    private:
        static uint16_t milliampsForChannel(const CurrentLimiterChannelMilliamps &current,
                                            size_t channelIndex)
        {
            switch (channelIndex)
            {
            case 0:
                return current.R;
            case 1:
                return current.G;
            case 2:
                return current.B;
            case 3:
                return current.W;
            case 4:
                return current.C;
            default:
                return 0;
            }
        }

        uint64_t estimateWeightedDraw(span<const TColor> colors) const
        {
            uint64_t totalDrawWeighted = 0;
            for (const auto &color : colors)
            {
                uint64_t pixelDrawWeighted = 0;
                size_t ch = 0;
                for (auto component = color.cbegin(); component != color.cend(); ++component, ++ch)
                {
                    pixelDrawWeighted += static_cast<uint64_t>(*component) * milliampsForChannel(_milliampsPerChannel, ch);
                }

                if (_rgbwDerating && (TColor::ChannelCount >= 4))
                {
                    pixelDrawWeighted = (pixelDrawWeighted * 3ULL) / 4ULL;
                }

                totalDrawWeighted += pixelDrawWeighted;
            }

            return totalDrawWeighted;
        }

        static void scaleAll(span<TColor> colors, uint32_t scale)
        {
            for (auto &color : colors)
            {
                for (auto &component : color)
                {
                    const uint64_t scaled = (static_cast<uint64_t>(component) * scale + 127ULL) / 255ULL;
                    component = static_cast<typename TColor::ComponentType>(scaled);
                }
            }
        }

        uint32_t _maxMilliamps;
        uint16_t _controllerMilliamps;
        uint16_t _standbyMilliampsPerPixel;
        bool _rgbwDerating;
        CurrentLimiterChannelMilliamps _milliampsPerChannel;
        uint32_t _lastEstimatedMilliamps{0};
    };

} // namespace npb

