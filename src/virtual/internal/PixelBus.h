#pragma once

#include <cstddef>
#include <vector>
#include <span>
#include <algorithm>
#include <memory>

#include "IPixelBus.h"
#include "emitters/IEmitPixels.h"

namespace npb
{

class PixelBus : public IPixelBus
{
public:
    PixelBus(size_t pixelCount,
             std::unique_ptr<IEmitPixels> emitter)
        : _colors(pixelCount)
        , _emitter{std::move(emitter)}
    {
    }

    void begin() override
    {
        _emitter->initialize();
    }

    void show() override
    {
        if (!_dirty && !_emitter->alwaysUpdate())
        {
            return;
        }
        _emitter->update(_colors);
        _dirty = false;
    }

    bool canShow() const override
    {
        return _emitter->isReadyToUpdate();
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
    std::unique_ptr<IEmitPixels> _emitter;
    bool                    _dirty{false};
};

} // namespace npb
