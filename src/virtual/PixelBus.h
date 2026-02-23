#pragma once

#include <cstddef>
#include <vector>
#include <algorithm>
#include <memory>

#include "IPixelBus.h"
#include "emitters/IProtocol.h"
#include "ResourceHandle.h"

namespace npb
{

    class PixelBus : public IPixelBus
    {
    public:
        PixelBus(size_t pixelCount,
                 ResourceHandle<IProtocol> emitter)
            : _colors(pixelCount), _emitter{std::move(emitter)}
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

        std::span<Color> colors()
        {
            return _colors;
        }

        std::span<const Color> colors() const
        {
            return _colors;
        }

        // -----------------------------------------------------------------
        // Primary interface overrides (iterator pair)
        // -----------------------------------------------------------------
        void setPixelColors(size_t offset,
                            ColorIterator first, ColorIterator last) override
        {
            auto available = static_cast<std::ptrdiff_t>(_colors.size() - offset);
            auto requested = last - first;
            auto count     = std::min(requested, available);
            std::copy_n(first, count, _colors.begin() + offset);
            _dirty = true;
        }

        void getPixelColors(size_t offset,
                            ColorIterator first, ColorIterator last) const override
        {
            auto available = static_cast<std::ptrdiff_t>(_colors.size() - offset);
            auto requested = last - first;
            auto count     = std::min(requested, available);
            std::copy_n(_colors.cbegin() + offset, count, first);
        }

        // -----------------------------------------------------------------
        // Convenience overrides – span (direct copy, no iterator wrapper)
        // -----------------------------------------------------------------
        void setPixelColors(size_t offset,
                            std::span<const Color> pixelData) override
        {
            auto available = _colors.size() - offset;
            auto count     = std::min(pixelData.size(), available);
            std::copy_n(pixelData.begin(), count, _colors.begin() + offset);
            _dirty = true;
        }

        void getPixelColors(size_t offset,
                            std::span<Color> pixelData) const override
        {
            auto available = _colors.size() - offset;
            auto count     = std::min(pixelData.size(), available);
            std::copy_n(_colors.cbegin() + offset, count, pixelData.begin());
        }

        // -----------------------------------------------------------------
        // Convenience overrides – single pixel (direct vector access)
        // -----------------------------------------------------------------
        void setPixelColor(size_t index, const Color& color) override
        {
            if (index < _colors.size())
            {
                _colors[index] = color;
                _dirty = true;
            }
        }

        Color getPixelColor(size_t index) const override
        {
            if (index < _colors.size())
            {
                return _colors[index];
            }
            return Color{};
        }

    private:
        std::vector<Color> _colors;
        ResourceHandle<IProtocol> _emitter;
        bool _dirty{false};
    };

} // namespace npb
