#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>

#include "Color.h"
#include "IShader.h"

namespace npb
{

    template<typename TColor>
    class CurrentLimiterShader : public IShader<TColor>
    {
    public:
        static constexpr uint16_t DefaultControllerMilliamps = 100;
        static constexpr uint16_t DefaultStandbyMilliampsPerPixel = 1;

        // maxMilliamps: total power budget including controller + standby current.
        // milliampsPerChannel: current draw per channel at full component value.
        //   e.g. {20, 20, 20, 0, 0} for RGB-only at 20 mA each.
        // controllerMilliamps: fixed draw from the MCU/controller.
        // standbyMilliampsPerPixel: fixed per-pixel idle current.
        // rgbwDerating: WLED-style derating for RGBW strips (approx. 3/4 of naive sum).
        CurrentLimiterShader(uint32_t maxMilliamps,
                             std::array<uint16_t, TColor::ChannelCount> milliampsPerChannel,
                             uint16_t controllerMilliamps = DefaultControllerMilliamps,
                             uint16_t standbyMilliampsPerPixel = DefaultStandbyMilliampsPerPixel,
                             bool rgbwDerating = true)
            : _maxMilliamps{maxMilliamps},
              _controllerMilliamps{controllerMilliamps},
              _standbyMilliampsPerPixel{standbyMilliampsPerPixel},
              _rgbwDerating{rgbwDerating},
              _milliampsPerChannel{milliampsPerChannel}
        {
        }

        void apply(std::span<TColor> colors) override
        {
            if (_maxMilliamps == 0)
            {
                _lastEstimatedMilliamps = 0;
                return;
            }

            const uint64_t maxComponent = static_cast<uint64_t>(TColor::MaxComponent);
            if (maxComponent == 0)
            {
                _lastEstimatedMilliamps = 0;
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
        uint64_t estimateWeightedDraw(std::span<const TColor> colors) const
        {
            uint64_t totalDrawWeighted = 0;
            for (const auto &color : colors)
            {
                uint64_t pixelDrawWeighted = 0;
                for (size_t ch = 0; ch < TColor::ChannelCount; ++ch)
                {
                    pixelDrawWeighted += static_cast<uint64_t>(color[ch]) * _milliampsPerChannel[ch];
                }

                if (_rgbwDerating && (TColor::ChannelCount >= 4))
                {
                    pixelDrawWeighted = (pixelDrawWeighted * 3ULL) / 4ULL;
                }

                totalDrawWeighted += pixelDrawWeighted;
            }

            return totalDrawWeighted;
        }

        static void scaleAll(std::span<TColor> colors, uint32_t scale)
        {
            for (auto &color : colors)
            {
                for (size_t ch = 0; ch < TColor::ChannelCount; ++ch)
                {
                    const uint64_t scaled = (static_cast<uint64_t>(color[ch]) * scale + 127ULL) / 255ULL;
                    color[ch] = static_cast<typename TColor::ComponentType>(scaled);
                }
            }
        }
        uint32_t _maxMilliamps;
        uint16_t _controllerMilliamps;
        uint16_t _standbyMilliampsPerPixel;
        bool _rgbwDerating;
        std::array<uint16_t, TColor::ChannelCount> _milliampsPerChannel;
        uint32_t _lastEstimatedMilliamps{0};
    };

} // namespace npb
