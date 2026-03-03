#pragma once

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "buses/OwningUnifiedPixelBus.h"
#include "colors/NilShader.h"
#include "core/IPixelBus.h"
#include "factory/DynamicBusConfigParser.h"
#include "factory/MakeBus.h"
#include "factory/descriptors/ProtocolDescriptors.h"
#include "factory/descriptors/TransportDescriptors.h"

namespace lw
{
namespace factory
{

    enum class DynamicBusFactoryError : uint8_t
    {
        None,
        ParseFailed,
        UnsupportedProtocolTransport
    };

    struct DynamicBusFactoryResult
    {
        std::unique_ptr<IPixelBus<Rgb8Color>> bus{};
        DynamicBusFactoryError error{DynamicBusFactoryError::None};
        DynamicBusConfigParseResult parse{};

        bool ok() const
        {
            return bus != nullptr && error == DynamicBusFactoryError::None && parse.ok();
        }

        bool failed() const
        {
            return !ok();
        }
    };

    struct DynamicBusAggregateFactoryResult
    {
        std::unique_ptr<IPixelBus<Rgb8Color>> bus{};
        DynamicBusFactoryError error{DynamicBusFactoryError::None};
        DynamicBusAggregateParseResult parse{};

        bool ok() const
        {
            return bus != nullptr && error == DynamicBusFactoryError::None && parse.ok();
        }

        bool failed() const
        {
            return !ok();
        }
    };

    namespace detail
    {

        template <typename TColor,
                  typename TProtocol,
                  typename TTransport>
        std::unique_ptr<IPixelBus<TColor>> makeSingleStrandDynamicBus(uint16_t pixelCount,
                                                                       TProtocol protocol,
                                                                       std::unique_ptr<TTransport> transport)
        {
            auto protocolPtr = std::make_unique<TProtocol>(std::move(protocol));
            auto shaderPtr = std::make_unique<NilShader<TColor>>();

            std::vector<StrandExtent<TColor>> strands{};
            strands.push_back(StrandExtent<TColor>{protocolPtr.release(),
                                                   transport.release(),
                                                   shaderPtr.release(),
                                                   0,
                                                   static_cast<size_t>(pixelCount)});

            auto bus = std::make_unique<UnifiedDynamicOwningBus<TColor>>(pixelCount,
                                                                          0,
                                                                          Topology::linear(pixelCount),
                                                                          std::move(strands));
            bus->begin();
            return std::unique_ptr<IPixelBus<TColor>>(std::move(bus));
        }

        template <typename TProtocolDesc,
                  typename TTransportDesc,
                  typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
                  typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
                  typename TProtocol = typename TProtocolTraits::ProtocolType,
                  typename TTransport = typename TTransportTraits::TransportType>
        std::unique_ptr<IPixelBus<typename TProtocol::ColorType>> makeRuntimeBusFromDescriptors(uint16_t pixelCount)
        {
            using TColor = typename TProtocol::ColorType;

            auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(TProtocolTraits::defaultSettings());
            auto transportSettings = resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                                                                         protocolSettings);

            if constexpr (BusDriverProtocolTransportCompatible<TProtocol, TTransport>)
            {
                auto transport = std::make_unique<TTransport>(std::move(transportSettings));
                auto protocol = makeOwningBusProtocol<TProtocol, TTransport>(pixelCount,
                                                                             *transport,
                                                                             std::move(protocolSettings));

                return makeSingleStrandDynamicBus<TColor>(pixelCount,
                                                          std::move(protocol),
                                                          std::move(transport));
            }
            else
            {
                return nullptr;
            }
        }

        template <typename TProtocolDesc,
                  typename TTransportDesc,
                  typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
                  typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
                  typename TProtocol = typename TProtocolTraits::ProtocolType,
                  typename TTransport = typename TTransportTraits::TransportType>
        bool appendRuntimeStrandFromDescriptors(uint16_t pixelCount,
                                                std::vector<StrandExtent<typename TProtocol::ColorType>> &strands)
        {
            using TColor = typename TProtocol::ColorType;

            auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(TProtocolTraits::defaultSettings());
            auto transportSettings = resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                                                                         protocolSettings);

