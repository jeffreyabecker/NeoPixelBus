#pragma once

#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "buses/MosaicBus.h"
#include "core/Compat.h"

namespace npb
{
namespace factory
{

    template <typename TBus>
    using MosaicBusColorType = decltype(_deduceBusColor(static_cast<npb::remove_cvref_t<TBus> *>(nullptr)));

    template <typename TColor,
              typename... TBuses>
    class StaticMosaicBusT : public I2dPixelBus<TColor>
    {
    public:
        static_assert(MosaicBusCompatibleBuses<TColor, TBuses...>,
                      "All owned buses must be compatible with IPixelBus<TColor>");

        using OwnedTuple = std::tuple<npb::remove_cvref_t<TBuses>...>;

        using IPixelBus<TColor>::setPixelColor;
        using IPixelBus<TColor>::getPixelColor;

        StaticMosaicBusT(MosaicBusSettings config,
                         TBuses &&...buses)
            : _ownedBuses(std::forward<TBuses>(buses)...)
            , _busList(makeBusList(_ownedBuses))
            , _mosaic(std::move(config), std::vector<IPixelBus<TColor> *>{_busList})
        {
        }

        StaticMosaicBusT(const StaticMosaicBusT &) = delete;
        StaticMosaicBusT &operator=(const StaticMosaicBusT &) = delete;
        StaticMosaicBusT(StaticMosaicBusT &&) = delete;
        StaticMosaicBusT &operator=(StaticMosaicBusT &&) = delete;

        void begin() override
        {
            _mosaic.begin();
        }

        void show() override
        {
            _mosaic.show();
        }

        bool canShow() const override
        {
            return _mosaic.canShow();
        }

        size_t pixelCount() const override
        {
            return _mosaic.pixelCount();
        }

        void setPixelColor(int16_t x, int16_t y, const TColor &color) override
        {
            _mosaic.setPixelColor(x, y, color);
        }

        TColor getPixelColor(int16_t x, int16_t y) const override
        {
            return _mosaic.getPixelColor(x, y);
        }

        uint16_t width() const override
        {
            return _mosaic.width();
        }

        uint16_t height() const override
        {
            return _mosaic.height();
        }

        void setPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) override
        {
            _mosaic.setPixelColors(offset, first, last);
        }

        void getPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) const override
        {
            _mosaic.getPixelColors(offset, first, last);
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
        MosaicBus<TColor> _mosaic;
    };

    template <typename TFirstBus,
              typename... TOtherBuses,
              typename TColor = MosaicBusColorType<TFirstBus>,
              typename = std::enable_if_t<std::is_convertible<npb::remove_cvref_t<TFirstBus> *, IPixelBus<TColor> *>::value &&
                                          std::conjunction<std::is_convertible<npb::remove_cvref_t<TOtherBuses> *, IPixelBus<TColor> *>...>::value>>
    auto makeStaticMosaicBus(MosaicBusSettings config,
                             TFirstBus &&firstBus,
                             TOtherBuses &&...otherBuses)
        -> StaticMosaicBusT<TColor, TFirstBus, TOtherBuses...>
    {
        return StaticMosaicBusT<TColor, TFirstBus, TOtherBuses...>{std::move(config),
                                                                    std::forward<TFirstBus>(firstBus),
                                                                    std::forward<TOtherBuses>(otherBuses)...};
    }

} // namespace factory
} // namespace npb
