#pragma once

#include <algorithm>
#include <cstddef>

#include "../IPixelBus.h"

namespace npb
{

    template <typename TColor>
    class NilBusT : public IPixelBus<TColor>
    {
    public:
        explicit NilBusT(size_t pixelCount = 0)
            : _pixelCount(pixelCount)
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

        size_t pixelCount() const override
        {
            return _pixelCount;
        }

        void setPixelColors(size_t,
                            ColorIteratorT<TColor>,
                            ColorIteratorT<TColor>) override
        {
        }

        void getPixelColors(size_t,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) const override
        {
            std::fill(first, last, TColor{});
        }

    private:
        size_t _pixelCount{0};
    };

} // namespace npb