            if constexpr (BusDriverProtocolTransportCompatible<TProtocol, TTransport>)
            {
                auto transport = std::make_unique<TTransport>(std::move(transportSettings));
                auto protocol = makeOwningBusProtocol<TProtocol, TTransport>(pixelCount,
                                                                             *transport,
                                                                             std::move(protocolSettings));
                auto protocolPtr = std::make_unique<TProtocol>(std::move(protocol));
                auto shaderPtr = std::make_unique<NilShader<TColor>>();

                strands.push_back(StrandExtent<TColor>{protocolPtr.release(),
                                                       transport.release(),
                                                       shaderPtr.release(),
                                                       0,
                                                       static_cast<size_t>(pixelCount)});
                return true;
            }
            else
            {
                return false;
            }
        }

        template <typename TProtocolDesc>
        std::unique_ptr<IPixelBus<Rgb8Color>> makeBusForTransportKind(DynamicBusTransportKind transport,
                                                                       uint16_t pixelCount)
        {
            switch (transport)
            {
            case DynamicBusTransportKind::Nil:
                return makeRuntimeBusFromDescriptors<TProtocolDesc, descriptors::Nil>(pixelCount);
            case DynamicBusTransportKind::PlatformDefault:
                return makeRuntimeBusFromDescriptors<TProtocolDesc, descriptors::PlatformDefault>(pixelCount);
            default:
                return nullptr;
            }
        }

        template <typename TProtocolDesc>
        bool appendStrandForTransportKind(DynamicBusTransportKind transport,
                                          uint16_t pixelCount,
                                          std::vector<StrandExtent<Rgb8Color>> &strands)
        {
            switch (transport)
            {
            case DynamicBusTransportKind::Nil:
                return appendRuntimeStrandFromDescriptors<TProtocolDesc, descriptors::Nil>(pixelCount, strands);
            case DynamicBusTransportKind::PlatformDefault:
                return appendRuntimeStrandFromDescriptors<TProtocolDesc, descriptors::PlatformDefault>(pixelCount, strands);
            default:
                return false;
            }
        }

        inline std::unique_ptr<IPixelBus<Rgb8Color>> makeBusFromParsedConfig(const DynamicBusConfig &config)
        {
            switch (config.protocol)
            {
            case DynamicBusProtocolKind::DotStar:
                return makeBusForTransportKind<descriptors::APA102>(config.transport,
                                                                     config.pixelCount);
            case DynamicBusProtocolKind::Ws2812:
                return makeBusForTransportKind<descriptors::Ws2812T<lw::Rgb8Color>>(config.transport,
                                                                                      config.pixelCount);
            case DynamicBusProtocolKind::Ws2811:
                return makeBusForTransportKind<descriptors::Ws2811T<lw::Rgb8Color>>(config.transport,
                                                                                      config.pixelCount);
            case DynamicBusProtocolKind::Sk6812:
                return makeBusForTransportKind<descriptors::Sk6812T<lw::Rgb8Color>>(config.transport,
                                                                                      config.pixelCount);
            default:
                return nullptr;
            }
        }

        inline bool appendStrandFromParsedConfig(const DynamicBusConfig &config,
                                                 std::vector<StrandExtent<Rgb8Color>> &strands)
        {
            switch (config.protocol)
            {
            case DynamicBusProtocolKind::DotStar:
                return appendStrandForTransportKind<descriptors::APA102>(config.transport,
                                                                         config.pixelCount,
                                                                         strands);
            case DynamicBusProtocolKind::Ws2812:
                return appendStrandForTransportKind<descriptors::Ws2812T<lw::Rgb8Color>>(config.transport,
                                                                                           config.pixelCount,
                                                                                           strands);
            case DynamicBusProtocolKind::Ws2811:
                return appendStrandForTransportKind<descriptors::Ws2811T<lw::Rgb8Color>>(config.transport,
                                                                                           config.pixelCount,
                                                                                           strands);
            case DynamicBusProtocolKind::Sk6812:
                return appendStrandForTransportKind<descriptors::Sk6812T<lw::Rgb8Color>>(config.transport,
                                                                                           config.pixelCount,
                                                                                           strands);
            default:
                return false;
            }
        }

