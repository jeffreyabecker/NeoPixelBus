#pragma once

// TODO: Rework CurrentLimiterShader for per-pixel IShader interface.
// The two-pass approach (sum all pixels, then scale) doesn't fit the
// single-pixel apply() model. Needs architectural rethink.

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

        void begin(std::span<const Color> colors) override
        {
            // Estimate total current draw across all pixels using
            // per-channel milliamp ratings
            uint32_t totalDrawWeighted = 0;
            for (const auto &color : colors)
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
                _scale = 0; // within budget, no scaling needed
                return;
            }

            // Scale all channels proportionally to fit within budget
            // Using 16-bit fixed point: scale = (max * 256) / total
            _scale = (_maxMilliamps * 256) / totalMilliamps;
        }

        const Color apply(uint16_t, const Color color) override
        {
            if (_scale == 0)
            {
                return color;
            }

            Color result = color;
            for (size_t ch = 0; ch < Color::ChannelCount; ++ch)
            {
                result[ch] = static_cast<uint8_t>((result[ch] * _scale) >> 8);
            }
            return result;
        }

    private:
        uint32_t _scale = 0;
        uint32_t _maxMilliamps;
        std::array<uint16_t, Color::ChannelCount> _milliampsPerChannel;
    };

} // namespace npb
