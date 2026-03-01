#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

#include "factory/busses/BusDriverConstraints.h"
#include "buses/OwningUnifiedPixelBus.h"
#include "colors/NilShader.h"
#include "core/Compat.h"
#include "factory/Traits.h"
#include "transports/OneWireWrapper.h"

namespace lw
{
namespace factory
{

    template <typename TColor,
              typename... TArgs>
    auto makeBus(BufferHolder<TColor> rootBuffer,
                 BufferHolder<TColor> shaderBuffer,
                 BufferHolder<uint8_t> protocolBuffer,
                 Topology topology,
                 TArgs &&...args)
        -> StaticOwningBus<TColor, lw::remove_cvref_t<TArgs>...>
    {
        return makeStaticOwningBus<TColor>(std::move(rootBuffer),
                                           std::move(shaderBuffer),
                                           std::move(protocolBuffer),
                                           std::move(topology),
                                           std::forward<TArgs>(args)...);
    }

    template <typename TColor>
    DynamicOwningBus<TColor> makeBus(BufferHolder<TColor> rootBuffer,
                                     BufferHolder<TColor> shaderBuffer,
                                     BufferHolder<uint8_t> protocolBuffer,
                                     Topology topology,
                                     std::vector<StrandExtent<TColor>> strands)
    {
        return DynamicOwningBus<TColor>{std::move(rootBuffer),
                                        std::move(shaderBuffer),
                                        std::move(protocolBuffer),
                                        std::move(topology),
                                        std::move(strands)};
    }

    template <typename TProtocol,
              typename TTransport>
    TProtocol makeOwningBusProtocol(uint16_t pixelCount,
                                    TTransport& transport,
                                    typename TProtocol::SettingsType settings)
    {
        if constexpr (ProtocolSettingsTransportBindable<TProtocol>)
        {
            settings.bus = &transport;
            return TProtocol(pixelCount, std::move(settings));
        }
        else if constexpr (std::is_constructible<TProtocol,
                                                 uint16_t,
                                                 typename TProtocol::SettingsType,
                                                 TTransport &>::value)
        {
            return TProtocol(pixelCount, std::move(settings), transport);
        }
        else
        {
            return TProtocol(pixelCount, std::move(settings));
        }
    }

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
              typename TProtocol,
              typename TTransport>
    static constexpr bool DescriptorWrappedOneWireCapabilityCompatible =
        TransportCategoryCompatible<typename ProtocolDescriptorCapabilityRequirement<TProtocolDesc, TProtocol>::Type,
                                    OneWireTransportTag> &&
        TransportCategoryCompatible<typename TransportDescriptorCapability<TTransportDesc, TTransport>::Type,
                                    TransportTag>;

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
              typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
              typename TProtocol = typename TProtocolTraits::ProtocolType,
              typename TTransport = typename TTransportTraits::TransportType,
              bool TDirectCompatible = BusDriverProtocolTransportCompatible<TProtocol, TTransport> &&
                                       DescriptorCapabilityCompatible<TProtocolDesc, TTransportDesc, TProtocol, TTransport>,
              bool TWrappedCompatible = BusDriverProtocolTransportCompatible<TProtocol, OneWireWrapper<TTransport>> &&
                                        DescriptorWrappedOneWireCapabilityCompatible<TProtocolDesc, TTransportDesc, TProtocol, TTransport>>
    struct BusTypeResolver;

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocolTraits,
              typename TTransportTraits,
              typename TProtocol,
              typename TTransport,
              bool TWrappedCompatible>
    struct BusTypeResolver<TProtocolDesc,
                           TTransportDesc,
                           TProtocolTraits,
                           TTransportTraits,
                           TProtocol,
                           TTransport,
                           true,
                           TWrappedCompatible>
    {
        using Type = StaticOwningBus<typename TProtocol::ColorType,
                                     TProtocol,
                                     TTransport,
                                     NilShader<typename TProtocol::ColorType>,
                                     uint16_t>;
    };

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocolTraits,
              typename TTransportTraits,
              typename TProtocol,
              typename TTransport>
    struct BusTypeResolver<TProtocolDesc,
                           TTransportDesc,
                           TProtocolTraits,
                           TTransportTraits,
                           TProtocol,
                           TTransport,
                           false,
                           true>
    {
        using Type = StaticOwningBus<typename TProtocol::ColorType,
                                     TProtocol,
                                     OneWireWrapper<TTransport>,
                                     NilShader<typename TProtocol::ColorType>,
                                     uint16_t>;
    };

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocolTraits,
              typename TTransportTraits,
              typename TProtocol,
              typename TTransport>
    struct BusTypeResolver<TProtocolDesc,
                           TTransportDesc,
                           TProtocolTraits,
                           TTransportTraits,
                           TProtocol,
                           TTransport,
                           false,
                           false>
    {
        static_assert(DescriptorCapabilityCompatible<TProtocolDesc, TTransportDesc, TProtocol, TTransport> ||
                          DescriptorWrappedOneWireCapabilityCompatible<TProtocolDesc, TTransportDesc, TProtocol, TTransport>,
                      "Protocol and transport descriptors are not compatible for Bus alias");
    };

