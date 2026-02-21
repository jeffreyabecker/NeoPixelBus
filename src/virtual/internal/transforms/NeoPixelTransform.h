#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>

#include "../colors/Color.h"
#include "ITransformColorToBytes.h"

namespace npb
{

struct NeoPixelTransformConfig
{
    uint8_t channelCount;                                    // 3, 4, or 5
    std::array<uint8_t, Color::ChannelCount> channelOrder;   // index mapping
    // Phase 6 adds: std::optional<std::variant<...>> inBandSettings;
};

class NeoPixelTransform : public ITransformColorToBytes
{
public:
    explicit NeoPixelTransform(const NeoPixelTransformConfig& config)
        : _config{config}
        , _bytesPerPixel{static_cast<size_t>(config.channelCount)}
    {
    }

    void apply(std::span<uint8_t> pixels,
               std::span<const Color> colors) override
    {
        size_t offset = 0;

        for (const auto& color : colors)
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
    NeoPixelTransformConfig _config;
    size_t _bytesPerPixel;
};

} // namespace npb
