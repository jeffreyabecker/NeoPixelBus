#pragma once

#include <cstddef>
#include <vector>

#include "core/IPixelBus.h"

namespace lw
{

    template <typename TColor>
    class NilBusT : public IAssignableBufferBus<TColor>
    {
    public:
        explicit NilBusT(size_t pixelCount = 0)
            : _ownedColors(pixelCount)
            , _colors(_ownedColors.data(), _ownedColors.size())
            , _pixelCount(static_cast<uint16_t>(pixelCount))
        {
        }

        void begin() override
        {
        }

        void show() override
        {
        }

        bool canShow() const override
        {
            return true;
        }

        void setBuffer(span<TColor> buffer) override
        {
            _colors = buffer;
        }

        span<TColor> pixelBuffer() override
        {
            return _colors;
        }

        span<const TColor> pixelBuffer() const override
        {
            return span<const TColor>{_colors.data(), _colors.size()};
        }

        uint16_t pixelCount() const override
        {
            return _pixelCount;
        }

    private:
        std::vector<TColor> _ownedColors;
        span<TColor> _colors;
        uint16_t _pixelCount{0};
    };

} // namespace lw


