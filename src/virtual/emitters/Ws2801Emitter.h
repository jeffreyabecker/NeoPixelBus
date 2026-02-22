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

struct Ws2801EmitterSettings
{
    IClockDataBus& bus;
    std::array<uint8_t, 3> channelOrder = {0, 1, 2};  // RGB default
};

// WS2801 emitter.
//
// Wire format: raw 3 bytes per pixel, full 8-bit per channel.
// No start or end frame.
// Latch: 500 µs clock-low after last byte.
//
class Ws2801Emitter : public IEmitPixels
{
public:
    Ws2801Emitter(uint16_t pixelCount,
                  std::unique_ptr<IShader> shader,
                  Ws2801EmitterSettings settings)
        : _bus{settings.bus}
        , _shader{std::move(shader)}
        , _pixelCount{pixelCount}
        , _channelOrder{settings.channelOrder}
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

        // Serialize: raw 3-byte channel data in configured order
        size_t offset = 0;
        for (const auto& color : source)
        {
            _byteBuffer[offset++] = color[_channelOrder[0]];
            _byteBuffer[offset++] = color[_channelOrder[1]];
            _byteBuffer[offset++] = color[_channelOrder[2]];
        }

        _bus.beginTransaction();

        // No start frame — pure data stream
        _bus.transmitBytes(_byteBuffer);

        _bus.endTransaction();

        _endTime = micros();

        // Latch delay: 500 µs
        delayMicroseconds(LatchDelayUs);
    }

    bool isReadyToUpdate() const override
    {
        return (micros() - _endTime) >= LatchDelayUs;
    }

    bool alwaysUpdate() const override
    {
        return false;
    }

private:
    static constexpr size_t BytesPerPixel = 3;
    static constexpr uint32_t LatchDelayUs = 500;

    IClockDataBus& _bus;
    std::unique_ptr<IShader> _shader;
    size_t _pixelCount;
    std::array<uint8_t, 3> _channelOrder;
    std::vector<Color> _scratchColors;
    std::vector<uint8_t> _byteBuffer;
    uint32_t _endTime{0};
};

} // namespace npb
