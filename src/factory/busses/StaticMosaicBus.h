#pragma once

#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "buses/MosaicBus.h"
#include "core/Compat.h"

namespace lw
{
namespace factory
{

    template <typename TBus>
    using MosaicBusColorType = decltype(_deduceBusColor(static_cast<lw::remove_cvref_t<TBus> *>(nullptr)));

    template <typename TColor,
              typename... TBuses>
    class StaticMosaicBusT : public I2dPixelBus<TColor>
    {
    public:
        static_assert(MosaicBusCompatibleBuses<TColor, TBuses...>,
                      "All owned buses must be compatible with IAssignableBufferBus<TColor>");

        using OwnedTuple = std::tuple<lw::remove_cvref_t<TBuses>...>;

        StaticMosaicBusT(MosaicBusSettings config,
                         TBuses &&...buses)
            : _ownedBuses(std::forward<TBuses>(buses)...)
            , _busList(makeBusList(_ownedBuses))
            , _mosaic(config,
                      std::vector<IAssignableBufferBus<TColor> *>{_busList},
                      BufferHolder<TColor>{static_cast<size_t>(config.panelWidth) *
                                               config.panelHeight *
                                               config.tilesWide *
                                               config.tilesHigh,
                                           nullptr,
                                           true})
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

        span<TColor> pixelBuffer() override
        {
            return _mosaic.pixelBuffer();
        }

        span<const TColor> pixelBuffer() const override
        {
            return _mosaic.pixelBuffer();
        }

        size_t pixelCount() const
        {
            return _mosaic.pixelCount();
        }

        const Topology &topology() const override
        {
            return _mosaic.topology();
        }

    private:
        static std::vector<IAssignableBufferBus<TColor> *> makeBusList(OwnedTuple &ownedBuses)
        {
            std::vector<IAssignableBufferBus<TColor> *> buses{};
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
        std::vector<IAssignableBufferBus<TColor> *> _busList;
        MosaicBus<TColor> _mosaic;
    };

    template <typename TFirstBus,
              typename... TOtherBuses,
              typename TColor = MosaicBusColorType<TFirstBus>,
              typename = std::enable_if_t<std::is_convertible<lw::remove_cvref_t<TFirstBus> *, IAssignableBufferBus<TColor> *>::value &&
                                          std::conjunction<std::is_convertible<lw::remove_cvref_t<TOtherBuses> *, IAssignableBufferBus<TColor> *>...>::value>>
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
} // namespace lw
