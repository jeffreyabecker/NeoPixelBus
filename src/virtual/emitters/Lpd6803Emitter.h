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
#include "../ResourceHandle.h"

namespace npb
{

struct Lpd6803EmitterSettings
{
    ResourceHandle<IClockDataBus> bus;
    std::array<uint8_t, 3> channelOrder = {0, 1, 2};  // RGB default
};

// LPD6803 emitter.
//
// Wire format: 5-5-5 packed RGB into 2 bytes per pixel (big-endian).
//   Bit 15: always 1
//   Bits 14..10: channel 1 (top 5 bits)
//   Bits  9.. 5: channel 2 (top 5 bits)
//   Bits  4.. 0: channel 3 (top 5 bits)
//
// Framing:
//   Start: 4 × 0x00
//   Pixel data: 2 bytes per pixel
//   End:   ceil(N / 8) bytes of 0x00  (1 bit per pixel)
//
class Lpd6803Emitter : public IEmitPixels
{
public:
    Lpd6803Emitter(uint16_t pixelCount,
                   ResourceHandle<IShader> shader,
                   Lpd6803EmitterSettings settings)
        : _bus{std::move(settings.bus)}
        , _shader{std::move(shader)}
        , _pixelCount{pixelCount}
        , _channelOrder{settings.channelOrder}
        , _scratchColors(pixelCount)
        , _byteBuffer(pixelCount * BytesPerPixel)
        , _endFrameSize{(pixelCount + 7u) / 8u}
    {
    }

    void initialize() override
    {
        _bus->begin();
    }

    void update(std::span<const Color> colors) override
    {
        // Apply shader
        std::span<const Color> source = colors;
        if (nullptr != _shader)
        {
            std::copy(colors.begin(), colors.end(), _scratchColors.begin());
            _shader->apply(_scratchColors);
            source = _scratchColors;
        }

        // Serialize: 5-5-5 packed into 2 bytes per pixel
        size_t offset = 0;
        for (const auto& color : source)
        {
            uint8_t ch1 = color[_channelOrder[0]] & 0xF8;
            uint8_t ch2 = color[_channelOrder[1]] & 0xF8;
            uint8_t ch3 = color[_channelOrder[2]] & 0xF8;

            // Pack: 1_ccccc_ccccc_ccccc (big-endian)
            uint16_t packed = 0x8000
                | (static_cast<uint16_t>(ch1) << 7)
                | (static_cast<uint16_t>(ch2) << 2)
                | (static_cast<uint16_t>(ch3) >> 3);

            _byteBuffer[offset++] = static_cast<uint8_t>(packed >> 8);
            _byteBuffer[offset++] = static_cast<uint8_t>(packed & 0xFF);
        }

        _bus->beginTransaction();

        // Start frame: 4 × 0x00
        for (size_t i = 0; i < StartFrameSize; ++i)
        {
            _bus->transmitByte(0x00);
        }

        // Pixel data
        _bus->transmitBytes(_byteBuffer);

        // End frame: ceil(N/8) × 0x00
        for (size_t i = 0; i < _endFrameSize; ++i)
        {
            _bus->transmitByte(0x00);
        }

        _bus->endTransaction();
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
    static constexpr size_t BytesPerPixel = 2;
    static constexpr size_t StartFrameSize = 4;

    ResourceHandle<IClockDataBus> _bus;
    ResourceHandle<IShader> _shader;
    size_t _pixelCount;
    std::array<uint8_t, 3> _channelOrder;
    std::vector<Color> _scratchColors;
    std::vector<uint8_t> _byteBuffer;
    size_t _endFrameSize;
};

} // namespace npb
