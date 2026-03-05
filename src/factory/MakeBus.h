#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

#include "factory/busses/FactoryTypeConstraints.h"
#include "factory/busses/StaticBus.h"
#include "buses/PixelBus.h"
#include "colors/NilShader.h"
#include "core/Compat.h"
#include "factory/ProtocolAliases.h"
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

    template <typename TProtocol,
              typename TTransport,
              typename TProtocolConfig,
              typename TTransportConfig,
              typename = void>
    struct DirectMakeBusCompatible : std::false_type
    {
    };

    template <typename TProtocol,
              typename TTransport,
              typename TProtocolConfig,
              typename TTransportConfig>
    struct DirectMakeBusCompatible<TProtocol,
                                   TTransport,
                                   TProtocolConfig,
                                   TTransportConfig,
                                   std::void_t<typename TProtocol::ColorType,
                                               typename TProtocol::SettingsType,
                                               typename TTransport::TransportSettingsType>>
        : std::integral_constant<bool,
                                 std::is_convertible<TProtocol *, IProtocol<typename TProtocol::ColorType> *>::value &&
                                     SettingsConstructibleTransportLike<TTransport> &&
                                     FactoryProtocolSettingsConstructible<TProtocol, TTransport> &&
                                     std::is_convertible<lw::remove_cvref_t<TProtocolConfig>, typename TProtocol::SettingsType>::value &&
                                     std::is_convertible<lw::remove_cvref_t<TTransportConfig>, typename TTransport::TransportSettingsType>::value>
    {
    };

    template <typename TShader,
              typename TColor,
              typename = void>
    struct DirectMakeBusShaderCompatible : std::false_type
    {
    };

    template <typename TShader,
              typename TColor>
    struct DirectMakeBusShaderCompatible<TShader,
                                         TColor,
                                         std::void_t<decltype(static_cast<IShader<TColor> *>(std::declval<TShader *>()))>>
        : std::true_type
    {
    };

    template <typename TProtocolAlias,
              typename = void>
    struct IsWs2812xProtocolAlias : std::false_type
    {
    };

    template <typename TProtocolAlias>
    struct IsWs2812xProtocolAlias<TProtocolAlias,
                                  std::void_t<typename TProtocolAlias::ProtocolType,
                                              typename TProtocolAlias::SettingsType,
                                              decltype(TProtocolAlias::defaultSettings()),
                                              decltype(TProtocolAlias::normalizeSettings(std::declval<typename TProtocolAlias::SettingsType>()))>>
        : std::true_type
    {
    };

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
              typename = std::enable_if_t<FactoryProtocolTransportCompatible<TProtocol, TTransport> &&
                                          FactoryProtocolSettingsConstructible<TProtocol, TTransport> &&
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
              typename = std::enable_if_t<FactoryProtocolTransportCompatible<TProtocol, TTransport> &&
                                          FactoryProtocolSettingsConstructible<TProtocol, TTransport> &&
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
              typename = std::enable_if_t<FactoryProtocolTransportCompatible<TProtocol, TTransport> &&
                                          FactoryProtocolSettingsConstructible<TProtocol, TTransport> &&
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
              typename = std::enable_if_t<FactoryProtocolTransportCompatible<TProtocol, TTransport> &&
                                          FactoryProtocolSettingsConstructible<TProtocol, TTransport> &&
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
              bool TDirectCompatible = FactoryProtocolTransportCompatible<TProtocol, TTransport>>
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
        static_assert(FactoryProtocolTransportCompatible<TProtocol, TTransport>,
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

    template <typename TProtocol,
              typename TTransport = PlatformDefaultStaticBusDriverTransport,
              typename TProtocolConfig,
              typename TTransportConfig,
              typename = std::enable_if_t<DirectMakeBusCompatible<TProtocol,
                                                                  TTransport,
                                                                  TProtocolConfig,
                                                                  TTransportConfig>::value>>
    PixelBus<TProtocol,
             TTransport,
             NilShader<typename TProtocol::ColorType>> makePixelBus(uint16_t pixelCount,
                                                               TProtocolConfig &&protocolConfig,
                                                               TTransportConfig &&transportConfig)
    {
        using ProtocolSettingsType = typename TProtocol::SettingsType;
        using TransportSettingsType = typename TTransport::TransportSettingsType;

        return PixelBus<TProtocol,
                        TTransport,
                        NilShader<typename TProtocol::ColorType>>{pixelCount,
                                                                  static_cast<ProtocolSettingsType>(std::forward<TProtocolConfig>(protocolConfig)),
                                                                  static_cast<TransportSettingsType>(std::forward<TTransportConfig>(transportConfig))};
    }

    template <typename TProtocol,
              typename TTransport = PlatformDefaultStaticBusDriverTransport,
              typename TTransportConfig,
              typename = std::enable_if_t<DirectMakeBusCompatible<TProtocol,
                                                                  TTransport,
                                                                  typename TProtocol::SettingsType,
                                                                  TTransportConfig>::value &&
                                          std::is_default_constructible<typename TProtocol::SettingsType>::value>>
    PixelBus<TProtocol,
             TTransport,
             NilShader<typename TProtocol::ColorType>> makePixelBus(uint16_t pixelCount,
                                                               TTransportConfig &&transportConfig)
    {
        return makePixelBus<TProtocol, TTransport>(pixelCount,
                                                   typename TProtocol::SettingsType{},
                                                   std::forward<TTransportConfig>(transportConfig));
    }

    template <typename TProtocol,
              typename TTransport = PlatformDefaultStaticBusDriverTransport,
              typename TProtocolConfig,
              typename TTransportConfig,
              typename = std::enable_if_t<DirectMakeBusCompatible<TProtocol,
                                                                  TTransport,
                                                                  TProtocolConfig,
                                                                  TTransportConfig>::value>>
    PixelBus<TProtocol,
             TTransport,
             NilShader<typename TProtocol::ColorType>> makePixelBus(uint16_t pixelCount,
                                                               TProtocolConfig &&protocolConfig,
                                                               OneWireTiming timing,
                                                               TTransportConfig &&transportConfig)
    {
        using ProtocolSettingsType = typename TProtocol::SettingsType;

        ProtocolSettingsType protocolSettings = static_cast<ProtocolSettingsType>(std::forward<TProtocolConfig>(protocolConfig));
        assignProtocolTimingIfPresent(protocolSettings, timing);

        return makePixelBus<TProtocol, TTransport>(pixelCount,
                               std::move(protocolSettings),
                               std::forward<TTransportConfig>(transportConfig));
    }

    template <typename TProtocol,
              typename TTransport = PlatformDefaultStaticBusDriverTransport,
              typename TTransportConfig,
              typename = std::enable_if_t<DirectMakeBusCompatible<TProtocol,
                                                                  TTransport,
                                                                  typename TProtocol::SettingsType,
                                                                  TTransportConfig>::value &&
                                          std::is_default_constructible<typename TProtocol::SettingsType>::value>>
    PixelBus<TProtocol,
             TTransport,
             NilShader<typename TProtocol::ColorType>> makePixelBus(uint16_t pixelCount,
                                                               OneWireTiming timing,
                                                               TTransportConfig &&transportConfig)
    {
        auto protocolSettings = typename TProtocol::SettingsType{};
        assignProtocolTimingIfPresent(protocolSettings, timing);

        return makePixelBus<TProtocol, TTransport>(pixelCount,
                               std::move(protocolSettings),
                               std::forward<TTransportConfig>(transportConfig));
    }

    template <typename TProtocol,
              typename TTransport = PlatformDefaultStaticBusDriverTransport,
              typename TShader,
              typename TProtocolConfig,
              typename TTransportConfig,
              typename = std::enable_if_t<DirectMakeBusCompatible<TProtocol,
                                                                  TTransport,
                                                                  TProtocolConfig,
                                                                  TTransportConfig>::value &&
                                          DirectMakeBusShaderCompatible<lw::remove_cvref_t<TShader>, typename TProtocol::ColorType>::value>>
    PixelBus<TProtocol,
             TTransport,
             lw::remove_cvref_t<TShader>> makePixelBus(uint16_t pixelCount,
                                                  TProtocolConfig &&protocolConfig,
                                                  TTransportConfig &&transportConfig,
                                                  TShader &&shader)
    {
        using ProtocolSettingsType = typename TProtocol::SettingsType;
        using TransportSettingsType = typename TTransport::TransportSettingsType;

        return PixelBus<TProtocol,
                        TTransport,
                        lw::remove_cvref_t<TShader>>{pixelCount,
                                                     static_cast<ProtocolSettingsType>(std::forward<TProtocolConfig>(protocolConfig)),
                                                     static_cast<TransportSettingsType>(std::forward<TTransportConfig>(transportConfig)),
                                                     std::forward<TShader>(shader)};
    }

    template <typename TWsAlias,
              typename TTransport = PlatformDefaultStaticBusDriverTransport,
              typename TTransportConfig,
              typename = std::enable_if_t<IsWs2812xProtocolAlias<TWsAlias>::value &&
                                          SettingsConstructibleTransportLike<TTransport> &&
                                          std::is_convertible<lw::remove_cvref_t<TTransportConfig>, typename TTransport::TransportSettingsType>::value>>
    PixelBus<typename TWsAlias::ProtocolType,
             TTransport,
             NilShader<typename TWsAlias::ColorType>> makePixelBus(uint16_t pixelCount,
                                                                   TTransportConfig &&transportConfig)
    {
        using ProtocolType = typename TWsAlias::ProtocolType;
        using SettingsType = typename TWsAlias::SettingsType;
        using TransportSettingsType = typename TTransport::TransportSettingsType;

        SettingsType protocolSettings = TWsAlias::defaultSettings();
        TransportSettingsType transportSettings = static_cast<TransportSettingsType>(std::forward<TTransportConfig>(transportConfig));

        return makePixelBus<ProtocolType, TTransport>(pixelCount,
                                                      std::move(protocolSettings),
                                                      std::move(transportSettings));
    }

    template <typename TWsAlias,
              typename TTransport = PlatformDefaultStaticBusDriverTransport,
              typename TProtocolSettings,
              typename TTransportConfig,
              typename = std::enable_if_t<IsWs2812xProtocolAlias<TWsAlias>::value &&
                                          SettingsConstructibleTransportLike<TTransport> &&
                                          std::is_convertible<lw::remove_cvref_t<TProtocolSettings>, typename TWsAlias::SettingsType>::value &&
                                          std::is_convertible<lw::remove_cvref_t<TTransportConfig>, typename TTransport::TransportSettingsType>::value>>
    PixelBus<typename TWsAlias::ProtocolType,
             TTransport,
             NilShader<typename TWsAlias::ColorType>> makePixelBus(uint16_t pixelCount,
                                                                   TProtocolSettings &&protocolSettings,
                                                                   TTransportConfig &&transportConfig)
    {
        using ProtocolType = typename TWsAlias::ProtocolType;
        using SettingsType = typename TWsAlias::SettingsType;
        using TransportSettingsType = typename TTransport::TransportSettingsType;

        SettingsType resolvedProtocolSettings = TWsAlias::normalizeSettings(static_cast<SettingsType>(std::forward<TProtocolSettings>(protocolSettings)));
        TransportSettingsType transportSettings = static_cast<TransportSettingsType>(std::forward<TTransportConfig>(transportConfig));

        return makePixelBus<ProtocolType, TTransport>(pixelCount,
                                                      std::move(resolvedProtocolSettings),
                                                      std::move(transportSettings));
    }

    template <typename TWsAlias,
              typename TTransport = PlatformDefaultStaticBusDriverTransport,
              typename TTransportConfig,
              typename = std::enable_if_t<IsWs2812xProtocolAlias<TWsAlias>::value &&
                                          SettingsConstructibleTransportLike<TTransport> &&
                                          std::is_convertible<lw::remove_cvref_t<TTransportConfig>, typename TTransport::TransportSettingsType>::value>>
    PixelBus<typename TWsAlias::ProtocolType,
             TTransport,
             NilShader<typename TWsAlias::ColorType>> makePixelBus(uint16_t pixelCount,
                                                                   OneWireTiming timing,
                                                                   TTransportConfig &&transportConfig)
    {
        auto protocolSettings = TWsAlias::defaultSettings();
        protocolSettings.timing = timing;

        return makePixelBus<TWsAlias, TTransport>(pixelCount,
                                                  std::move(protocolSettings),
                                                  std::forward<TTransportConfig>(transportConfig));
    }

    template <typename TWsAlias,
              typename TTransport = PlatformDefaultStaticBusDriverTransport,
              typename TProtocolSettings,
              typename TTransportConfig,
              typename = std::enable_if_t<IsWs2812xProtocolAlias<TWsAlias>::value &&
                                          SettingsConstructibleTransportLike<TTransport> &&
                                          std::is_convertible<lw::remove_cvref_t<TProtocolSettings>, typename TWsAlias::SettingsType>::value &&
                                          std::is_convertible<lw::remove_cvref_t<TTransportConfig>, typename TTransport::TransportSettingsType>::value>>
    PixelBus<typename TWsAlias::ProtocolType,
             TTransport,
             NilShader<typename TWsAlias::ColorType>> makePixelBus(uint16_t pixelCount,
                                                                   TProtocolSettings &&protocolSettings,
                                                                   OneWireTiming timing,
                                                                   TTransportConfig &&transportConfig)
    {
        using SettingsType = typename TWsAlias::SettingsType;
        auto wsProtocolSettings = static_cast<SettingsType>(std::forward<TProtocolSettings>(protocolSettings));
        wsProtocolSettings.timing = timing;

        return makePixelBus<TWsAlias, TTransport>(pixelCount,
                                                  std::move(wsProtocolSettings),
                                                  std::forward<TTransportConfig>(transportConfig));
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
              typename = std::enable_if_t<FactoryProtocolTransportCompatible<TProtocol, TTransport> &&
                                          FactoryProtocolSettingsConstructible<TProtocol, TTransport> &&
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
              typename = std::enable_if_t<FactoryProtocolTransportCompatible<TProtocol, TTransport> &&
                                          FactoryProtocolSettingsConstructible<TProtocol, TTransport> &&
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
              typename = std::enable_if_t<FactoryProtocolTransportCompatible<TProtocol, TTransport> &&
                                          FactoryProtocolSettingsConstructible<TProtocol, TTransport> &&
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
              typename = std::enable_if_t<FactoryProtocolTransportCompatible<TProtocol, TTransport> &&
                                          FactoryProtocolSettingsConstructible<TProtocol, TTransport> &&
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
