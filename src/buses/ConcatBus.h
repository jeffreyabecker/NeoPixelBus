#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <algorithm>
#include "core/BufferHolder.h"
#include "core/IPixelBus.h"

namespace lw
{

    template <typename TColor, typename... TBuses>
    static constexpr bool ConcatBusCompatibleBuses =
        (std::is_convertible<std::remove_reference_t<TBuses> *, IAssignableBufferBus<TColor> *>::value && ...);

    template <typename TBus>
    auto _deduceBusColor(IPixelBus<TBus>* ) -> TBus;

    // -------------------------------------------------------------------
    // ConcatBus ? 1D concatenation of multiple IPixelBus strips
    //
    // Concatenates an arbitrary number of child buses into a single
    // virtual strip.  Child strips may have different lengths (uneven).
    // Pixel index 0 starts at the first strip, continues through each
    // subsequent strip in order.
    //
    // Each child bus is held as a borrowed IPixelBus pointer.
    //
    // Usage (borrowing):
    //   PixelBus strip0(8, emitter0);
    //   PixelBus strip1(6, emitter1);
    //   std::vector<IPixelBus*> buses;
    //   buses.emplace_back(&strip0);
    //   buses.emplace_back(&strip1);
    //   ConcatBus combined(std::move(buses));
    //
    // Usage (borrowing, variadic refs):
    //   PixelBus strip0(8, emitter0);
    //   PixelBus strip1(6, emitter1);
    //   ConcatBus combined(strip0, strip1);
    // -------------------------------------------------------------------
    template <typename TColor>
    class ConcatBus : public IPixelBus<TColor>
    {
    public:
        explicit ConcatBus(std::vector<IAssignableBufferBus<TColor> *> buses,
                           BufferHolder<TColor> colors)
            : _buses(std::move(buses))
            , _colors(std::move(colors))
        {
            _buses.erase(std::remove(_buses.begin(), _buses.end(), nullptr), _buses.end());

            if (_colors.size == 0)
            {
                size_t pixelCount = 0;
                for (auto *bus : _buses)
                {
                    if (bus != nullptr)
                    {
                        pixelCount += bus->pixelCount();
                    }
                }

                _colors = BufferHolder<TColor>{pixelCount, nullptr, true};
            }
        }

        // --- IPixelBus lifecycle ----------------------------------------

        void begin() override
        {
            _colors.init();

            size_t offset = 0;

            for (auto &bus : _buses)
            {
                auto count = static_cast<size_t>(bus->pixelCount());
                bus->setBuffer(_colors.getSpan(offset, count));
                offset += count;
                bus->begin();
            }
        }

        void show() override
        {
            for (auto &bus : _buses)
            {
                bus->show();
            }
        }

        bool canShow() const override
        {
            return std::all_of(_buses.begin(), _buses.end(),
                               [](const IAssignableBufferBus<TColor> *b)
                               { return b != nullptr && b->canShow(); });
        }

        span<TColor> pixelBuffer() override
        {
            return _colors.getSpan(0, _colors.size);
        }

        span<const TColor> pixelBuffer() const override
        {
            return _colors.getSpan(0, _colors.size);
        }

        // --- Primary IPixelBus interface (iterator pair) ----------------

    private:
        std::vector<IAssignableBufferBus<TColor> *> _buses;
        BufferHolder<TColor> _colors;

    };

} // namespace lw


