#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <memory>
#include <vector>

#include <Arduino.h>

#include "IEmitPixels.h"
#include "../shaders/IShader.h"
#include "../buses/IClockDataBus.h"
#include "../buses/ClockDataProtocol.h"
#include "ITransformColorToBytes.h"

namespace npb
{

class ClockDataEmitter : public IEmitPixels
{
public:
    ClockDataEmitter(IClockDataBus& bus,
                     const ClockDataProtocol& protocol,
                     ITransformColorToBytes& transform,
                     std::unique_ptr<IShader> shader,
                     size_t pixelCount)
        : _bus{bus}
        , _protocol{protocol}
        , _transform{transform}
        , _shader{std::move(shader)}
        , _pixelCount{pixelCount}
        , _scratchColors(pixelCount)
        , _byteBuffer(_transform.bytesNeeded(pixelCount))
    {
    }

    void initialize() override
    {
        _bus.begin();
    }

    void update(std::span<const Color> colors) override
    {
        if (_shader)
        {
            // Copy to scratch so the shader can mutate without
            // touching the caller's pixel buffer
            std::copy(colors.begin(), colors.end(), _scratchColors.begin());
            _shader->apply(_scratchColors);
            _transform.apply(_byteBuffer, _scratchColors);
        }
        else
        {
            _transform.apply(_byteBuffer, colors);
        }

        _bus.beginTransaction();

        // Start frame
        if (!_protocol.startFrame.empty())
        {
            _bus.transmitBytes(_protocol.startFrame);
        }

        // Pixel data
        _bus.transmitBytes(_byteBuffer);

        // Fixed end frame
        if (!_protocol.endFrame.empty())
        {
            _bus.transmitBytes(_protocol.endFrame);
        }

        // Per-pixel end frame bits
        if (_protocol.endFrameBitsPerPixel > 0)
        {
            size_t endBytes = (_pixelCount * _protocol.endFrameBitsPerPixel + 15) / 16;
            for (size_t i = 0; i < endBytes; ++i)
            {
                _bus.transmitByte(_protocol.endFrameFillByte);
            }
        }

        _bus.endTransaction();

        // Latch delay
        if (_protocol.latchDelayUs > 0)
        {
            delayMicroseconds(_protocol.latchDelayUs);
        }
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
    IClockDataBus& _bus;
    const ClockDataProtocol& _protocol;
    ITransformColorToBytes& _transform;
    std::unique_ptr<IShader> _shader;
    size_t _pixelCount;
    std::vector<Color> _scratchColors;      // pre-allocated at construction
    std::vector<uint8_t> _byteBuffer;       // pre-allocated at construction
};

} // namespace npb
