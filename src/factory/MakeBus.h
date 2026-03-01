#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

#include "factory/busses/BusDriverConstraints.h"
#include "core/Compat.h"
#include "factory/busses/StaticBusDriverPixelBus.h"
#include "factory/Traits.h"
#include "colors/NilShader.h"
#include "protocols/WithShaderProtocol.h"
#include "transports/OneWireWrapper.h"

namespace npb
{
namespace factory
{

    template <typename TProtocol>
    using NilShaderProtocol = WithOwnedShader<typename TProtocol::ColorType,
                                              NilShader<typename TProtocol::ColorType>,
                                              TProtocol>;

    template <typename TProtocol>
    typename NilShaderProtocol<TProtocol>::SettingsType makeNilShaderProtocolSettings(
        typename TProtocol::SettingsType settings)
    {
        using ShaderProtocol = NilShaderProtocol<TProtocol>;
        using ColorType = typename TProtocol::ColorType;
        using ShaderSettings = typename ShaderProtocol::SettingsType;

        ShaderSettings shaderSettings{};
        static_cast<typename TProtocol::SettingsType &>(shaderSettings) = std::move(settings);
        shaderSettings.shader = NilShader<ColorType>{};
        shaderSettings.allowDirtyShaders = true;
        return shaderSettings;
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
              typename TShaderProtocol = NilShaderProtocol<TProtocol>,
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
              typename TShaderProtocol,
              bool TWrappedCompatible>
    struct BusTypeResolver<TProtocolDesc,
                           TTransportDesc,
                           TProtocolTraits,
                           TTransportTraits,
                           TProtocol,
                           TTransport,
                           TShaderProtocol,
                           true,
                           TWrappedCompatible>
    {
        using Type = StaticBusDriverPixelBusT<TTransport, TShaderProtocol>;
    };

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocolTraits,
              typename TTransportTraits,
              typename TProtocol,
              typename TTransport,
              typename TShaderProtocol>
    struct BusTypeResolver<TProtocolDesc,
                           TTransportDesc,
                           TProtocolTraits,
                           TTransportTraits,
                           TProtocol,
                           TTransport,
                           TShaderProtocol,
                           false,
                           true>
    {
        using Type = StaticBusDriverPixelBusT<OneWireWrapper<TTransport>, TShaderProtocol>;
    };

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocolTraits,
              typename TTransportTraits,
              typename TProtocol,
              typename TTransport,
              typename TShaderProtocol>
    struct BusTypeResolver<TProtocolDesc,
                           TTransportDesc,
                           TProtocolTraits,
                           TTransportTraits,
                           TProtocol,
                           TTransport,
                           TShaderProtocol,
                           false,
                           false>
    {
        static_assert(DescriptorCapabilityCompatible<TProtocolDesc, TTransportDesc, TProtocol, TTransport> ||
                          DescriptorWrappedOneWireCapabilityCompatible<TProtocolDesc, TTransportDesc, TProtocol, TTransport>,
                      "Protocol and transport descriptors are not compatible for Bus alias");
    };

    template <typename TProtocol,
              template <typename>
              class TShaderTemplate>
    using ShaderProtocol = WithOwnedShader<typename TProtocol::ColorType,
                                           TShaderTemplate<typename TProtocol::ColorType>,
                                           TProtocol>;

    template <typename TProtocolDesc,
              typename TTransportDesc = descriptors::PlatformDefault,
              template <typename>
              class TShaderTemplate = NilShader,
              typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
              typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
              typename TProtocol = typename TProtocolTraits::ProtocolType,
              typename TTransport = typename TTransportTraits::TransportType,
              typename TShaderProtocol = ShaderProtocol<TProtocol, TShaderTemplate>>
    using Bus = typename BusTypeResolver<TProtocolDesc,
                                         TTransportDesc,
                                         TProtocolTraits,
                                         TTransportTraits,
                                         TProtocol,
                                         TTransport,
                                         TShaderProtocol>::Type;

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
              typename TShaderProtocol = NilShaderProtocol<TProtocol>,
              typename TProtocolSettings = typename TProtocolTraits::SettingsType,
              typename TTransportSettings = typename TTransportTraits::SettingsType,
              typename TProtocolConfig,
              typename TTransportConfig,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TTransport> &&
                                          DescriptorCapabilityCompatible<TProtocolDesc, TTransportDesc, TProtocol, TTransport> &&
                                          BusDriverProtocolSettingsConstructible<TShaderProtocol, TTransport> &&
                                          std::is_same<typename TProtocolTraits::ColorType, typename TProtocol::ColorType>::value &&
                                          std::is_convertible<npb::remove_cvref_t<decltype(resolveProtocolSettings<TProtocolDesc>(std::declval<TProtocolConfig>()))>,
                                                              TProtocolSettings>::value &&
                                          std::is_convertible<npb::remove_cvref_t<decltype(resolveTransportSettings<TTransportDesc>(std::declval<uint16_t>(),
                                                                                                                                    std::declval<TTransportConfig>()))>,
                                                              TTransportSettings>::value>>
    StaticBusDriverPixelBusT<TTransport, TShaderProtocol> makeBus(uint16_t pixelCount,
                                                                   TProtocolConfig &&protocolConfig,
                                                                   TTransportConfig &&transportConfig)
    {
        auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(std::forward<TProtocolConfig>(protocolConfig));
        auto transportSettings = resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                         protocolSettings,
                                                         std::forward<TTransportConfig>(transportConfig));
        auto shaderSettings = makeNilShaderProtocolSettings<TProtocol>(std::move(protocolSettings));

        return makeStaticDriverPixelBus<TTransport, TShaderProtocol>(pixelCount,
                                                                      std::move(transportSettings),
                                                                      std::move(shaderSettings));
    }

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
              typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
              typename TProtocol = typename TProtocolTraits::ProtocolType,
              typename TTransport = typename TTransportTraits::TransportType,
              typename TShaderProtocol = NilShaderProtocol<TProtocol>,
              typename TProtocolSettings = typename TProtocolTraits::SettingsType,
              typename TTransportSettings = typename TTransportTraits::SettingsType,
              typename TTransportConfig,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TTransport> &&
                                          DescriptorCapabilityCompatible<TProtocolDesc, TTransportDesc, TProtocol, TTransport> &&
                                          BusDriverProtocolSettingsConstructible<TShaderProtocol, TTransport> &&
                                          std::is_same<typename TProtocolTraits::ColorType, typename TProtocol::ColorType>::value &&
                                          std::is_convertible<npb::remove_cvref_t<decltype(resolveTransportSettings<TTransportDesc>(std::declval<uint16_t>(),
                                                                                                                                    std::declval<TTransportConfig>()))>,
                                                              TTransportSettings>::value>>
    StaticBusDriverPixelBusT<TTransport, TShaderProtocol> makeBus(uint16_t pixelCount,
                                                                   TTransportConfig &&transportConfig)
    {
        auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(TProtocolSettings{});
        auto transportSettings = resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                         protocolSettings,
                                                         std::forward<TTransportConfig>(transportConfig));
        auto shaderSettings = makeNilShaderProtocolSettings<TProtocol>(std::move(protocolSettings));