    template <typename TProtocolDesc,
              typename TTransportDesc = descriptors::PlatformDefault,
              typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
              typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
              typename TProtocol = typename TProtocolTraits::ProtocolType,
              typename TTransport = typename TTransportTraits::TransportType>
    using Bus = typename BusTypeResolver<TProtocolDesc,
                                         TTransportDesc,
                                         TProtocolTraits,
                                         TTransportTraits,
                                         TProtocol,
                                         TTransport>::Type;

    template <typename TTransportSettings>
    OneWireWrapperSettings<TTransportSettings> makeOneWireWrapperSettings(TTransportSettings settings,
                                                                          OneWireTiming timing)
    {
        OneWireWrapperSettings<TTransportSettings> wrapperSettings{};
        static_cast<TTransportSettings &>(wrapperSettings) = std::move(settings);
        wrapperSettings.timing = timing;
        return wrapperSettings;
    }

    template <typename TProtocolSettings, typename = void>
    struct ProtocolSettingsHasTiming : std::false_type
    {
    };

    template <typename TProtocolSettings>
    struct ProtocolSettingsHasTiming<TProtocolSettings,
                                     std::void_t<decltype(std::declval<TProtocolSettings &>().timing)>>
        : std::true_type
    {
    };

    template <typename TProtocolSettings>
    void assignProtocolTimingIfPresent(TProtocolSettings &settings,
                                       OneWireTiming timing)
    {
        if constexpr (ProtocolSettingsHasTiming<TProtocolSettings>::value)
        {
            settings.timing = timing;
        }
    }

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
                                          std::is_convertible<lw::remove_cvref_t<decltype(resolveProtocolSettings<TProtocolDesc>(std::declval<TProtocolConfig>()))>,
                                                              TProtocolSettings>::value &&
                                          std::is_convertible<lw::remove_cvref_t<decltype(resolveTransportSettings<TTransportDesc>(std::declval<uint16_t>(),
                                                                                                                                    std::declval<TTransportConfig>()))>,
                                                              TTransportSettings>::value>>
    StaticOwningBus<typename TProtocol::ColorType,
                    TProtocol,
                    TTransport,
                    NilShader<typename TProtocol::ColorType>,
                    uint16_t> makeBus(uint16_t pixelCount,
                                      TProtocolConfig &&protocolConfig,
                                      TTransportConfig &&transportConfig)
    {
        auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(std::forward<TProtocolConfig>(protocolConfig));
        auto transportSettings = resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                         protocolSettings,
                                                         std::forward<TTransportConfig>(transportConfig));

        TTransport transport{std::move(transportSettings)};
        TProtocol protocol = makeOwningBusProtocol<TProtocol, TTransport>(pixelCount,
                                                                          transport,
                                                                          std::move(protocolSettings));
        const size_t protocolBufferSize = protocol.requiredBufferSizeBytes();
        NilShader<typename TProtocol::ColorType> shader{};

