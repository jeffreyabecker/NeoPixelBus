#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

#include "factory/busses/BusDriverConstraints.h"
#include "buses/impl/StaticBus.h"
#include "colors/NilShader.h"
#include "core/Compat.h"
#include "factory/Traits.h"

namespace lw
{
namespace factory
{

    template <typename TProtocol,
              typename TTransport>
    TProtocol makeOwningBusProtocol(uint16_t pixelCount,
                                    TTransport &transport,
                                    typename TProtocol::SettingsType settings);

    template <typename TProtocolSettings>
    void assignProtocolTimingIfPresent(TProtocolSettings &settings,
                                       OneWireTiming timing);

    template <typename TBus>
    class StaticBusFactoryAccessor
    {
    public:
        explicit StaticBusFactoryAccessor(TBus &bus)
            : _bus(bus)
        {
        }

        size_t getBufferSize() const
        {
            return _bus.getBufferSize();
        }

    private:
        TBus &_bus;
    };

    template <typename TProtocolDesc,
              typename TTransportDesc = descriptors::PlatformDefault,
              typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
              typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
              typename TProtocol = typename TProtocolTraits::ProtocolType,
              typename TTransport = typename TTransportTraits::TransportType,
              typename TProtocolSettings = typename TProtocolTraits::SettingsType,
              typename TTransportSettings = typename TTransportTraits::SettingsType>
    class StaticBusFactory
    {
    public:
        using ColorType = typename TProtocol::ColorType;
        using BusType = UnifiedStaticBus<ColorType,
                                         TProtocol,
                                         TTransport,
                                         NilShader<ColorType>,
                                         uint16_t>;

        StaticBusFactory(uint16_t pixelCount,
                         TProtocolSettings protocolSettings,
                         TTransportSettings transportSettings)
            : _pixelCount(pixelCount),
              _protocolSettings(std::move(protocolSettings)),
              _transportSettings(std::move(transportSettings))
        {
        }

        size_t getBufferSize() const
        {
            const size_t protocolBytes = TProtocol::requiredBufferSize(_pixelCount, _protocolSettings);
            return FixedBufferAccessor<ColorType>::totalBytes(_pixelCount,
                                                              0,
                                                              protocolBytes);
        }

        BusType make(uint8_t *buffer = nullptr,
                     ssize_t bufferSize = -1,
                     bool owns = true) const
        {
            TTransport transport{_transportSettings};
            auto protocolSettings = _protocolSettings;
            TProtocol protocol = makeOwningBusProtocol<TProtocol, TTransport>(_pixelCount,
                                                                              transport,
                                                                              std::move(protocolSettings));
            NilShader<ColorType> shader{};

            return makeUnifiedStaticBus<ColorType>(_pixelCount,
                                                   0,
                                                   Topology::linear(_pixelCount),
                                                   buffer,
                                                   bufferSize,
                                                   owns,
                                                   std::move(protocol),
                                                   std::move(transport),
                                                   std::move(shader),
                                                   static_cast<uint16_t>(_pixelCount));
        }

    private:
        uint16_t _pixelCount{0};
        TProtocolSettings _protocolSettings{};
        TTransportSettings _transportSettings{};
    };

    template <typename TBus,
              typename = std::void_t<decltype(std::declval<TBus &>().getBufferSize())>>
    StaticBusFactoryAccessor<TBus> getFactory(TBus &bus)
    {
        return StaticBusFactoryAccessor<TBus>{bus};
    }

