#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>

#include "../colors/Color.h"

namespace npb
{

    struct ColorOrderTransformConfig
    {
        uint8_t channelCount;                                  // 3, 4, or 5
        std::array<uint8_t, Color::ChannelCount> channelOrder; // index mapping
    };

    // ColorOrderTransform is intentionally a protocol-internal helper.
    // It is not part of the consumer-facing bus/shader API surface.
    //
    // Role:
    // - Parameterize a family of channel-packing operations with one config
    //   (channel count + channel order mapping)
    // - Keep protocol update paths concise and consistent across chips that
    //   differ mainly by color-channel ordering
    //
    // This keeps per-protocol logic focused on framing/timing/settings while
    // centralizing reusable channel-order serialization behavior.
    class ColorOrderTransform
    {
    public:
        explicit ColorOrderTransform(const ColorOrderTransformConfig &config)
            : _config{config}, _bytesPerPixel{static_cast<size_t>(config.channelCount)}
        {
        }

        void apply(std::span<uint8_t> pixels,
                   std::span<const Color> colors)
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

        size_t bytesNeeded(size_t pixelCount) const
        {
            return pixelCount * _bytesPerPixel;
        }

    private:
        ColorOrderTransformConfig _config;
        size_t _bytesPerPixel;
    };

} // namespace npb