        return makeStaticOwningBus<typename TProtocol::ColorType>(BufferHolder<typename TProtocol::ColorType>{pixelCount, nullptr, true},
                                                                   BufferHolder<typename TProtocol::ColorType>::nil(),
                                       BufferHolder<uint8_t>{protocolBufferSize, nullptr, true},
                                                                   Topology::linear(pixelCount),
                                                                   std::move(protocol),
                                                                   std::move(transport),
                                                                   std::move(shader),
                                                                   static_cast<uint16_t>(pixelCount));
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
                                          std::is_convertible<lw::remove_cvref_t<decltype(resolveTransportSettings<TTransportDesc>(std::declval<uint16_t>(),
                                                                                                                                    std::declval<TTransportConfig>()))>,
                                                              TTransportSettings>::value>>
    StaticOwningBus<typename TProtocol::ColorType,
                    TProtocol,
                    TTransport,
                    NilShader<typename TProtocol::ColorType>,
                    uint16_t> makeBus(uint16_t pixelCount,
                                      TTransportConfig &&transportConfig)
    {
        auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(TProtocolTraits::defaultSettings());
        auto transportSettings = resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                         protocolSettings,
                                                         std::forward<TTransportConfig>(transportConfig));

        TTransport transport{std::move(transportSettings)};
        TProtocol protocol = makeOwningBusProtocol<TProtocol, TTransport>(pixelCount,
                                                                          transport,
                                                                          std::move(protocolSettings));
        const size_t protocolBufferSize = protocol.requiredBufferSizeBytes();
        NilShader<typename TProtocol::ColorType> shader{};

        return makeStaticOwningBus<typename TProtocol::ColorType>(BufferHolder<typename TProtocol::ColorType>{pixelCount, nullptr, true},
                                                                   BufferHolder<typename TProtocol::ColorType>::nil(),
                                       BufferHolder<uint8_t>{protocolBufferSize, nullptr, true},
                                                                   Topology::linear(pixelCount),
                                                                   std::move(protocol),
                                                                   std::move(transport),
                                                                   std::move(shader),
                                                                   static_cast<uint16_t>(pixelCount));
    }

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
              typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
              typename TProtocol = typename TProtocolTraits::ProtocolType,
              typename TTransport = typename TTransportTraits::TransportType,
              typename TWrappedTransport = OneWireWrapper<TTransport>,
              typename TProtocolSettings = typename TProtocolTraits::SettingsType,
              typename TTransportSettings = typename TTransportTraits::SettingsType,
              typename TProtocolConfig,
              typename TTransportConfig,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TWrappedTransport> &&
                                          DescriptorWrappedOneWireCapabilityCompatible<TProtocolDesc, TTransportDesc, TProtocol, TTransport> &&
                                          BusDriverProtocolSettingsConstructible<TProtocol, TWrappedTransport> &&
                                          std::is_same<typename TProtocolTraits::ColorType, typename TProtocol::ColorType>::value &&
                                          std::is_convertible<lw::remove_cvref_t<decltype(resolveProtocolSettings<TProtocolDesc>(std::declval<TProtocolConfig>()))>,
                                                              TProtocolSettings>::value &&
                                          std::is_convertible<lw::remove_cvref_t<decltype(resolveTransportSettings<TTransportDesc>(std::declval<uint16_t>(),
                                                                                                                                    std::declval<const OneWireTiming *>(),
                                                                                                                                    std::declval<TTransportConfig>()))>,
                                                              TTransportSettings>::value>>
    StaticOwningBus<typename TProtocol::ColorType,
                    TProtocol,
                    TWrappedTransport,
                    NilShader<typename TProtocol::ColorType>,
                    uint16_t> makeBus(uint16_t pixelCount,
                                      TProtocolConfig &&protocolConfig,
                                      OneWireTiming timing,
                                      TTransportConfig &&transportConfig)
    {
        auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(std::forward<TProtocolConfig>(protocolConfig));
        assignProtocolTimingIfPresent(protocolSettings, timing);
        auto transportSettings = resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                         protocolSettings,
                                                         &timing,
                                                         std::forward<TTransportConfig>(transportConfig));
        auto wrapperSettings = makeOneWireWrapperSettings(std::move(transportSettings), timing);

