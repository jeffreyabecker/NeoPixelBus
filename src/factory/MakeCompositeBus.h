#pragma once

#include <type_traits>
#include <tuple>
#include <utility>
#include <initializer_list>

#include "factory/busses/StaticBus.h"
#include "colors/NilShader.h"
#include "core/Compat.h"
#include "core/IPixelBus.h"

namespace lw
{
namespace factory
{

    template <typename TFirstBus,
              typename... TOtherBuses>
    using CompositeBus = StaticBus<typename lw::remove_cvref_t<TFirstBus>::ColorType>;

    template <typename TBus>
    using BusColorType = typename lw::remove_cvref_t<TBus>::ColorType;

    template <typename TBus,
              typename = void>
    struct FactoryBusStrandExtractable : std::false_type
    {
    };

    template <typename TBus>
    struct FactoryBusStrandExtractable<TBus,
                                       std::void_t<typename lw::remove_cvref_t<TBus>::OwnedTuple,
                                                   decltype(std::declval<lw::remove_cvref_t<TBus> &>().pixelCount())>> : std::true_type
    {
    };

    template <typename TColor,
              typename TBus>
    auto makeOwningStrandTuple(TBus &&bus,
                               uint16_t strandLength)
    {
        using BusType = lw::remove_cvref_t<TBus>;
        using OwnedTupleType = typename BusType::OwnedTuple;
        using ProtocolType = typename std::tuple_element<0, OwnedTupleType>::type;
        using TransportType = typename std::tuple_element<1, OwnedTupleType>::type;

        return std::tuple<ProtocolType, TransportType, NilShader<TColor>, uint16_t>{
            ProtocolType{std::move(bus.protocol())},
            TransportType{std::move(bus.transport())},
            NilShader<TColor>{},
            strandLength};
    }

    template <typename TColor,
              typename... TBuses>
    auto makeStaticBusFromFactoryBuses(size_t rootPixelCount,
                                       size_t shaderPixelCount,
                                       Topology topology,
                                       TBuses &&...buses)
    {
        auto strandTuple = std::tuple_cat(makeOwningStrandTuple<TColor>(std::forward<TBuses>(buses),
                                                                        static_cast<uint16_t>(buses.pixelCount()))...);

        return std::apply(
            [&](auto &&...args)
            {
                return makeStaticBus<TColor>(rootPixelCount,
                                             shaderPixelCount,
                                             std::move(topology),
                                             std::forward<decltype(args)>(args)...);
            },
            std::move(strandTuple));
    }

    template <typename TFirstBus,
              typename... TOtherBuses,
              typename = std::enable_if_t<std::is_convertible<lw::remove_cvref_t<TFirstBus> *, IPixelBus<BusColorType<TFirstBus>> *>::value &&
                                          std::conjunction<std::is_convertible<lw::remove_cvref_t<TOtherBuses> *, IPixelBus<BusColorType<TFirstBus>> *>...>::value &&
                                          FactoryBusStrandExtractable<TFirstBus>::value &&
                                          std::conjunction<FactoryBusStrandExtractable<TOtherBuses>...>::value &&
                                          std::is_rvalue_reference<TFirstBus &&>::value &&
                                          std::conjunction<std::is_rvalue_reference<TOtherBuses &&>...>::value>>
    auto makeBus(TFirstBus &&firstBus,
                 TOtherBuses &&...otherBuses)
    {
        using TColor = BusColorType<TFirstBus>;

        size_t pixelCount = static_cast<size_t>(firstBus.pixelCount());
        ((pixelCount += static_cast<size_t>(otherBuses.pixelCount())), ...);

        return makeStaticBusFromFactoryBuses<TColor>(pixelCount,
                                 0,
                                 Topology::linear(pixelCount),
                                 std::forward<TFirstBus>(firstBus),
                                 std::forward<TOtherBuses>(otherBuses)...);
    }

    template <typename TFirstBus,
              typename... TOtherBuses,
              typename = std::enable_if_t<std::is_convertible<lw::remove_cvref_t<TFirstBus> *, IPixelBus<BusColorType<TFirstBus>> *>::value &&
                                          std::conjunction<std::is_convertible<lw::remove_cvref_t<TOtherBuses> *, IPixelBus<BusColorType<TFirstBus>> *>...>::value &&
                                          FactoryBusStrandExtractable<TFirstBus>::value &&
                                          std::conjunction<FactoryBusStrandExtractable<TOtherBuses>...>::value &&
                                          std::is_rvalue_reference<TFirstBus &&>::value &&
                                          std::conjunction<std::is_rvalue_reference<TOtherBuses &&>...>::value>>
    auto makeBus(TopologySettings config,
                 TFirstBus &&firstBus,
                 TOtherBuses &&...otherBuses)
    {
        using TColor = BusColorType<TFirstBus>;

        const size_t pixelCount = static_cast<size_t>(config.panelWidth) *
                                  config.panelHeight *
                                  config.tilesWide *
                                  config.tilesHigh;

        return makeStaticBusFromFactoryBuses<TColor>(pixelCount,
                                 0,
                                 Topology{config},
                                 std::forward<TFirstBus>(firstBus),
                                 std::forward<TOtherBuses>(otherBuses)...);
    }

} // namespace factory
} // namespace lw