    template <typename TBus,
              typename = std::void_t<decltype(std::declval<const TBus &>().getBufferSize())>>
    StaticBusFactoryAccessor<const TBus> getFactory(const TBus &bus)
    {
        return StaticBusFactoryAccessor<const TBus>{bus};
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
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport> &&
                                          std::is_same<typename TProtocolTraits::ColorType, typename TProtocol::ColorType>::value &&
                                          std::is_convertible<lw::remove_cvref_t<decltype(resolveProtocolSettings<TProtocolDesc>(std::declval<TProtocolConfig>()))>,
                                                              TProtocolSettings>::value &&
                                          std::is_convertible<lw::remove_cvref_t<decltype(resolveTransportSettings<TTransportDesc>(std::declval<uint16_t>(),
                                                                                                                                    std::declval<TTransportConfig>()))>,
                                                              TTransportSettings>::value>>
    StaticBusFactory<TProtocolDesc, TTransportDesc> getFactory(uint16_t pixelCount,
                                                                TProtocolConfig &&protocolConfig,
                                                                TTransportConfig &&transportConfig)
    {
        auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(std::forward<TProtocolConfig>(protocolConfig));
        auto transportSettings = resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                                                                     protocolSettings,
                                                                                                     std::forward<TTransportConfig>(transportConfig));

        return StaticBusFactory<TProtocolDesc, TTransportDesc>{pixelCount,
                                                                std::move(protocolSettings),
                                                                std::move(transportSettings)};
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
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport> &&
                                          std::is_same<typename TProtocolTraits::ColorType, typename TProtocol::ColorType>::value &&
                                          std::is_convertible<lw::remove_cvref_t<decltype(resolveTransportSettings<TTransportDesc>(std::declval<uint16_t>(),
                                                                                                                                    std::declval<TTransportConfig>()))>,
                                                              TTransportSettings>::value>>
    StaticBusFactory<TProtocolDesc, TTransportDesc> getFactory(uint16_t pixelCount,
                                                                TTransportConfig &&transportConfig)
    {
        auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(TProtocolTraits::defaultSettings());
        auto transportSettings = resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                                                                     protocolSettings,
                                                                                                     std::forward<TTransportConfig>(transportConfig));

        return StaticBusFactory<TProtocolDesc, TTransportDesc>{pixelCount,
                                                                std::move(protocolSettings),
                                                                std::move(transportSettings)};
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
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport> &&
                                          std::is_same<typename TProtocolTraits::ColorType, typename TProtocol::ColorType>::value &&
                                          std::is_convertible<lw::remove_cvref_t<decltype(resolveProtocolSettings<TProtocolDesc>(std::declval<TProtocolConfig>()))>,
                                                              TProtocolSettings>::value &&
                                          std::is_convertible<lw::remove_cvref_t<decltype(resolveTransportSettings<TTransportDesc>(std::declval<uint16_t>(),
                                                                                                                                    std::declval<const OneWireTiming *>(),
                                                                                                                                    std::declval<TTransportConfig>()))>,
                                                              TTransportSettings>::value>>
    StaticBusFactory<TProtocolDesc, TTransportDesc> getFactory(uint16_t pixelCount,
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

        return StaticBusFactory<TProtocolDesc, TTransportDesc>{pixelCount,
                                                                std::move(protocolSettings),
                                                                std::move(transportSettings)};
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
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport> &&
                                          std::is_same<typename TProtocolTraits::ColorType, typename TProtocol::ColorType>::value &&
                                          std::is_convertible<lw::remove_cvref_t<decltype(resolveTransportSettings<TTransportDesc>(std::declval<uint16_t>(),
                                                                                                                                    std::declval<const OneWireTiming *>(),
                                                                                                                                    std::declval<TTransportConfig>()))>,
                                                              TTransportSettings>::value>>
    StaticBusFactory<TProtocolDesc, TTransportDesc> getFactory(uint16_t pixelCount,
                                                                OneWireTiming timing,
                                                                TTransportConfig &&transportConfig)
    {
        auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(TProtocolTraits::defaultSettings());
        assignProtocolTimingIfPresent(protocolSettings, timing);
        auto transportSettings = resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                                                                     protocolSettings,
                                                                                                     &timing,
                                                                                                     std::forward<TTransportConfig>(transportConfig));

        return StaticBusFactory<TProtocolDesc, TTransportDesc>{pixelCount,
                                                                std::move(protocolSettings),
                                                                std::move(transportSettings)};
    }

    template <typename TProtocol,
              typename TTransport>
    TProtocol makeOwningBusProtocol(uint16_t pixelCount,
                                    TTransport &transport,
                                    typename TProtocol::SettingsType settings)
    {
        if constexpr (std::is_constructible<TProtocol,
                                            uint16_t,
                                            typename TProtocol::SettingsType>::value)
        {
            TProtocol protocol(pixelCount, std::move(settings));
            return protocol;
        }
        else if constexpr (std::is_constructible<TProtocol,
                                                 uint16_t,
                                                 typename TProtocol::SettingsType,
                                                 TTransport &>::value)
        {
            TProtocol protocol(pixelCount, std::move(settings), transport);
            return protocol;
        }
        else
        {
            static_assert(std::is_constructible<TProtocol,
                                               uint16_t,
                                               typename TProtocol::SettingsType>::value ||
                              std::is_constructible<TProtocol,
                                                    uint16_t,
                                                    typename TProtocol::SettingsType,
                                                    TTransport &>::value,
                          "Protocol must be constructible from settings (with or without transport) to build owning bus.");
        }
    }

    template <typename TProtocolDesc,
              typename = void>
    struct ProtocolDescriptorIdleHigh : std::false_type
    {
    };

    template <typename TProtocolDesc>
    struct ProtocolDescriptorIdleHigh<TProtocolDesc,
                                      std::void_t<decltype(TProtocolDesc::IdleHigh)>>
        : std::integral_constant<bool, static_cast<bool>(TProtocolDesc::IdleHigh)>
    {
    };

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
              typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
              typename TProtocol = typename TProtocolTraits::ProtocolType,
              typename TTransport = typename TTransportTraits::TransportType,
              bool TDirectCompatible = BusDriverProtocolTransportCompatible<TProtocol, TTransport>>
    struct BusTypeResolver;

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
                           true>
    {
        using Type = UnifiedStaticBus<typename TProtocol::ColorType,
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
                           false>
    {
        static_assert(BusDriverProtocolTransportCompatible<TProtocol, TTransport>,
                      "Protocol and transport descriptors are not shape-compatible for Bus alias");
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
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport> &&
                                          std::is_same<typename TProtocolTraits::ColorType, typename TProtocol::ColorType>::value &&
                                          std::is_convertible<lw::remove_cvref_t<decltype(resolveProtocolSettings<TProtocolDesc>(std::declval<TProtocolConfig>()))>,
                                                              TProtocolSettings>::value &&
                                          std::is_convertible<lw::remove_cvref_t<decltype(resolveTransportSettings<TTransportDesc>(std::declval<uint16_t>(),
                                                                                                                                    std::declval<TTransportConfig>()))>,
                                                              TTransportSettings>::value>>
    UnifiedStaticBus<typename TProtocol::ColorType,
                     TProtocol,
                     TTransport,
                     NilShader<typename TProtocol::ColorType>,
                     uint16_t> makeBus(uint16_t pixelCount,
                                       TProtocolConfig &&protocolConfig,
                                       TTransportConfig &&transportConfig)
    {
        return getFactory<TProtocolDesc, TTransportDesc>(pixelCount,
                                 std::forward<TProtocolConfig>(protocolConfig),
                                 std::forward<TTransportConfig>(transportConfig))
            .make();
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
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport> &&
                                          std::is_same<typename TProtocolTraits::ColorType, typename TProtocol::ColorType>::value &&
                                          std::is_convertible<lw::remove_cvref_t<decltype(resolveTransportSettings<TTransportDesc>(std::declval<uint16_t>(),
                                                                                                                                    std::declval<TTransportConfig>()))>,
                                                              TTransportSettings>::value>>
    UnifiedStaticBus<typename TProtocol::ColorType,
                     TProtocol,
                     TTransport,
                     NilShader<typename TProtocol::ColorType>,
                     uint16_t> makeBus(uint16_t pixelCount,
                                       TTransportConfig &&transportConfig)
    {
        return getFactory<TProtocolDesc, TTransportDesc>(pixelCount,
                                 std::forward<TTransportConfig>(transportConfig))
            .make();
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
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport> &&
                                          std::is_same<typename TProtocolTraits::ColorType, typename TProtocol::ColorType>::value &&
                                          std::is_convertible<lw::remove_cvref_t<decltype(resolveProtocolSettings<TProtocolDesc>(std::declval<TProtocolConfig>()))>,
                                                              TProtocolSettings>::value &&
                                          std::is_convertible<lw::remove_cvref_t<decltype(resolveTransportSettings<TTransportDesc>(std::declval<uint16_t>(),
                                                                                                                                    std::declval<const OneWireTiming *>(),
                                                                                                                                    std::declval<TTransportConfig>()))>,
                                                              TTransportSettings>::value>>
    UnifiedStaticBus<typename TProtocol::ColorType,
                     TProtocol,
                     TTransport,
                     NilShader<typename TProtocol::ColorType>,
                     uint16_t> makeBus(uint16_t pixelCount,
                                       TProtocolConfig &&protocolConfig,
                                       OneWireTiming timing,
                                       TTransportConfig &&transportConfig)
    {
        return getFactory<TProtocolDesc, TTransportDesc>(pixelCount,
                                 std::forward<TProtocolConfig>(protocolConfig),
                                 timing,
                                 std::forward<TTransportConfig>(transportConfig))
            .make();
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
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport> &&
                                          std::is_same<typename TProtocolTraits::ColorType, typename TProtocol::ColorType>::value &&
                                          std::is_convertible<lw::remove_cvref_t<decltype(resolveTransportSettings<TTransportDesc>(std::declval<uint16_t>(),
                                                                                                                                    std::declval<const OneWireTiming *>(),
                                                                                                                                    std::declval<TTransportConfig>()))>,
                                                              TTransportSettings>::value>>
    UnifiedStaticBus<typename TProtocol::ColorType,
                     TProtocol,
                     TTransport,
                     NilShader<typename TProtocol::ColorType>,
                     uint16_t> makeBus(uint16_t pixelCount,
                                       OneWireTiming timing,
                                       TTransportConfig &&transportConfig)
    {
        return getFactory<TProtocolDesc, TTransportDesc>(pixelCount,
                                 timing,
                                 std::forward<TTransportConfig>(transportConfig))
            .make();
    }

} // namespace factory
} // namespace lw
