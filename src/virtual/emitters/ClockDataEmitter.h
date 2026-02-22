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
    {
    }

    void initialize() override
    {
        _bus.begin();
    }

    void update(std::span<const Color> colors) override
    {
        // Shade colors and serialize to byte buffer
        _byteBuffer.resize(_transform.bytesNeeded(colors.size()));

        if (_shader)
        {
            _shader->begin(colors);
            // Apply shader per-pixel into scratch, then transform
            size_t offset = 0;
            const size_t bpp = _transform.bytesNeeded(1);
            for (uint16_t i = 0; i < colors.size(); ++i)
            {
                Color shaded = _shader->apply(i, colors[i]);
                _transform.apply(
                    std::span<uint8_t>(_byteBuffer).subspan(offset, bpp),
                    std::span<const Color>(&shaded, 1));
                offset += bpp;
            }
            _shader->end();
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
    std::vector<uint8_t> _byteBuffer;
    size_t _pixelCount;
};

} // namespace npb
