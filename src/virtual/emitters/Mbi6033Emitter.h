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

// MBI6033 emitter.
//
// 16-bit per channel, 12 channels per chip (24 bytes data per chip).
// Pixels are mapped across chips: 4 RGB pixels per chip (3 channels × 4 = 12).
//
// Protocol:
//   1. Reset sequence: 21 µs low → transmitBit(0) → 21 µs low
//   2. Header frame (6 bytes):
//        Byte 0: 0xF3 (command)
//        Byte 1: 0x00
//        Byte 2: (chipLength >> 12) & 0xFF
//        Byte 3: (chipLength >> 4)  & 0xFF
//        Byte 4: (chipLength << 4)  & 0xFF
//        Byte 5: 0x20  (config: bit 1 = ON)
//      where chipLength = chipCount - 1
//   3. Pixel data: 24 bytes per chip (12 × 16-bit PWM values)
//
// Cannot use hardware SPI — reset sequence requires direct clock pin control.
//
class Mbi6033Emitter : public IEmitPixels
{
public:
    Mbi6033Emitter(IClockDataBus& bus,
                   std::unique_ptr<IShader> shader,
                   size_t pixelCount,
                   size_t channelsPerPixel = 3)
        : _bus{bus}
        , _shader{std::move(shader)}
        , _pixelCount{pixelCount}
        , _channelsPerPixel{channelsPerPixel}
        , _chipCount{((pixelCount * channelsPerPixel) + ChannelsPerChip - 1) / ChannelsPerChip}
        , _scratchColors(pixelCount)
        , _byteBuffer(_chipCount * BytesPerChip)
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

        // Serialize: expand 8-bit channels to 16-bit, packed per chip
        std::fill(_byteBuffer.begin(), _byteBuffer.end(), 0);
        size_t channelIdx = 0;
        for (const auto& color : source)
        {
            for (size_t ch = 0; ch < _channelsPerPixel && ch < Color::ChannelCount; ++ch)
            {
                // 8-bit to 16-bit: replicate to both bytes
                uint8_t val = color[ch];
                size_t byteOffset = channelIdx * 2;
                if (byteOffset + 1 < _byteBuffer.size())
                {
                    _byteBuffer[byteOffset]     = val;  // high byte
                    _byteBuffer[byteOffset + 1] = val;  // low byte (replicate)
                }
                ++channelIdx;
            }
        }

        // Reset sequence
        delayMicroseconds(ResetDelayUs);
        _bus.transmitBit(0);
        delayMicroseconds(ResetDelayUs);

        _bus.beginTransaction();

        // Header frame (6 bytes)
        uint16_t chipLength = static_cast<uint16_t>(_chipCount > 0 ? _chipCount - 1 : 0);
        uint8_t header[HeaderSize] =
        {
            0xF3,                                            // command
            0x00,                                            // sync high
            static_cast<uint8_t>(chipLength >> 12),          // length bits 13..12
            static_cast<uint8_t>((chipLength >> 4) & 0xFF),  // length bits 11..4
            static_cast<uint8_t>((chipLength << 4) & 0xFF),  // length bits 3..0
            0x20                                             // config
        };
        _bus.transmitBytes(std::span<const uint8_t>(header, HeaderSize));

        // Pixel data
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
    static constexpr size_t ChannelsPerChip = 12;
    static constexpr size_t BytesPerChip = 24;  // 12 × 2 bytes (16-bit)
    static constexpr size_t HeaderSize = 6;
    static constexpr uint32_t ResetDelayUs = 21;

    IClockDataBus& _bus;
    std::unique_ptr<IShader> _shader;
    size_t _pixelCount;
    size_t _channelsPerPixel;
    size_t _chipCount;
    std::vector<Color> _scratchColors;
    std::vector<uint8_t> _byteBuffer;
};

} // namespace npb
