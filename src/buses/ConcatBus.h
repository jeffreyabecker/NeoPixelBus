#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <algorithm>
#include "core/IPixelBus.h"

namespace npb
{

    template <typename TColor, typename... TBuses>
    static constexpr bool ConcatBusCompatibleBuses =
        (std::is_convertible<std::remove_reference_t<TBuses> *, IPixelBus<TColor> *>::value && ...);

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
        explicit ConcatBus(std::vector<IPixelBus<TColor> *> buses)
            : _buses(std::move(buses))
        {
            _buses.erase(std::remove(_buses.begin(), _buses.end(), nullptr), _buses.end());
            _buildOffsetTable();
        }

        template <typename... TBuses,
                  typename = std::enable_if_t<ConcatBusCompatibleBuses<TColor, TBuses...>>>
        explicit ConcatBus(TBuses &...buses)
        {
            _buses.reserve(sizeof...(buses));
            (_buses.emplace_back(&buses), ...);
            _buildOffsetTable();
        }

        void add(IPixelBus<TColor> *bus)
        {
            if (nullptr == bus)
            {
                return;
            }

            _buses.emplace_back(bus);
            _buildOffsetTable();
        }

        bool remove(IPixelBus<TColor> &bus)
        {
            auto it = std::find(_buses.begin(), _buses.end(), &bus);
            if (it == _buses.end())
            {
                return false;
            }

            _buses.erase(it);
            _buildOffsetTable();
            return true;
        }



        // --- IPixelBus lifecycle ----------------------------------------

        void begin() override
        {
            for (auto &bus : _buses)
            {
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
                               [](const IPixelBus<TColor> *b)
                               { return b != nullptr && b->canShow(); });
        }

        size_t pixelCount() const override
        {
            return _totalPixelCount;
        }

        // --- Primary IPixelBus interface (iterator pair) ----------------

        void setPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) override
        {
            auto count = static_cast<size_t>(last - first);

            for (size_t i = 0; i < count; ++i)
            {
                size_t globalIdx = offset + i;
                auto resolved = _resolve(globalIdx);
                if (resolved.isValid())
                {
                    _buses[resolved.busIndex]->setPixelColor(
                        resolved.localIndex,
                        first[static_cast<std::ptrdiff_t>(i)]);
                }
            }
        }

        void getPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) const override
        {
            auto count = static_cast<size_t>(last - first);

            for (size_t i = 0; i < count; ++i)
            {
                size_t globalIdx = offset + i;
                auto resolved = _resolve(globalIdx);
                if (resolved.isValid())
                {
                    first[static_cast<std::ptrdiff_t>(i)] =
                        _buses[resolved.busIndex]->getPixelColor(
                            resolved.localIndex);
                }
            }
        }

    private:
        std::vector<IPixelBus<TColor> *> _buses;

        // Prefix-sum offset table: _offsets[i] = starting linear index
        // of bus i in the flattened pixel space.
        std::vector<size_t> _offsets;
        size_t _totalPixelCount{0};

        struct ResolvedPixel
        {
            size_t busIndex;
            size_t localIndex;

            static constexpr size_t InvalidIndex = static_cast<size_t>(-1);

            bool isValid() const
            {
                return busIndex != InvalidIndex;
            }
        };

        // ---------------------------------------------------------------
        // Build prefix-sum offset table at construction
        // ---------------------------------------------------------------
        void _buildOffsetTable()
        {
            _offsets.resize(_buses.size());
            size_t running = 0;
            for (size_t i = 0; i < _buses.size(); ++i)
            {
                _offsets[i] = running;
                running += _buses[i]->pixelCount();
            }
            _totalPixelCount = running;
        }

        // ---------------------------------------------------------------
        // _resolve ? map linear index ? bus + local pixel index
        //
        // Uses binary search on the prefix-sum table (O(log N) buses).
        // Supports uneven-length strips naturally.
        // ---------------------------------------------------------------
        ResolvedPixel _resolve(size_t globalIdx) const
        {
            if (globalIdx >= _totalPixelCount)
            {
                return ResolvedPixel{ResolvedPixel::InvalidIndex, ResolvedPixel::InvalidIndex};
            }

            // Find bus: largest i where _offsets[i] <= globalIdx
            auto it = std::upper_bound(_offsets.begin(), _offsets.end(),
                                       globalIdx);
            size_t busIdx = static_cast<size_t>(
                                std::distance(_offsets.begin(), it)) -
                            1;
            size_t localIdx = globalIdx - _offsets[busIdx];

            return ResolvedPixel{busIdx, localIdx};
        }

    };

} // namespace npb


