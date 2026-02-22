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

struct Sm16716EmitterSettings
{
    IClockDataBus& bus;
    std::array<uint8_t, 3> channelOrder = {0, 1, 2};  // RGB default
};

// SM16716 emitter.
//
// Bit-level protocol — NOT byte-aligned — pre-packed into a byte buffer.
//
// Bit stream layout:
//   Start frame: 50 zero-bits
//   Per pixel:   1 HIGH bit (separator) + 3 × 8-bit channel data = 25 bits
//
// Total bits = 50 + pixelCount × 25
// Pre-packed into ceil(totalBits / 8) bytes, MSB-first.
//
// No end frame. Entire stream transmitted as bytes via transmitBytes().
//
class Sm16716Emitter : public IEmitPixels
{
public:
    Sm16716Emitter(uint16_t pixelCount,
                   std::unique_ptr<IShader> shader,
                   Sm16716EmitterSettings settings)
        : _bus{settings.bus}
        , _shader{std::move(shader)}
        , _pixelCount{pixelCount}
        , _channelOrder{settings.channelOrder}
        , _scratchColors(pixelCount)
        , _byteBuffer((StartFrameBits + pixelCount * BitsPerPixel + 7) / 8)
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

        // Pack entire bit stream into byte buffer
        serialize(source);

        _bus.beginTransaction();
        _bus.transmitBytes(_byteBuffer);
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
    static constexpr size_t StartFrameBits = 50;
    static constexpr size_t BitsPerPixel = 25;   // 1 separator + 24 data

    IClockDataBus& _bus;
    std::unique_ptr<IShader> _shader;
    size_t _pixelCount;
    std::array<uint8_t, 3> _channelOrder;
    std::vector<Color> _scratchColors;
    std::vector<uint8_t> _byteBuffer;

    // Set a single bit in the buffer (MSB-first ordering)
    void setBit(size_t bitPos)
    {
        _byteBuffer[bitPos / 8] |= (0x80 >> (bitPos % 8));
    }

    // Pack an 8-bit value at an arbitrary bit position (MSB-first)
    void packByte(uint8_t val, size_t& bitPos)
    {
        size_t byteIdx = bitPos / 8;
        uint8_t shift = bitPos % 8;

        // Value may span two output bytes
        _byteBuffer[byteIdx] |= (val >> shift);
        if (shift > 0 && byteIdx + 1 < _byteBuffer.size())
        {
            _byteBuffer[byteIdx + 1] |= (val << (8 - shift));
        }
        bitPos += 8;
    }

    void serialize(std::span<const Color> colors)
    {
        // Clear buffer — start frame is 50 zero-bits, so zeros are default
        std::fill(_byteBuffer.begin(), _byteBuffer.end(), 0);

        size_t bitPos = StartFrameBits;  // skip 50 zero-bits

        for (const auto& color : colors)
        {
            // 1-bit HIGH separator
            setBit(bitPos++);

            // 3 channel bytes
            packByte(color[_channelOrder[0]], bitPos);
            packByte(color[_channelOrder[1]], bitPos);
            packByte(color[_channelOrder[2]], bitPos);
        }
    }
};

} // namespace npb
