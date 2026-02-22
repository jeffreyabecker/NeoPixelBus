#pragma once

#include <cstdint>
#include <cstddef>
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

// P9813 emitter (Total Control Lighting).
//
// Wire format: 4 bytes per pixel.
//   Byte 0: 0xC0 | (~B >> 6 & 3) << 4 | (~G >> 6 & 3) << 2 | (~R >> 6 & 3)
//   Byte 1: Blue
//   Byte 2: Green
//   Byte 3: Red
//
// The header byte contains inverted top-2-bits of each channel as a checksum.
// Fixed channel order: BGR in data bytes.
//
// Framing:
//   Start: 4 × 0x00
//   End:   4 × 0x00
//
class P9813Emitter : public IEmitPixels
{
public:
    P9813Emitter(IClockDataBus& bus,
                 std::unique_ptr<IShader> shader,
                 size_t pixelCount)
        : _bus{bus}
        , _shader{std::move(shader)}
        , _pixelCount{pixelCount}
        , _scratchColors(pixelCount)
        , _byteBuffer(pixelCount * BytesPerPixel)
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

        // Serialize: checksum prefix + BGR
        size_t offset = 0;
        for (const auto& color : source)
        {
            uint8_t r = color[Color::IdxR];
            uint8_t g = color[Color::IdxG];
            uint8_t b = color[Color::IdxB];

            // Header: 0xC0 | inverted top-2-bits of each channel
            uint8_t header = 0xC0
                | ((~b >> 6) & 0x03) << 4
                | ((~g >> 6) & 0x03) << 2
                | ((~r >> 6) & 0x03);

            _byteBuffer[offset++] = header;
            _byteBuffer[offset++] = b;
            _byteBuffer[offset++] = g;
            _byteBuffer[offset++] = r;
        }

        _bus.beginTransaction();

        // Start frame: 4 × 0x00
        for (size_t i = 0; i < FrameSize; ++i)
        {
            _bus.transmitByte(0x00);
        }

        // Pixel data
        _bus.transmitBytes(_byteBuffer);

        // End frame: 4 × 0x00
        for (size_t i = 0; i < FrameSize; ++i)
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
    static constexpr size_t FrameSize = 4;

    IClockDataBus& _bus;
    std::unique_ptr<IShader> _shader;
    size_t _pixelCount;
    std::vector<Color> _scratchColors;
    std::vector<uint8_t> _byteBuffer;
};

} // namespace npb
