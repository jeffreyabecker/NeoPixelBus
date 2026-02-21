#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

#include "../colors/Color.h"
#include "IShader.h"

namespace npb
{

class CurrentLimiterShader : public IShader
{
public:
    // maxMilliamps: total power budget for the strip
    // milliampsPerChannel: current draw per channel at full brightness (typ. 20 mA)
    CurrentLimiterShader(uint32_t maxMilliamps, uint16_t milliampsPerChannel)
        : _maxMilliamps{maxMilliamps}
        , _milliampsPerChannel{milliampsPerChannel}
    {
    }

    void apply(std::span<Color> colors) override
    {
        // Estimate total current draw across all pixels and channels
        uint32_t totalDraw = 0;
        for (const auto& color : colors)
        {
            for (size_t ch = 0; ch < Color::ChannelCount; ++ch)
            {
                totalDraw += color[ch];
            }
        }

        // Scale from 0-255 channel value to milliamps:
        // channelCurrent = (value / 255) * milliampsPerChannel
        // totalCurrent = sum of all channelCurrents
        // = totalDraw * milliampsPerChannel / 255
        uint32_t totalMilliamps = (totalDraw * _milliampsPerChannel) / 255;

        if (totalMilliamps <= _maxMilliamps)
        {
            return; // within budget, no scaling needed
        }

        // Scale all channels proportionally to fit within budget
        // scaleFactor = maxMilliamps / totalMilliamps, applied as fixed-point
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
    uint16_t _milliampsPerChannel;
};

} // namespace npb
