#pragma once

#include <type_traits>
#include <utility>
#include <vector>
#include <initializer_list>

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
              typename = std::enable_if_t<std::is_convertible<npb::remove_cvref_t<TFirstBus> *, IAssignableBufferBus<TColor> *>::value &&
                                          std::is_convertible<npb::remove_cvref_t<TSecondBus> *, IAssignableBufferBus<TColor> *>::value &&
                                          std::conjunction<std::is_convertible<npb::remove_cvref_t<TOtherBuses> *, IAssignableBufferBus<TColor> *>...>::value>>
    ConcatBus<TColor> concatBus(TFirstBus &firstBus,
                                TSecondBus &secondBus,
                                TOtherBuses &...otherBuses)
    {
        std::vector<IAssignableBufferBus<TColor> *> busList{};
        busList.reserve(2 + sizeof...(otherBuses));
        busList.emplace_back(&firstBus);
        busList.emplace_back(&secondBus);
        (busList.emplace_back(&otherBuses), ...);

        size_t pixelCount = 0;
        for (auto* bus : busList)
        {
            if (bus != nullptr)
            {
                pixelCount += bus->pixelBuffer().size();
            }
        }

        return ConcatBus<TColor>{std::move(busList), BufferHolder<TColor>{pixelCount, nullptr, true}};
    }

    template <typename TColor>
    ConcatBus<TColor> concatBus(std::vector<IAssignableBufferBus<TColor> *> buses)
    {
        size_t pixelCount = 0;
        for (auto* bus : buses)
        {
            if (bus != nullptr)
            {
                pixelCount += bus->pixelBuffer().size();
            }
        }

        return ConcatBus<TColor>{std::move(buses), BufferHolder<TColor>{pixelCount, nullptr, true}};
    }

    template <typename TFirstBus,
              typename... TOtherBuses,
              typename = std::enable_if_t<std::is_convertible<npb::remove_cvref_t<TFirstBus> *, IPixelBus<BusColorType<TFirstBus>> *>::value &&
                                          std::conjunction<std::is_convertible<npb::remove_cvref_t<TOtherBuses> *, IPixelBus<BusColorType<TFirstBus>> *>...>::value>>
    auto makeBus(std::initializer_list<uint16_t> segmentLengths,
                 TFirstBus &&firstBus,
                 TOtherBuses &&...otherBuses)
        -> RootOwnedConcatBusT<BusColorType<TFirstBus>, TFirstBus, TOtherBuses...>
    {
        return makeRootOwnedConcatBus(std::move(segmentLengths),
                                      std::forward<TFirstBus>(firstBus),
                                      std::forward<TOtherBuses>(otherBuses)...);
    }

    template <typename TFirstBus,
              typename TSecondBus,
              typename... TOtherBuses,
              typename = std::enable_if_t<std::is_convertible<npb::remove_cvref_t<TFirstBus> *, IAssignableBufferBus<BusColorType<TFirstBus>> *>::value &&
                                          std::is_convertible<npb::remove_cvref_t<TSecondBus> *, IAssignableBufferBus<BusColorType<TFirstBus>> *>::value &&
                                          std::conjunction<std::is_convertible<npb::remove_cvref_t<TOtherBuses> *, IAssignableBufferBus<BusColorType<TFirstBus>> *>...>::value>>
    auto makeBus(TFirstBus &firstBus,
                 TSecondBus &secondBus,
                 TOtherBuses &...otherBuses)
        -> ConcatBus<BusColorType<TFirstBus>>
    {
        return concatBus(firstBus, secondBus, otherBuses...);
    }

    template <typename TColor>
    MosaicBus<TColor> makeBus(MosaicBusSettings config,
                              std::vector<IAssignableBufferBus<TColor> *> buses)
    {
        const size_t pixelCount = static_cast<size_t>(config.panelWidth) *
                                  config.panelHeight *
                                  config.tilesWide *
                                  config.tilesHigh;
        return MosaicBus<TColor>{std::move(config),
                                 std::move(buses),
                                 BufferHolder<TColor>{pixelCount, nullptr, true}};
    }

    template <typename TFirstBus,
              typename... TOtherBuses,
              typename TColor = BusColorType<TFirstBus>,
              typename = std::enable_if_t<std::is_convertible<npb::remove_cvref_t<TFirstBus> *, IAssignableBufferBus<TColor> *>::value &&
                                          std::conjunction<std::is_convertible<npb::remove_cvref_t<TOtherBuses> *, IAssignableBufferBus<TColor> *>...>::value>>
    MosaicBus<TColor> makeBus(MosaicBusSettings config,
                              TFirstBus &firstBus,
                              TOtherBuses &...otherBuses)
    {
        std::vector<IAssignableBufferBus<TColor> *> busList{};
        busList.reserve(1 + sizeof...(otherBuses));
        busList.emplace_back(&firstBus);
        (busList.emplace_back(&otherBuses), ...);
        const size_t pixelCount = static_cast<size_t>(config.panelWidth) *
                                  config.panelHeight *
                                  config.tilesWide *
                                  config.tilesHigh;
        return MosaicBus<TColor>{std::move(config),
                                 std::move(busList),
                                 BufferHolder<TColor>{pixelCount, nullptr, true}};
    }

} // namespace factory
} // namespace npb