        inline void cleanupOwnedStrands(std::vector<StrandExtent<Rgb8Color>> &strands)
        {
            for (auto &strand : strands)
            {
                delete strand.protocol;
                delete strand.transport;
                delete strand.shader;
                strand.protocol = nullptr;
                strand.transport = nullptr;
                strand.shader = nullptr;
            }
            strands.clear();
        }

    } // namespace detail

    inline DynamicBusFactoryResult tryMakeDynamicBus(const char *configText)
    {
        DynamicBusFactoryResult result{};
        result.parse = parseDynamicBusConfig(configText);

        if (result.parse.failed())
        {
            result.error = DynamicBusFactoryError::ParseFailed;
            return result;
        }

        const DynamicBusConfig config = result.parse.config;
        result.bus = detail::makeBusFromParsedConfig(config);

        if (result.bus == nullptr)
        {
            result.error = DynamicBusFactoryError::UnsupportedProtocolTransport;
            return result;
        }

        return result;
    }

    inline DynamicBusFactoryResult tryMakeBus(const char *configText)
    {
        return tryMakeDynamicBus(configText);
    }

    inline DynamicBusFactoryResult tryMakeDynamicBus(const char *configText,
                                                     const char *busName)
    {
        DynamicBusFactoryResult result{};
        result.parse = parseDynamicBusConfig(configText, busName);

        if (result.parse.failed())
        {
            result.error = DynamicBusFactoryError::ParseFailed;
            return result;
        }

        const DynamicBusConfig config = result.parse.config;
        result.bus = detail::makeBusFromParsedConfig(config);

        if (result.bus == nullptr)
        {
            result.error = DynamicBusFactoryError::UnsupportedProtocolTransport;
            return result;
        }

        return result;
    }

    inline DynamicBusFactoryResult tryMakeBus(const char *configText,
                                              const char *busName)
    {
        return tryMakeDynamicBus(configText, busName);
    }

    inline std::unique_ptr<IPixelBus<Rgb8Color>> makeDynamicBus(const char *configText)
    {
        auto result = tryMakeDynamicBus(configText);
        return std::move(result.bus);
    }

    inline std::unique_ptr<IPixelBus<Rgb8Color>> makeBus(const char *configText)
    {
        auto result = tryMakeBus(configText);
        return std::move(result.bus);
    }

    inline std::unique_ptr<IPixelBus<Rgb8Color>> makeDynamicBus(const char *configText,
                                                                const char *busName)
    {
        auto result = tryMakeDynamicBus(configText, busName);
        return std::move(result.bus);
    }

    inline std::unique_ptr<IPixelBus<Rgb8Color>> makeBus(const char *configText,
                                                         const char *busName)
    {
        auto result = tryMakeBus(configText, busName);
        return std::move(result.bus);
    }

    inline DynamicBusAggregateFactoryResult tryMakeDynamicAggregateBus(const char *configText)
    {
        DynamicBusAggregateFactoryResult result{};
        result.parse = parseDynamicAggregateConfig(configText);

        if (result.parse.failed())
        {
            result.error = DynamicBusFactoryError::ParseFailed;
            return result;
        }

        std::vector<StrandExtent<Rgb8Color>> strands{};
        size_t totalPixels = 0;

        for (uint8_t index = 0; index < result.parse.config.childCount; ++index)
        {
            const char *name = result.parse.config.children[index].value;
            const auto childParse = parseDynamicBusConfig(configText, name);
            if (childParse.failed())
            {
                detail::cleanupOwnedStrands(strands);
                result.error = DynamicBusFactoryError::ParseFailed;
                return result;
            }

            if (!detail::appendStrandFromParsedConfig(childParse.config, strands))
            {
                detail::cleanupOwnedStrands(strands);
                result.error = DynamicBusFactoryError::UnsupportedProtocolTransport;
                return result;
            }

            totalPixels += static_cast<size_t>(childParse.config.pixelCount);
        }

        auto bus = std::make_unique<UnifiedDynamicOwningBus<Rgb8Color>>(totalPixels,
                                                                         0,
                                                                         Topology::linear(totalPixels),
                                                                         std::move(strands));
        bus->begin();
        result.bus = std::unique_ptr<IPixelBus<Rgb8Color>>(std::move(bus));
        return result;
    }

    inline std::unique_ptr<IPixelBus<Rgb8Color>> makeDynamicAggregateBus(const char *configText)
    {
        auto result = tryMakeDynamicAggregateBus(configText);
        return std::move(result.bus);
    }

} // namespace factory
} // namespace lw
