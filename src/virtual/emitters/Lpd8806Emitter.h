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

namespace npb
{

struct Lpd8806EmitterSettings
{
    IClockDataBus& bus;
    std::array<uint8_t, 3> channelOrder = {1, 0, 2};  // GRB default
};

// LPD8806 emitter.
//
// Wire format: 7-bit color with MSB set — (value >> 1) | 0x80 per channel.
// Framing:
//   Start: ceil(N / 32) bytes of 0x00
//   Pixel data: 3 bytes per pixel
//   End:   ceil(N / 32) bytes of 0xFF
//
class Lpd8806Emitter : public IEmitPixels
{
public:
    Lpd8806Emitter(uint16_t pixelCount,
                   std::unique_ptr<IShader> shader,
                   Lpd8806EmitterSettings settings)
        : _bus{settings.bus}
        , _shader{std::move(shader)}
        , _pixelCount{pixelCount}
        , _channelOrder{settings.channelOrder}
        , _scratchColors(pixelCount)
        , _byteBuffer(pixelCount * BytesPerPixel)
        , _frameSize{(pixelCount + 31u) / 32u}
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

        // Serialize: 7-bit per channel with MSB set
        size_t offset = 0;
        for (const auto& color : source)
        {
            _byteBuffer[offset++] = (color[_channelOrder[0]] >> 1) | 0x80;
            _byteBuffer[offset++] = (color[_channelOrder[1]] >> 1) | 0x80;
            _byteBuffer[offset++] = (color[_channelOrder[2]] >> 1) | 0x80;
        }

        _bus.beginTransaction();

        // Start frame: ceil(N/32) × 0x00
        for (size_t i = 0; i < _frameSize; ++i)
        {
            _bus.transmitByte(0x00);
        }

        // Pixel data
        _bus.transmitBytes(_byteBuffer);

        // End frame: ceil(N/32) × 0xFF
        for (size_t i = 0; i < _frameSize; ++i)
        {
            _bus.transmitByte(0xFF);
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
    static constexpr size_t BytesPerPixel = 3;

    IClockDataBus& _bus;
    std::unique_ptr<IShader> _shader;
    size_t _pixelCount;
    std::array<uint8_t, 3> _channelOrder;
    std::vector<Color> _scratchColors;
    std::vector<uint8_t> _byteBuffer;
    size_t _frameSize;
};

} // namespace npb
