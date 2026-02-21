#pragma once

#include <cstddef>
#include <vector>
#include <span>
#include <algorithm>

#include "IPixelBus.h"
#include "transforms/ITransformColorToBytes.h"
#include "emitters/IEmitPixels.h"

namespace npb
{

class PixelBus : public IPixelBus
{
public:
    PixelBus(size_t pixelCount,
             ITransformColorToBytes& transform,
             IEmitPixels& emitter)
        : _colors(pixelCount)
        , _transform{transform}
        , _emitter{emitter}
    {
    }

    void begin() override
    {
        _byteBuffer.resize(_transform.bytesNeeded(_colors.size()));
        _emitter.initialize();
    }

    void show() override
    {
        if (!_dirty && !_emitter.alwaysUpdate())
        {
            return;
        }
        _transform.apply(_byteBuffer, _colors);
        _emitter.update(_byteBuffer);
        _dirty = false;
    }

    bool canShow() const override
    {
        return _emitter.isReadyToUpdate();
    }

    size_t pixelCount() const override
    {
        return _colors.size();
    }

    std::span<Color> colors() override
    {
        return _colors;
    }

    std::span<const Color> colors() const override
    {
        return _colors;
    }

    void setPixelColor(size_t offset,
                       std::span<const Color> pixelData) override
    {
        size_t count = std::min(pixelData.size(), _colors.size() - offset);
        std::copy_n(pixelData.begin(), count, _colors.begin() + offset);
        _dirty = true;
    }

    void getPixelColor(size_t offset,
                       std::span<Color> pixelData) const override
    {
        size_t count = std::min(pixelData.size(), _colors.size() - offset);
        std::copy_n(_colors.begin() + offset, count, pixelData.begin());
    }

    // Single-pixel convenience (not on IPixelBus)
    void setPixelColor(size_t index, const Color& color)
    {
        if (index < _colors.size())
        {
            _colors[index] = color;
            _dirty = true;
        }
    }

    Color getPixelColor(size_t index) const
    {
        if (index < _colors.size())
        {
            return _colors[index];
        }
        return Color{};
    }

private:
    std::vector<Color>      _colors;
    std::vector<uint8_t>    _byteBuffer;
    ITransformColorToBytes& _transform;
    IEmitPixels&            _emitter;
    bool                    _dirty{false};
};

} // namespace npb
