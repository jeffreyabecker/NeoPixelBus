#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <optional>
#include <algorithm>
#include "IPixelBus.h"
#include "ResourceHandle.h"

namespace npb
{

    // -------------------------------------------------------------------
    // ConcatBus — 1D concatenation of multiple IPixelBus strips
    //
    // Concatenates an arbitrary number of child buses into a single
    // virtual strip.  Child strips may have different lengths (uneven).
    // Pixel index 0 starts at the first strip, continues through each
    // subsequent strip in order.
    //
    // Each child bus is held via ResourceHandle<IPixelBus>: pass a
    // unique_ptr to transfer ownership, or pass a reference to borrow.
    //
    // Usage (borrowing):
    //   PixelBus strip0(8, emitter0);
    //   PixelBus strip1(6, emitter1);
    //   std::vector<ResourceHandle<IPixelBus>> buses;
    //   buses.emplace_back(strip0);          // borrow
    //   buses.emplace_back(strip1);          // borrow
    //   ConcatBus combined(std::move(buses));
    //
    // Usage (owning):
    //   std::vector<ResourceHandle<IPixelBus>> buses;
    //   buses.emplace_back(std::make_unique<PixelBus>(8, emitter0));
    //   buses.emplace_back(std::make_unique<PixelBus>(6, emitter1));
    //   ConcatBus combined(std::move(buses));
    // -------------------------------------------------------------------
    template <typename TColor = Color>
    class ConcatBusT : public IPixelBusT<TColor>
    {
    public:
        explicit ConcatBusT(std::vector<ResourceHandle<IPixelBusT<TColor>>> buses)
            : _buses(std::move(buses))
        {
            _buildOffsetTable();
        }

        // --- IPixelBus lifecycle ----------------------------------------

        void begin() override
        {
            for (auto& bus : _buses)
            {
                bus->begin();
            }
        }

        void show() override
        {
            for (auto& bus : _buses)
            {
                bus->show();
            }
        }

        bool canShow() const override
        {
            return std::all_of(_buses.begin(), _buses.end(),
                [](const ResourceHandle<IPixelBusT<TColor>>& b) { return b->canShow(); });
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
                if (resolved)
                {
                    _buses[resolved->busIndex]->setPixelColor(
                        resolved->localIndex,
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
                if (resolved)
                {
                    first[static_cast<std::ptrdiff_t>(i)] =
                        _buses[resolved->busIndex]->getPixelColor(
                            resolved->localIndex);
                }
            }
        }

    private:
        std::vector<ResourceHandle<IPixelBusT<TColor>>> _buses;

        // Prefix-sum offset table: _offsets[i] = starting linear index
        // of bus i in the flattened pixel space.
        std::vector<size_t> _offsets;
        size_t _totalPixelCount{0};

        struct ResolvedPixel
        {
            size_t busIndex;
            size_t localIndex;
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
        // _resolve — map linear index → bus + local pixel index
        //
        // Uses binary search on the prefix-sum table (O(log N) buses).
        // Supports uneven-length strips naturally.
        // ---------------------------------------------------------------
        std::optional<ResolvedPixel> _resolve(size_t globalIdx) const
        {
            if (globalIdx >= _totalPixelCount)
            {
                return std::nullopt;
            }

            // Find bus: largest i where _offsets[i] <= globalIdx
            auto it = std::upper_bound(_offsets.begin(), _offsets.end(),
                                       globalIdx);
            size_t busIdx = static_cast<size_t>(
                std::distance(_offsets.begin(), it)) - 1;
            size_t localIdx = globalIdx - _offsets[busIdx];

            return ResolvedPixel{busIdx, localIdx};
        }
    };

    using ConcatBus = ConcatBusT<Color>;

} // namespace npb
