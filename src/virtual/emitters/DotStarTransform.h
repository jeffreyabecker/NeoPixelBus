#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>

#include "../colors/Color.h"
#include "ITransformColorToBytes.h"

namespace npb
{

// DotStar (APA102 / HD108) pixel serialization.
//
// Two modes:
//   FixedBrightness — 0xFF prefix byte, 3 color channels (ignores W)
//   Luminance       — 0xE0 | (WW clamped to 0-31) prefix, 3 color channels
//
// Wire format per pixel: [prefix] [ch1] [ch2] [ch3]   (4 bytes)
//
// Channel order is configurable via channelOrder[3].
// Channel indices refer to Color::operator[] (0=R, 1=G, 2=B, 3=WW, 4=CW).

enum class DotStarMode : uint8_t
{
    FixedBrightness,   // 0xFF prefix — W channel ignored
    Luminance,         // 0xE0 | WW — uses WW channel as 5-bit luminance
};

struct DotStarTransformConfig
{
    std::array<uint8_t, 3> channelOrder;   // e.g., {2,1,0} for BGR
    DotStarMode mode{DotStarMode::FixedBrightness};
};

class DotStarTransform : public ITransformColorToBytes
{
public:
    explicit DotStarTransform(const DotStarTransformConfig& config)
        : _config{config}
    {
    }

    void apply(std::span<uint8_t> pixels,
               std::span<const Color> colors) override
    {
        size_t offset = 0;

        if (_config.mode == DotStarMode::FixedBrightness)
        {
            for (const auto& color : colors)
            {
                pixels[offset++] = 0xFF;
                pixels[offset++] = color[_config.channelOrder[0]];
                pixels[offset++] = color[_config.channelOrder[1]];
                pixels[offset++] = color[_config.channelOrder[2]];
            }
        }
        else // Luminance
        {
            for (const auto& color : colors)
            {
                // WW channel (index 3) provides 5-bit luminance, clamped to 0-31
                uint8_t lum = color[Color::IdxWW] < 31 ? color[Color::IdxWW] : 31;
                pixels[offset++] = 0xE0 | lum;
                pixels[offset++] = color[_config.channelOrder[0]];
                pixels[offset++] = color[_config.channelOrder[1]];
                pixels[offset++] = color[_config.channelOrder[2]];
            }
        }
    }

    size_t bytesNeeded(size_t pixelCount) const override
    {
        return pixelCount * BytesPerPixel;
    }

private:
    static constexpr size_t BytesPerPixel = 4;
    DotStarTransformConfig _config;
};

} // namespace npb
