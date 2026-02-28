#pragma once

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "buses/ConcatBus.h"
#include "core/Compat.h"

namespace npb
{
namespace factory
{

    template <typename TBus>
    using BusColorType = decltype(_deduceBusColor(static_cast<npb::remove_cvref_t<TBus> *>(nullptr)));

    template <typename TColor,
              typename... TBuses>
    class StaticConcatBusT : public IPixelBus<TColor>
    {
    public:
        static_assert(ConcatBusCompatibleBuses<TColor, TBuses...>,
                      "All owned buses must be compatible with IPixelBus<TColor>");

        using OwnedTuple = std::tuple<npb::remove_cvref_t<TBuses>...>;

        explicit StaticConcatBusT(TBuses &&...buses)
            : _ownedBuses(std::forward<TBuses>(buses)...)
            , _busList(makeBusList(_ownedBuses))
            , _concat(std::vector<IPixelBus<TColor> *>{_busList})
        {
        }

        StaticConcatBusT(const StaticConcatBusT &) = delete;
        StaticConcatBusT &operator=(const StaticConcatBusT &) = delete;
        StaticConcatBusT(StaticConcatBusT &&) = delete;
        StaticConcatBusT &operator=(StaticConcatBusT &&) = delete;

        void begin() override
        {
            _concat.begin();
        }

        void show() override
        {
            _concat.show();
        }

        bool canShow() const override
        {
            return _concat.canShow();
        }

        size_t pixelCount() const override
        {
            return _concat.pixelCount();
        }

        void setPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) override
        {
            _concat.setPixelColors(offset, first, last);
        }

        void getPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) const override
        {
            _concat.getPixelColors(offset, first, last);
        }

    private:
        static std::vector<IPixelBus<TColor> *> makeBusList(OwnedTuple &ownedBuses)
        {
            std::vector<IPixelBus<TColor> *> buses{};
            buses.reserve(sizeof...(TBuses));
            std::apply(
                [&](auto &...bus)
                {
                    (buses.emplace_back(&bus), ...);
                },
                ownedBuses);
            return buses;
        }

        OwnedTuple _ownedBuses;
        std::vector<IPixelBus<TColor> *> _busList;
        ConcatBus<TColor> _concat;
    };

    template <typename TColor>
    using HeapConcatBusT = npb::HeapConcatBusT<TColor>;

    template <typename TFirstBus,
              typename... TOtherBuses,
              typename TColor = BusColorType<TFirstBus>,
              typename = std::enable_if_t<std::is_convertible<npb::remove_cvref_t<TFirstBus> *, IPixelBus<TColor> *>::value &&
                                          std::conjunction<std::is_convertible<npb::remove_cvref_t<TOtherBuses> *, IPixelBus<TColor> *>...>::value>>
    auto makeStaticConcatBus(TFirstBus &&firstBus,
                             TOtherBuses &&...otherBuses)
        -> StaticConcatBusT<TColor, TFirstBus, TOtherBuses...>
    {
        return StaticConcatBusT<TColor, TFirstBus, TOtherBuses...>{
            std::forward<TFirstBus>(firstBus),
            std::forward<TOtherBuses>(otherBuses)...};
    }

    template <typename TColor>
    HeapConcatBusT<TColor> makeHeapConcatBus(std::vector<std::unique_ptr<IPixelBus<TColor>>> buses)
    {
        return HeapConcatBusT<TColor>{std::move(buses)};
    }

} // namespace factory
} // namespace npb
