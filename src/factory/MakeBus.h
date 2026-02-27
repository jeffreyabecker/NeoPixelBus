#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

#include "buses/BusDriver.h"
#include "core/Compat.h"
#include "factory/Traits.h"

namespace npb
{
namespace factory
{

    template <typename TProtocolDesc,
              typename TProtocol,
              typename = void>
    struct ProtocolDescriptorCapabilityRequirement
    {
        using Type = typename TProtocol::TransportCategory;
    };

    template <typename TProtocolDesc,
              typename TProtocol>
    struct ProtocolDescriptorCapabilityRequirement<TProtocolDesc,
                                                  TProtocol,
                                                  std::void_t<typename TProtocolDesc::CapabilityRequirement>>
    {
        using Type = typename TProtocolDesc::CapabilityRequirement;
    };

    template <typename TTransportDesc,
              typename TTransport,
              typename = void>
    struct TransportDescriptorCapability
    {
        using Type = typename TTransport::TransportCategory;
    };

    template <typename TTransportDesc,
              typename TTransport>
    struct TransportDescriptorCapability<TTransportDesc,
                                        TTransport,
                                        std::void_t<typename TTransportDesc::Capability>>
    {
        using Type = typename TTransportDesc::Capability;
    };

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocol,
              typename TTransport>
    static constexpr bool DescriptorCapabilityCompatible =
        TransportCategoryCompatible<typename ProtocolDescriptorCapabilityRequirement<TProtocolDesc, TProtocol>::Type,
                                    typename TransportDescriptorCapability<TTransportDesc, TTransport>::Type>;

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
              typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
              typename TProtocol = typename TProtocolTraits::ProtocolType,
              typename TTransport = typename TTransportTraits::TransportType,
              typename TProtocolSettings = typename TProtocolTraits::SettingsType,
              typename TTransportSettings = typename TTransportTraits::SettingsType,
              typename TProtocolConfig,
              typename TTransportConfig,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TTransport> &&
                                          DescriptorCapabilityCompatible<TProtocolDesc, TTransportDesc, TProtocol, TTransport> &&
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport> &&
                                          std::is_same<typename TProtocolTraits::ColorType, typename TProtocol::ColorType>::value &&
                                          std::is_convertible<npb::remove_cvref_t<decltype(resolveProtocolSettings<TProtocolDesc>(std::declval<TProtocolConfig>()))>,
                                                              TProtocolSettings>::value &&
                                          std::is_convertible<npb::remove_cvref_t<decltype(resolveTransportSettings<TTransportDesc>(std::declval<TTransportConfig>()))>,
                                                              TTransportSettings>::value>>
    StaticBusDriverPixelBusT<TTransport, TProtocol> makeBus(uint16_t pixelCount,
                                                             TProtocolConfig &&protocolConfig,
                                                             TTransportConfig &&transportConfig)
    {
        auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(std::forward<TProtocolConfig>(protocolConfig));
        auto transportSettings = resolveTransportSettings<TTransportDesc>(std::forward<TTransportConfig>(transportConfig));

        return makeStaticDriverPixelBus<TTransport, TProtocol>(pixelCount,
                                                                std::move(transportSettings),
                                                                std::move(protocolSettings));
    }

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
              typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
              typename TProtocol = typename TProtocolTraits::ProtocolType,
              typename TTransport = typename TTransportTraits::TransportType,
              typename TProtocolSettings = typename TProtocolTraits::SettingsType,
              typename TTransportSettings = typename TTransportTraits::SettingsType,
              typename TTransportConfig,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TTransport> &&
                                          DescriptorCapabilityCompatible<TProtocolDesc, TTransportDesc, TProtocol, TTransport> &&
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport> &&
                                          std::is_same<typename TProtocolTraits::ColorType, typename TProtocol::ColorType>::value &&
                                          std::is_convertible<npb::remove_cvref_t<decltype(resolveTransportSettings<TTransportDesc>(std::declval<TTransportConfig>()))>,
                                                              TTransportSettings>::value>>
    StaticBusDriverPixelBusT<TTransport, TProtocol> makeBus(uint16_t pixelCount,
                                                             TTransportConfig &&transportConfig)
    {
        auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(TProtocolSettings{});
        auto transportSettings = resolveTransportSettings<TTransportDesc>(std::forward<TTransportConfig>(transportConfig));

        return makeStaticDriverPixelBus<TTransport, TProtocol>(pixelCount,
                                                                std::move(transportSettings),
                                                                std::move(protocolSettings));
    }

} // namespace factory
} // namespace npb
