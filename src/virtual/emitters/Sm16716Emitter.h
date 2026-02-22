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

// SM16716 emitter.
//
// Bit-level protocol — NOT byte-aligned.
//
// Start frame: 50 zero-bits total
//   - 6 bytes of 0x00 (48 bits)
//   - 2 individual LOW bits via transmitBit()
//   - 1 HIGH bit (start of first pixel frame)
//
// Per pixel: 25 bits
//   - 3 × 8-bit channel values (24 bits) via transmitBytes
//   - 1 HIGH bit via transmitBit() (frame separator)
//
// No end frame. Cannot use hardware SPI due to bit-level framing.
//
class Sm16716Emitter : public IEmitPixels
{
public:
    Sm16716Emitter(IClockDataBus& bus,
                   std::unique_ptr<IShader> shader,
                   size_t pixelCount,
                   std::array<uint8_t, 3> channelOrder = {0, 1, 2})  // RGB default
        : _bus{bus}
        , _shader{std::move(shader)}
        , _pixelCount{pixelCount}
        , _channelOrder{channelOrder}
        , _scratchColors(pixelCount)
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

        _bus.beginTransaction();

        // Start frame: 6 bytes of 0x00 (48 zero-bits)
        for (size_t i = 0; i < StartFrameBytes; ++i)
        {
            _bus.transmitByte(0x00);
        }
        // + 2 individual zero-bits
        _bus.transmitBit(0);
        _bus.transmitBit(0);

        // Per-pixel data
        uint8_t pixelBytes[3];
        for (const auto& color : source)
        {
            // Start-of-pixel HIGH bit
            _bus.transmitBit(1);

            // 3 channel bytes
            pixelBytes[0] = color[_channelOrder[0]];
            pixelBytes[1] = color[_channelOrder[1]];
            pixelBytes[2] = color[_channelOrder[2]];
            _bus.transmitBytes(std::span<const uint8_t>(pixelBytes, 3));
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
    static constexpr size_t StartFrameBytes = 6;

    IClockDataBus& _bus;
    std::unique_ptr<IShader> _shader;
    size_t _pixelCount;
    std::array<uint8_t, 3> _channelOrder;
    std::vector<Color> _scratchColors;
};

} // namespace npb
