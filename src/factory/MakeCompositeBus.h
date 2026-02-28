#pragma once

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "buses/ConcatBus.h"
#include "buses/MosaicBus.h"
#include "core/Compat.h"
#include "factory/busses/StaticConcatBus.h"
#include "factory/busses/StaticMosaicBus.h"

namespace npb
{
namespace factory
{

    template <typename TFirstBus,
              typename TSecondBus,
              typename... TOtherBuses,
              typename TColor = BusColorType<TFirstBus>,
              typename = std::enable_if_t<std::is_convertible<npb::remove_cvref_t<TFirstBus> *, IPixelBus<TColor> *>::value &&
                                          std::is_convertible<npb::remove_cvref_t<TSecondBus> *, IPixelBus<TColor> *>::value &&
                                          std::conjunction<std::is_convertible<npb::remove_cvref_t<TOtherBuses> *, IPixelBus<TColor> *>...>::value>>
    ConcatBus<TColor> concatBus(TFirstBus &firstBus,
                                TSecondBus &secondBus,
                                TOtherBuses &...otherBuses)
    {
        return ConcatBus<TColor>{firstBus, secondBus, otherBuses...};
    }

    template <typename TColor>
    ConcatBus<TColor> concatBus(std::vector<IPixelBus<TColor> *> buses)
    {
        return ConcatBus<TColor>{std::move(buses)};
    }

    template <typename TFirstBus,
              typename TSecondBus,
              typename... TOtherBuses,
              typename = std::enable_if_t<std::is_convertible<npb::remove_cvref_t<TFirstBus> *, IPixelBus<BusColorType<TFirstBus>> *>::value &&
                                          std::is_convertible<npb::remove_cvref_t<TSecondBus> *, IPixelBus<BusColorType<TFirstBus>> *>::value &&
                                          std::conjunction<std::is_convertible<npb::remove_cvref_t<TOtherBuses> *, IPixelBus<BusColorType<TFirstBus>> *>...>::value>>
    auto makeBus(TFirstBus &firstBus,
                 TSecondBus &secondBus,
                 TOtherBuses &...otherBuses)
        -> ConcatBus<BusColorType<TFirstBus>>
    {
        return concatBus(firstBus, secondBus, otherBuses...);
    }

    template <typename TColor>
    MosaicBus<TColor> makeMosaicBus(MosaicBusSettings<TColor> config,
                                    std::vector<IPixelBus<TColor> *> buses)
    {
        return MosaicBus<TColor>{std::move(config), std::move(buses)};
    }

    template <typename TColor,
              typename... TBuses,
              typename = std::enable_if_t<MosaicBusCompatibleBuses<TColor, TBuses...>>>
    MosaicBus<TColor> makeMosaicBus(MosaicBusSettings<TColor> config,
                                    TBuses &...buses)
    {
        std::vector<IPixelBus<TColor> *> busList{};
        busList.reserve(sizeof...(buses));
        (busList.emplace_back(&buses), ...);
        return MosaicBus<TColor>{std::move(config), std::move(busList)};
    }

} // namespace factory
} // namespace npb
