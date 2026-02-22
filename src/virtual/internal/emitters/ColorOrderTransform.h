#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>

#include "../colors/Color.h"
#include "ITransformColorToBytes.h"
#include "SettingsData.h"

namespace npb
{

    struct ColorOrderTransformConfig
    {
        uint8_t channelCount;                                  // 3, 4, or 5
        std::array<uint8_t, Color::ChannelCount> channelOrder; // index mapping
    };

    class ColorOrderTransform : public ITransformColorToBytes
    {
    public:
        explicit ColorOrderTransform(const ColorOrderTransformConfig &config)
            : _config{config}, _bytesPerPixel{static_cast<size_t>(config.channelCount)}
        {
        }

        void apply(std::span<uint8_t> pixels,
                   std::span<const Color> colors) override
        {
            size_t offset = 0;

            // Write pixel color data in configured channel order
            for (const auto &color : colors)
            {
                for (uint8_t ch = 0; ch < _config.channelCount; ++ch)
                {
                    pixels[offset++] = color[_config.channelOrder[ch]];
                }
            }
        }

        size_t bytesNeeded(size_t pixelCount) const override
        {
            return pixelCount * _bytesPerPixel;
        }

    private:
        ColorOrderTransformConfig _config;
        size_t _bytesPerPixel;
    };

} // namespace npb
