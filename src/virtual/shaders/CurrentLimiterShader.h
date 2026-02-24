#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>

#include "../colors/Color.h"
#include "IShader.h"

namespace npb
{

    template<typename TColor>
    class CurrentLimiterShader : public IShader<TColor>
    {
    public:
        // maxMilliamps: total power budget for the strip
        // milliampsPerChannel: current draw per channel at full brightness
        //   e.g. {20, 20, 20, 0, 0} for RGB-only at 20 mA each
        //   Channels with 0 mA are excluded from the current estimate
        //   but still scaled proportionally when over budget.
        CurrentLimiterShader(uint32_t maxMilliamps,
                             std::array<uint16_t, TColor::ChannelCount> milliampsPerChannel)
            : _maxMilliamps{maxMilliamps}, _milliampsPerChannel{milliampsPerChannel}
        {
        }

        void apply(std::span<TColor> colors) override
        {
            // Pass 1: estimate total current draw across all pixels
            // using per-channel milliamp ratings
            uint64_t totalDrawWeighted = 0;
            for (const auto& color : colors)
            {
                for (size_t ch = 0; ch < TColor::ChannelCount; ++ch)
                {
                    totalDrawWeighted += static_cast<uint64_t>(color[ch]) * _milliampsPerChannel[ch];
                }
            }

            // totalDrawWeighted is in units of (value * mA).
            // Actual milliamps = totalDrawWeighted / MaxComponent
            const uint64_t maxComponent = static_cast<uint64_t>(TColor::MaxComponent);
            uint64_t totalMilliamps = (maxComponent == 0) ? 0 : (totalDrawWeighted / maxComponent);

            if (totalMilliamps <= _maxMilliamps)
            {
                return; // within budget, no scaling needed
            }

            // Pass 2: scale all channels proportionally to fit within budget.
            // Using 16-bit fixed point: scale = (max * 65536) / total
            uint64_t scale = (static_cast<uint64_t>(_maxMilliamps) << 16) / totalMilliamps;

            for (auto& color : colors)
            {
                for (size_t ch = 0; ch < TColor::ChannelCount; ++ch)
                {
                    const uint64_t scaled = (static_cast<uint64_t>(color[ch]) * scale) >> 16;
                    color[ch] = static_cast<typename TColor::ComponentType>(scaled);
                }
            }
        }

    private:
        uint32_t _maxMilliamps;
        std::array<uint16_t, TColor::ChannelCount> _milliampsPerChannel;
    };

} // namespace npb
