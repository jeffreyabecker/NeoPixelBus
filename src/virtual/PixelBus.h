#pragma once

#include <cstddef>
#include <vector>
#include <algorithm>
#include <memory>

#include "IPixelBus.h"
#include "protocols/IProtocol.h"
#include "ResourceHandle.h"

namespace npb
{

    template <typename TColor = Color>
    class PixelBusT : public IPixelBusT<TColor>
    {
    public:
        PixelBusT(size_t pixelCount,
                  ResourceHandle<IProtocol<TColor>> protocol)
            : _colors(pixelCount), _protocol{std::move(protocol)}
        {
        }

        void begin() override
        {
            _protocol->initialize();
        }

        void show() override
        {
            if (!_dirty && !_protocol->alwaysUpdate())
            {
                return;
            }
            _protocol->update(_colors);
            _dirty = false;
        }

        bool canShow() const override
        {
            return _protocol->isReadyToUpdate();
        }

        size_t pixelCount() const override
        {
            return _colors.size();
        }

        std::span<TColor> colors()
        {
            return _colors;
        }

        std::span<const TColor> colors() const
        {
            return _colors;
        }

        // -----------------------------------------------------------------
        // Primary interface overrides (iterator pair)
        // -----------------------------------------------------------------
        void setPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) override
        {
            auto available = static_cast<std::ptrdiff_t>(_colors.size() - offset);
            auto requested = last - first;
            auto count     = std::min(requested, available);
            std::copy_n(first, count, _colors.begin() + offset);
            _dirty = true;
        }

        void getPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) const override
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
                            std::span<const TColor> pixelData) override
        {
            auto available = _colors.size() - offset;
            auto count     = std::min(pixelData.size(), available);
            std::copy_n(pixelData.begin(), count, _colors.begin() + offset);
            _dirty = true;
        }

        void getPixelColors(size_t offset,
                            std::span<TColor> pixelData) const override
        {
            auto available = _colors.size() - offset;
            auto count     = std::min(pixelData.size(), available);
            std::copy_n(_colors.cbegin() + offset, count, pixelData.begin());
        }

        // -----------------------------------------------------------------
        // Convenience overrides – single pixel (direct vector access)
        // -----------------------------------------------------------------
        void setPixelColor(size_t index, const TColor& color) override
        {
            if (index < _colors.size())
            {
                _colors[index] = color;
                _dirty = true;
            }
        }

        TColor getPixelColor(size_t index) const override
        {
            if (index < _colors.size())
            {
                return _colors[index];
            }
            return TColor{};
        }

    private:
        std::vector<TColor> _colors;
        ResourceHandle<IProtocol<TColor>> _protocol;
        bool _dirty{false};
    };

    using PixelBus = PixelBusT<Color>;

} // namespace npb
