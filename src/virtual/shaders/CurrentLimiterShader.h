#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>

#include "../colors/Color.h"
#include "IShader.h"

namespace npb
{

    class CurrentLimiterShader : public IShader
    {
    public:
        // maxMilliamps: total power budget for the strip
        // milliampsPerChannel: current draw per channel at full brightness
        //   e.g. {20, 20, 20, 0, 0} for RGB-only at 20 mA each
        //   Channels with 0 mA are excluded from the current estimate
        //   but still scaled proportionally when over budget.
        CurrentLimiterShader(uint32_t maxMilliamps,
                             std::array<uint16_t, Color::ChannelCount> milliampsPerChannel)
            : _maxMilliamps{maxMilliamps}, _milliampsPerChannel{milliampsPerChannel}
        {
        }

        void apply(std::span<Color> colors) override
        {
            // Pass 1: estimate total current draw across all pixels
            // using per-channel milliamp ratings
            uint32_t totalDrawWeighted = 0;
            for (const auto& color : colors)
            {
                for (size_t ch = 0; ch < Color::ChannelCount; ++ch)
                {
                    totalDrawWeighted += static_cast<uint32_t>(color[ch]) * _milliampsPerChannel[ch];
                }
            }

            // totalDrawWeighted is in units of (value * mA).
            // Actual milliamps = totalDrawWeighted / 255
            uint32_t totalMilliamps = totalDrawWeighted / 255;

            if (totalMilliamps <= _maxMilliamps)
            {
                return; // within budget, no scaling needed
            }

            // Pass 2: scale all channels proportionally to fit within budget
            // Using 16-bit fixed point: scale = (max * 256) / total
            uint32_t scale = (_maxMilliamps * 256) / totalMilliamps;

            for (auto& color : colors)
            {
                for (size_t ch = 0; ch < Color::ChannelCount; ++ch)
                {
                    color[ch] = static_cast<uint8_t>((color[ch] * scale) >> 8);
                }
            }
        }

    private:
        uint32_t _maxMilliamps;
        std::array<uint16_t, Color::ChannelCount> _milliampsPerChannel;
    };

} // namespace npb