        return makeStaticDriverPixelBus<TTransport, TShaderProtocol>(pixelCount,
                                                                      std::move(transportSettings),
                                                                      std::move(shaderSettings));
    }

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
              typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
              typename TProtocol = typename TProtocolTraits::ProtocolType,
              typename TTransport = typename TTransportTraits::TransportType,
              typename TShaderProtocol = NilShaderProtocol<TProtocol>,
              typename TWrappedTransport = OneWireWrapper<TTransport>,
              typename TProtocolSettings = typename TProtocolTraits::SettingsType,
              typename TTransportSettings = typename TTransportTraits::SettingsType,
              typename TProtocolConfig,
              typename TTransportConfig,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TWrappedTransport> &&
                                          DescriptorWrappedOneWireCapabilityCompatible<TProtocolDesc, TTransportDesc, TProtocol, TTransport> &&
                                          BusDriverProtocolSettingsConstructible<TShaderProtocol, TWrappedTransport> &&
                                          std::is_same<typename TProtocolTraits::ColorType, typename TProtocol::ColorType>::value &&
                                          std::is_convertible<npb::remove_cvref_t<decltype(resolveProtocolSettings<TProtocolDesc>(std::declval<TProtocolConfig>()))>,
                                                              TProtocolSettings>::value &&
                                          std::is_convertible<npb::remove_cvref_t<decltype(resolveTransportSettings<TTransportDesc>(std::declval<uint16_t>(),
                                                                                                                                    std::declval<const OneWireTiming *>(),
                                                                                                                                    std::declval<TTransportConfig>()))>,
                                                              TTransportSettings>::value>>
    StaticBusDriverPixelBusT<TWrappedTransport, TShaderProtocol> makeBus(uint16_t pixelCount,
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
        auto shaderSettings = makeNilShaderProtocolSettings<TProtocol>(std::move(protocolSettings));

        return makeStaticDriverPixelBus<TWrappedTransport, TShaderProtocol>(pixelCount,
                                                                             std::move(wrapperSettings),
                                                                             std::move(shaderSettings));
    }

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
              typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
              typename TProtocol = typename TProtocolTraits::ProtocolType,
              typename TTransport = typename TTransportTraits::TransportType,
              typename TShaderProtocol = NilShaderProtocol<TProtocol>,
              typename TWrappedTransport = OneWireWrapper<TTransport>,
              typename TProtocolSettings = typename TProtocolTraits::SettingsType,
              typename TTransportSettings = typename TTransportTraits::SettingsType,
              typename TTransportConfig,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TWrappedTransport> &&
                                          DescriptorWrappedOneWireCapabilityCompatible<TProtocolDesc, TTransportDesc, TProtocol, TTransport> &&
                                          BusDriverProtocolSettingsConstructible<TShaderProtocol, TWrappedTransport> &&
                                          std::is_same<typename TProtocolTraits::ColorType, typename TProtocol::ColorType>::value &&
                                          std::is_convertible<npb::remove_cvref_t<decltype(resolveTransportSettings<TTransportDesc>(std::declval<uint16_t>(),
                                                                                                                                    std::declval<const OneWireTiming *>(),
                                                                                                                                    std::declval<TTransportConfig>()))>,
                                                              TTransportSettings>::value>>
    StaticBusDriverPixelBusT<TWrappedTransport, TShaderProtocol> makeBus(uint16_t pixelCount,
                                                                          OneWireTiming timing,
                                                                          TTransportConfig &&transportConfig)
    {
        auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(TProtocolSettings{});
        assignProtocolTimingIfPresent(protocolSettings, timing);
        auto transportSettings = resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                         protocolSettings,
                                                         &timing,
                                                         std::forward<TTransportConfig>(transportConfig));
        auto wrapperSettings = makeOneWireWrapperSettings(std::move(transportSettings), timing);
        auto shaderSettings = makeNilShaderProtocolSettings<TProtocol>(std::move(protocolSettings));

        return makeStaticDriverPixelBus<TWrappedTransport, TShaderProtocol>(pixelCount,
                                                                             std::move(wrapperSettings),
                                                                             std::move(shaderSettings));
    }

} // namespace factory
} // namespace npb
