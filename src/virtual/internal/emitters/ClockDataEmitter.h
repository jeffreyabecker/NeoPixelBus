#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

#include <Arduino.h>

#include "IEmitPixels.h"
#include "../buses/IClockDataBus.h"
#include "../buses/ClockDataProtocol.h"

namespace npb
{

class ClockDataEmitter : public IEmitPixels
{
public:
    ClockDataEmitter(IClockDataBus& bus,
                     const ClockDataProtocol& protocol,
                     size_t pixelCount)
        : _bus{bus}
        , _protocol{protocol}
        , _pixelCount{pixelCount}
    {
    }

    void initialize() override
    {
        _bus.begin();
    }

    void update(std::span<const uint8_t> data) override
    {
        _bus.beginTransaction();

        // Start frame
        if (!_protocol.startFrame.empty())
        {
            _bus.transmitBytes(_protocol.startFrame);
        }

        // Pixel data
        _bus.transmitBytes(data);

        // Fixed end frame
        if (!_protocol.endFrame.empty())
        {
            _bus.transmitBytes(_protocol.endFrame);
        }

        // Per-pixel end frame bits
        // DotStar: 1 bit per 2 pixels → ceil(N * bitsPerPixel / 2 / 8) bytes
        // General: ceil(N * endFrameBitsPerPixel / 8) bytes, but DotStar
        // is actually ceil(N / 16) — 1 bit per 2 pixels.
        // We use the original formula: (pixelCount + 15) / 16 for 1 bit/2px
        if (_protocol.endFrameBitsPerPixel > 0)
        {
            // Calculate bytes needed: ceil(pixelCount * bitsPerPixel / 2 / 8)
            // For DotStar (1 bit per 2 pixels): (pixelCount + 15) / 16
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
    size_t _pixelCount;
};

} // namespace npb
