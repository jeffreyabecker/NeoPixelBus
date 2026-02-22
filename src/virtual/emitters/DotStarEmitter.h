#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>
#include <memory>
#include <vector>
#include <algorithm>

#include <Arduino.h>

#include "IEmitPixels.h"
#include "../shaders/IShader.h"
#include "../buses/IClockDataBus.h"
#include "../colors/Color.h"

namespace npb
{

// DotStar / APA102 brightness modes.
//
//   FixedBrightness — 0xFF prefix byte, W channel ignored
//   Luminance       — 0xE0 | WW prefix, uses WW channel as 5-bit luminance
//
enum class DotStarMode : uint8_t
{
    FixedBrightness,
    Luminance,
};

struct DotStarEmitterSettings
{
    IClockDataBus& bus;
    std::array<uint8_t, 3> channelOrder = {2, 1, 0};  // BGR default
    DotStarMode mode = DotStarMode::FixedBrightness;
};

// DotStar / APA102 emitter.
//
// Wire format per pixel: [prefix] [ch1] [ch2] [ch3]  (4 bytes)
// Framing:
//   Start: 4 x 0x00
//   End:   4 x 0x00 + ceil(N/16) x 0x00
//
class DotStarEmitter : public IEmitPixels
{
public:
    DotStarEmitter(uint16_t pixelCount,
                   std::unique_ptr<IShader> shader,
                   DotStarEmitterSettings settings)
        : _bus{settings.bus}
        , _shader{std::move(shader)}
        , _pixelCount{pixelCount}
        , _channelOrder{settings.channelOrder}
        , _mode{settings.mode}
        , _scratchColors(pixelCount)
        , _byteBuffer(pixelCount * BytesPerPixel)
        , _endFrameExtraBytes{(pixelCount + 15u) / 16u}
    {
    }

    void initialize() override
    {
        _bus.begin();
    }

    void update(std::span<const Color> colors) override
    {
        // Apply shader
        std::span<const Color> source = colors;
        if (_shader)
        {
            std::copy(colors.begin(), colors.end(), _scratchColors.begin());
            _shader->apply(_scratchColors);
            source = _scratchColors;
        }

        // Serialize
        size_t offset = 0;
        if (_mode == DotStarMode::FixedBrightness)
        {
            for (const auto& color : source)
            {
                _byteBuffer[offset++] = 0xFF;
                _byteBuffer[offset++] = color[_channelOrder[0]];
                _byteBuffer[offset++] = color[_channelOrder[1]];
                _byteBuffer[offset++] = color[_channelOrder[2]];
            }
        }
        else // Luminance
        {
            for (const auto& color : source)
            {
                uint8_t lum = color[Color::IdxWW] < 31 ? color[Color::IdxWW] : 31;
                _byteBuffer[offset++] = 0xE0 | lum;
                _byteBuffer[offset++] = color[_channelOrder[0]];
                _byteBuffer[offset++] = color[_channelOrder[1]];
                _byteBuffer[offset++] = color[_channelOrder[2]];
            }
        }

        _bus.beginTransaction();

        // Start frame: 4 x 0x00
        for (size_t i = 0; i < StartFrameSize; ++i)
        {
            _bus.transmitByte(0x00);
        }

        // Pixel data
        _bus.transmitBytes(_byteBuffer);

        // End frame: 4 x 0x00
        for (size_t i = 0; i < EndFrameFixedSize; ++i)
        {
            _bus.transmitByte(0x00);
        }

        // Extra end-frame bytes: ceil(N/16) x 0x00
        for (size_t i = 0; i < _endFrameExtraBytes; ++i)
        {
            _bus.transmitByte(0x00);
        }

        _bus.endTransaction();
    }

    bool isReadyToUpdate() const override
    {
        return true;
    }

    bool alwaysUpdate() const override
    {
        return false;
    }

private:
    static constexpr size_t BytesPerPixel = 4;
    static constexpr size_t StartFrameSize = 4;
    static constexpr size_t EndFrameFixedSize = 4;

    IClockDataBus& _bus;
    std::unique_ptr<IShader> _shader;
    size_t _pixelCount;
    std::array<uint8_t, 3> _channelOrder;
    DotStarMode _mode;
    std::vector<Color> _scratchColors;
    std::vector<uint8_t> _byteBuffer;
    size_t _endFrameExtraBytes;
};

} // namespace npb
