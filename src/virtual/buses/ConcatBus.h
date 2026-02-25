#pragma once

#include <cstddef>
#include <cstdint>
#include <concepts>
#include <array>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <optional>
#include <algorithm>
#include "../IPixelBus.h"
#include "../ResourceHandle.h"

namespace npb
{

    template <typename TBus>
    auto _deduceBusColor(IPixelBus<TBus>* ) -> TBus;

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
        explicit ConcatBus(std::vector<ResourceHandle<IPixelBus<TColor>>> buses)
            : _buses(std::move(buses))
        {
            _buildOffsetTable();
        }

        template <typename... TBuses>
            requires(std::convertible_to<TBuses *, IPixelBus<TColor> *> && ...)
        explicit ConcatBus(TBuses &...buses)
        {
            _buses.reserve(sizeof...(buses));
            (_buses.emplace_back(buses), ...);
            _buildOffsetTable();
        }

        void add(ResourceHandle<IPixelBus<TColor>> bus)
        {
            if (bus == nullptr)
            {
                return;
            }

            _buses.emplace_back(std::move(bus));
            _buildOffsetTable();
        }

        bool remove(const ResourceHandle<IPixelBus<TColor>>& bus)
        {
            if (bus == nullptr)
            {
                return false;
            }

            return _removeByPointer(bus.operator->());
        }

        bool remove(IPixelBus<TColor>& bus)
        {
            return _removeByPointer(&bus);
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
                               [](const ResourceHandle<IPixelBus<TColor>> &b)
                               { return b->canShow(); });
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
        std::vector<ResourceHandle<IPixelBus<TColor>>> _buses;

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
                                std::distance(_offsets.begin(), it)) -
                            1;
            size_t localIdx = globalIdx - _offsets[busIdx];

            return ResolvedPixel{busIdx, localIdx};
        }

        bool _removeByPointer(const IPixelBus<TColor>* bus)
        {
            auto before = _buses.size();

            _buses.erase(
                std::remove_if(
                    _buses.begin(),
                    _buses.end(),
                    [bus](const ResourceHandle<IPixelBus<TColor>>& item)
                    {
                        return item.operator->() == bus;
                    }),
                _buses.end());

            if (_buses.size() == before)
            {
                return false;
            }

            _buildOffsetTable();
            return true;
        }
    };

    template <typename TColor, typename... TBuses>
        requires (std::convertible_to<std::remove_reference_t<TBuses>*, IPixelBus<TColor>*> && ...)
    class ConcatBusStateT
    {
    public:
        explicit ConcatBusStateT(TBuses&&... buses)
            : _owned(std::forward<TBuses>(buses)...)
            , _borrowedBuses(_buildBorrowedBuses(_owned))
        {
        }

        std::vector<ResourceHandle<IPixelBus<TColor>>> takeBorrowedBuses()
        {
            return std::move(_borrowedBuses);
        }

    private:
        static std::vector<ResourceHandle<IPixelBus<TColor>>> _buildBorrowedBuses(std::tuple<std::remove_reference_t<TBuses>...>& owned)
        {
            std::vector<ResourceHandle<IPixelBus<TColor>>> borrowedBuses;
            borrowedBuses.reserve(sizeof...(TBuses));

            std::apply(
                [&](auto&... bus)
                {
                    (borrowedBuses.emplace_back(bus), ...);
                },
                owned);

            return borrowedBuses;
        }

        std::tuple<std::remove_reference_t<TBuses>...> _owned;
        std::vector<ResourceHandle<IPixelBus<TColor>>> _borrowedBuses;
    };

    template <typename TColor, typename... TBuses>
        requires (std::convertible_to<std::remove_reference_t<TBuses>*, IPixelBus<TColor>*> && ...)
    class OwningConcatBusT
        : private ConcatBusStateT<TColor, TBuses...>
        , public ConcatBus<TColor>
    {
    public:
        using StateType = ConcatBusStateT<TColor, TBuses...>;
        using BusType = ConcatBus<TColor>;

        explicit OwningConcatBusT(TBuses&&... buses)
            : StateType(std::forward<TBuses>(buses)...)
            , BusType(static_cast<StateType&>(*this).takeBorrowedBuses())
        {
        }
    };

    template <typename TColor, typename... TBuses>
    using OwningConcatBus = OwningConcatBusT<TColor, TBuses...>;

    template <typename TColor, typename... TBuses>
        requires (std::convertible_to<std::remove_reference_t<TBuses>*, IPixelBus<TColor>*> && ...)
    auto makeOwningConcatBus(TBuses&&... buses)
    {
        return OwningConcatBusT<TColor, TBuses...>(std::forward<TBuses>(buses)...);
    }

    template <typename TFirstBus, typename... TRestBuses>
    auto makeOwningConcatBus(TFirstBus&& firstBus,
                             TRestBuses&&... restBuses)
    {
        using TColor = decltype(_deduceBusColor(
            static_cast<std::remove_reference_t<TFirstBus>*>(nullptr)));

        static_assert(
            std::convertible_to<std::remove_reference_t<TFirstBus>*, IPixelBus<TColor>*> &&
            (std::convertible_to<std::remove_reference_t<TRestBuses>*, IPixelBus<TColor>*> && ...),
            "All buses passed to makeOwningConcatBus must share the same color type.");

        return OwningConcatBusT<TColor, TFirstBus, TRestBuses...>(
            std::forward<TFirstBus>(firstBus),
            std::forward<TRestBuses>(restBuses)...);
    }

} // namespace npb