                                TWrappedTransport transport{std::move(wrapperSettings)};
                                TProtocol protocol = makeOwningBusProtocol<TProtocol, TWrappedTransport>(pixelCount,
                                                                      transport,
                                                                      std::move(protocolSettings));
                                const size_t protocolBufferSize = protocol.requiredBufferSizeBytes();
                                NilShader<typename TProtocol::ColorType> shader{};

                                return makeStaticOwningBus<typename TProtocol::ColorType>(BufferHolder<typename TProtocol::ColorType>{pixelCount, nullptr, true},
                                                               BufferHolder<typename TProtocol::ColorType>::nil(),
                                                               BufferHolder<uint8_t>{protocolBufferSize, nullptr, true},
                                                               Topology::linear(pixelCount),
                                                               std::move(protocol),
                                                               std::move(transport),
                                                               std::move(shader),
                                                                   static_cast<uint16_t>(pixelCount));
    }

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
              typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
              typename TProtocol = typename TProtocolTraits::ProtocolType,
              typename TTransport = typename TTransportTraits::TransportType,
              typename TWrappedTransport = OneWireWrapper<TTransport>,
              typename TProtocolSettings = typename TProtocolTraits::SettingsType,
              typename TTransportSettings = typename TTransportTraits::SettingsType,
              typename TTransportConfig,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TWrappedTransport> &&
                                          DescriptorWrappedOneWireCapabilityCompatible<TProtocolDesc, TTransportDesc, TProtocol, TTransport> &&
                                          BusDriverProtocolSettingsConstructible<TProtocol, TWrappedTransport> &&
                                          std::is_same<typename TProtocolTraits::ColorType, typename TProtocol::ColorType>::value &&
                                          std::is_convertible<lw::remove_cvref_t<decltype(resolveTransportSettings<TTransportDesc>(std::declval<uint16_t>(),
                                                                                                                                    std::declval<const OneWireTiming *>(),
                                                                                                                                    std::declval<TTransportConfig>()))>,
                                                              TTransportSettings>::value>>
    StaticOwningBus<typename TProtocol::ColorType,
                    TProtocol,
                    TWrappedTransport,
                    NilShader<typename TProtocol::ColorType>,
                    uint16_t> makeBus(uint16_t pixelCount,
                                      OneWireTiming timing,
                                      TTransportConfig &&transportConfig)
    {
        auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(TProtocolTraits::defaultSettings());
        assignProtocolTimingIfPresent(protocolSettings, timing);
        auto transportSettings = resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                         protocolSettings,
                                                         &timing,
                                                         std::forward<TTransportConfig>(transportConfig));
        auto wrapperSettings = makeOneWireWrapperSettings(std::move(transportSettings), timing);

                                TWrappedTransport transport{std::move(wrapperSettings)};
                                TProtocol protocol = makeOwningBusProtocol<TProtocol, TWrappedTransport>(pixelCount,
                                                                      transport,
                                                                      std::move(protocolSettings));
                                const size_t protocolBufferSize = protocol.requiredBufferSizeBytes();
                                NilShader<typename TProtocol::ColorType> shader{};

                                return makeStaticOwningBus<typename TProtocol::ColorType>(BufferHolder<typename TProtocol::ColorType>{pixelCount, nullptr, true},
                                                               BufferHolder<typename TProtocol::ColorType>::nil(),
                                                               BufferHolder<uint8_t>{protocolBufferSize, nullptr, true},
                                                               Topology::linear(pixelCount),
                                                               std::move(protocol),
                                                               std::move(transport),
                                                               std::move(shader),
                                                                   static_cast<uint16_t>(pixelCount));
    }

} // namespace factory
} // namespace lw
