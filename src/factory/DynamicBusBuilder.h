#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "buses/OwningUnifiedPixelBus.h"
#include "colors/NilShader.h"
#include "core/IPixelBus.h"
#include "factory/MakeBus.h"
#include "factory/MakeShader.h"
#include "factory/descriptors/ProtocolDescriptors.h"
#include "factory/descriptors/TransportDescriptors.h"

namespace lw
{
namespace factory
{

    enum class DynamicBusBuilderError : uint8_t
    {
        None,
        EmptyName,
        NameTooLong,
        DuplicateName,
        TooManyNodes,
        TooManyChildren,
        UnknownName,
        InvalidAggregateRef,
        CycleDetected,
        ColorMismatch,
        IncompatibleAggregateColor,
        UnsupportedProtocolTransport,
        BuildFailed
    };

    template <typename TColor>
    struct DynamicBusBuilderResult
    {
        std::unique_ptr<IPixelBus<TColor>> bus{};
        DynamicBusBuilderError error{DynamicBusBuilderError::None};
        size_t childIndex{0};

        bool ok() const
        {
            return bus != nullptr && error == DynamicBusBuilderError::None;
        }

        bool failed() const
        {
            return !ok();
        }
    };

    struct DynamicBusBuilderColorRequirement
    {
        const void *token{nullptr};
        DynamicBusBuilderError error{DynamicBusBuilderError::None};

        bool ok() const
        {
            return token != nullptr && error == DynamicBusBuilderError::None;
        }

        bool failed() const
        {
            return !ok();
        }

        template <typename TColor>
        bool is() const
        {
            return token == colorToken<TColor>();
        }

        template <typename TColor>
        static const void *colorToken()
        {
            static const uint8_t key = 0;
            return static_cast<const void *>(&key);
        }
    };

    template <size_t TMaxNodes = 32,
              size_t TMaxChildren = 16>
    class DynamicBusBuilder
    {
    public:
        DynamicBusBuilder() = default;

        DynamicBusBuilderError lastError() const
        {
            return _lastError;
        }

        template <typename TProtocolDesc,
                  typename TTransportDesc = descriptors::PlatformDefault,
                  typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
                  typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>>
        bool addBus(const char *name,
                    uint16_t pixelCount)
        {
            return addBus<TProtocolDesc, TTransportDesc>(name,
                                                         pixelCount,
                                                         TProtocolTraits::defaultSettings(),
                                                         TTransportTraits::defaultSettings(pixelCount));
        }

        template <typename TProtocolDesc,
                  typename TTransportDesc = descriptors::PlatformDefault,
                  typename TProtocolConfig,
                  typename TTransportConfig>
        bool addBus(const char *name,
                    uint16_t pixelCount,
                    TProtocolConfig protocolConfig,
                    TTransportConfig transportConfig)
        {
            size_t nameIndex = 0;
            if (!reserveNode(name, NodeKind::Bus, nameIndex))
            {
                return false;
            }

            auto recipe = std::make_unique<ConfiguredBusRecipe<TProtocolDesc,
                                                               TTransportDesc,
                                                               TProtocolConfig,
                                                               TTransportConfig>>(std::move(protocolConfig), std::move(transportConfig));
            _nodes[nameIndex].asBus.recipe = std::move(recipe);
            _nodes[nameIndex].asBus.pixelCount = pixelCount;
            return true;
        }

        template <typename TProtocolDesc,
                  typename TTransportDesc = descriptors::PlatformDefault,
                  typename TTransportConfig>
        bool addBus(const char *name,
                    uint16_t pixelCount,
                    OneWireTiming timing,
                    TTransportConfig transportConfig)
        {
            size_t nameIndex = 0;
            if (!reserveNode(name, NodeKind::Bus, nameIndex))
            {
                return false;
            }

            auto recipe = std::make_unique<TimingFirstBusRecipe<TProtocolDesc,
                                                                TTransportDesc,
                                                                TTransportConfig>>(timing, std::move(transportConfig));
            _nodes[nameIndex].asBus.recipe = std::move(recipe);
            _nodes[nameIndex].asBus.pixelCount = pixelCount;
            return true;
        }

        template <typename TProtocolDesc,
                  typename TTransportDesc = descriptors::PlatformDefault,
                  typename TProtocolConfig,
                  typename TTransportConfig>
        bool addBus(const char *name,
                    uint16_t pixelCount,
                    TProtocolConfig protocolConfig,
                    OneWireTiming timing,
                    TTransportConfig transportConfig)
        {
            size_t nameIndex = 0;
            if (!reserveNode(name, NodeKind::Bus, nameIndex))
            {
                return false;
            }

            auto recipe = std::make_unique<TimingConfiguredBusRecipe<TProtocolDesc,
                                                                     TTransportDesc,
                                                                     TProtocolConfig,
                                                                     TTransportConfig>>(std::move(protocolConfig), timing, std::move(transportConfig));
            _nodes[nameIndex].asBus.recipe = std::move(recipe);
            _nodes[nameIndex].asBus.pixelCount = pixelCount;
            return true;
        }

        template <typename TProtocolDesc,
                  typename TTransportDesc,
                  typename TShaderDesc,
                  typename TProtocolConfig,
                  typename TTransportConfig,
                  typename TShaderConfig>
        bool addBus(const char *name,
                    uint16_t pixelCount,
                    TProtocolConfig protocolConfig,
                    TTransportConfig transportConfig,
                    TShaderConfig shaderConfig)
        {
            using TProtocol = typename ProtocolDescriptorTraits<TProtocolDesc>::ProtocolType;
            using TShader = typename ShaderDescriptorTraits<TShaderDesc>::ShaderType;
            static_assert(std::is_same<typename TProtocol::ColorType, typename TShader::ColorType>::value,
                          "Shader descriptor color must match protocol descriptor color");

            size_t nameIndex = 0;
            if (!reserveNode(name, NodeKind::Bus, nameIndex))
            {
                return false;
            }

            auto recipe = std::make_unique<ConfiguredShaderBusRecipe<TProtocolDesc,
                                                                     TTransportDesc,
                                                                     TShaderDesc,
                                                                     TProtocolConfig,
                                                                     TTransportConfig,
                                                                     TShaderConfig>>(std::move(protocolConfig), std::move(transportConfig), std::move(shaderConfig));
            _nodes[nameIndex].asBus.recipe = std::move(recipe);
            _nodes[nameIndex].asBus.pixelCount = pixelCount;
            return true;
        }

        template <typename TProtocolDesc,
                  typename TTransportDesc,
                  typename TShaderDesc,
                  typename TProtocolConfig,
                  typename TTransportConfig,
                  typename TShaderTraits = ShaderDescriptorTraits<TShaderDesc>>
        bool addBus(const char *name,
                    uint16_t pixelCount,
                    TProtocolConfig protocolConfig,
                    TTransportConfig transportConfig)
        {
            return addBus<TProtocolDesc, TTransportDesc, TShaderDesc>(name,
                                                                      pixelCount,
                                                                      std::move(protocolConfig),
                                                                      std::move(transportConfig),
                                                                      TShaderTraits::defaultSettings());
        }

        template <typename TProtocolDesc,
                  typename TTransportDesc,
                  typename TShaderDesc,
                  typename TTransportConfig,
                  typename TShaderConfig>
        bool addBus(const char *name,
                    uint16_t pixelCount,
                    OneWireTiming timing,
                    TTransportConfig transportConfig,
                    TShaderConfig shaderConfig)
        {
            using TProtocol = typename ProtocolDescriptorTraits<TProtocolDesc>::ProtocolType;
            using TShader = typename ShaderDescriptorTraits<TShaderDesc>::ShaderType;
            static_assert(std::is_same<typename TProtocol::ColorType, typename TShader::ColorType>::value,
                          "Shader descriptor color must match protocol descriptor color");

            size_t nameIndex = 0;
            if (!reserveNode(name, NodeKind::Bus, nameIndex))
            {
                return false;
            }

            auto recipe = std::make_unique<TimingFirstShaderBusRecipe<TProtocolDesc,
                                                                      TTransportDesc,
                                                                      TShaderDesc,
                                                                      TTransportConfig,
                                                                      TShaderConfig>>(timing, std::move(transportConfig), std::move(shaderConfig));
            _nodes[nameIndex].asBus.recipe = std::move(recipe);
            _nodes[nameIndex].asBus.pixelCount = pixelCount;
            return true;
        }

        template <typename TProtocolDesc,
                  typename TTransportDesc,
                  typename TShaderDesc,
                  typename TTransportConfig,
                  typename TShaderTraits = ShaderDescriptorTraits<TShaderDesc>>
        bool addBus(const char *name,
                    uint16_t pixelCount,
                    OneWireTiming timing,
                    TTransportConfig transportConfig)
        {
            return addBus<TProtocolDesc, TTransportDesc, TShaderDesc>(name,
                                                                      pixelCount,
                                                                      timing,
                                                                      std::move(transportConfig),
                                                                      TShaderTraits::defaultSettings());
        }

        template <typename TProtocolDesc,
                  typename TTransportDesc,
                  typename TShaderDesc,
                  typename TProtocolConfig,
                  typename TTransportConfig,
                  typename TShaderConfig>
        bool addBus(const char *name,
                    uint16_t pixelCount,
                    TProtocolConfig protocolConfig,
                    OneWireTiming timing,
                    TTransportConfig transportConfig,
                    TShaderConfig shaderConfig)
        {
            using TProtocol = typename ProtocolDescriptorTraits<TProtocolDesc>::ProtocolType;
            using TShader = typename ShaderDescriptorTraits<TShaderDesc>::ShaderType;
            static_assert(std::is_same<typename TProtocol::ColorType, typename TShader::ColorType>::value,
                          "Shader descriptor color must match protocol descriptor color");

            size_t nameIndex = 0;
            if (!reserveNode(name, NodeKind::Bus, nameIndex))
            {
                return false;
            }

            auto recipe = std::make_unique<TimingConfiguredShaderBusRecipe<TProtocolDesc,
                                                                            TTransportDesc,
                                                                            TShaderDesc,
                                                                            TProtocolConfig,
                                                                            TTransportConfig,
                                                                            TShaderConfig>>(std::move(protocolConfig), timing, std::move(transportConfig), std::move(shaderConfig));
            _nodes[nameIndex].asBus.recipe = std::move(recipe);
            _nodes[nameIndex].asBus.pixelCount = pixelCount;
            return true;
        }

        template <typename TProtocolDesc,
                  typename TTransportDesc,
                  typename TShaderDesc,
                  typename TProtocolConfig,
                  typename TTransportConfig,
                  typename TShaderTraits = ShaderDescriptorTraits<TShaderDesc>>
        bool addBus(const char *name,
                    uint16_t pixelCount,
                    TProtocolConfig protocolConfig,
                    OneWireTiming timing,
                    TTransportConfig transportConfig)
        {
            return addBus<TProtocolDesc, TTransportDesc, TShaderDesc>(name,
                                                                      pixelCount,
                                                                      std::move(protocolConfig),
                                                                      timing,
                                                                      std::move(transportConfig),
                                                                      TShaderTraits::defaultSettings());
        }

        bool addAggregate(const char *name,
                          std::initializer_list<const char *> children)
        {
            return addAggregate(name,
                                TopologySettings{},
                                false,
                                span<const char *const>{children.begin(), children.size()});
        }

        bool addAggregate(const char *name,
                          TopologySettings topology,
                          std::initializer_list<const char *> children)
        {
            return addAggregate(name,
                                topology,
                                true,
                                span<const char *const>{children.begin(), children.size()});
        }

        bool addAggregate(const char *name,
                          span<const char *const> children)
        {
            return addAggregate(name,
                                TopologySettings{},
                                false,
                                children);
        }

        bool addAggregate(const char *name,
                          TopologySettings topology,
                          span<const char *const> children)
        {
            return addAggregate(name,
                                topology,
                                true,
                                children);
        }

        DynamicBusBuilderColorRequirement colorRequirement(const char *name) const
        {
            DynamicBusBuilderColorRequirement requirement{};

            NameToken token{};
            DynamicBusBuilderError nameError = DynamicBusBuilderError::None;
            if (!assignName(token, name, nameError))
            {
                requirement.error = nameError;
                return requirement;
            }

            const size_t rootIndex = findNode(token);
            if (rootIndex == npos)
            {
                requirement.error = DynamicBusBuilderError::UnknownName;
                return requirement;
            }

            std::array<uint8_t, TMaxNodes> visiting{};
            size_t failingChildIndex = 0;
            const void *resolvedToken = nullptr;
            DynamicBusBuilderError resolveError = DynamicBusBuilderError::None;

            if (!resolveNodeColor(rootIndex,
                                  visiting,
                                  resolvedToken,
                                  resolveError,
                                  failingChildIndex))
            {
                requirement.error = resolveError;
                return requirement;
            }

            requirement.token = resolvedToken;
            requirement.error = DynamicBusBuilderError::None;
            return requirement;
        }

        template <typename TColor>
        bool canBuildAs(const char *name) const
        {
            const auto requirement = colorRequirement(name);
            return requirement.ok() && requirement.template is<TColor>();
        }

        template <typename TColor>
        DynamicBusBuilderResult<TColor> tryBuild(const char *name) const
        {
            DynamicBusBuilderResult<TColor> result{};

            NameToken token{};
            DynamicBusBuilderError nameError = DynamicBusBuilderError::None;
            if (!assignName(token, name, nameError))
            {
                result.error = nameError;
                return result;
            }

            const size_t rootIndex = findNode(token);
            if (rootIndex == npos)
            {
                result.error = DynamicBusBuilderError::UnknownName;
                return result;
            }

            std::array<uint8_t, TMaxNodes> visiting{};
            size_t failingChildIndex = 0;
            const void *requiredToken = nullptr;
            DynamicBusBuilderError resolveError = DynamicBusBuilderError::None;
            if (!resolveNodeColor(rootIndex,
                                  visiting,
                                  requiredToken,
                                  resolveError,
                                  failingChildIndex))
            {
                result.error = resolveError;
                result.childIndex = failingChildIndex;
                return result;
            }

            if (requiredToken != DynamicBusBuilderColorRequirement::colorToken<TColor>())
            {
                result.error = DynamicBusBuilderError::ColorMismatch;
                return result;
            }

            std::vector<StrandExtent<TColor>> strands{};
            visiting.fill(0);
            size_t maxShaderPixels = 0;
            DynamicBusBuilderError buildError = DynamicBusBuilderError::None;
            if (!appendNode<TColor>(rootIndex, strands, visiting, maxShaderPixels, buildError, failingChildIndex))
            {
                cleanupOwnedStrands(strands);
                result.error = buildError;
                result.childIndex = failingChildIndex;
                return result;
            }

            size_t totalPixels = 0;
            for (const auto &strand : strands)
            {
                totalPixels += strand.length;
            }

            Topology topology = Topology::linear(totalPixels);
            const Node &root = _nodes[rootIndex];
            if (root.kind == NodeKind::Aggregate && root.asAggregate.hasCustomTopology)
            {
                topology = Topology{root.asAggregate.topologySettings};
            }

            auto bus = std::make_unique<UnifiedDynamicOwningBus<TColor>>(totalPixels,
                                                                          maxShaderPixels,
                                                                          std::move(topology),
                                                                          std::move(strands));
            bus->begin();
            result.bus = std::unique_ptr<IPixelBus<TColor>>(std::move(bus));
            return result;
        }

        template <typename TColor>
        std::unique_ptr<IPixelBus<TColor>> build(const char *name) const
        {
            auto result = tryBuild<TColor>(name);
            return std::move(result.bus);
        }

    private:
        static constexpr size_t npos = static_cast<size_t>(-1);

        struct NameToken
        {
            static constexpr size_t MaxLength = 31;
            char value[MaxLength + 1]{};
            uint8_t length{0};
        };

        struct IBusRecipe
        {
            virtual ~IBusRecipe() = default;
            virtual const void *requiredColorToken() const = 0;
            virtual size_t shaderPixelCount(uint16_t pixelCount) const = 0;
            virtual bool appendErased(const void *requestedColorToken,
                                      uint16_t pixelCount,
                                      void *strandsOpaque) const = 0;
        };

        template <typename TProtocolSettings>
        static OneWireTiming resolveOneWireTimingFromSettings(const TProtocolSettings &settings)
        {
            if constexpr (ProtocolSettingsHasTiming<TProtocolSettings>::value)
            {
                return settings.timing;
            }

            return timing::Ws2812x;
        }

        template <typename TProtocolDesc,
                  typename TTransportDesc,
                  typename TProtocolConfig,
                  typename TTransportConfig,
                  typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
                  typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
                  typename TProtocol = typename TProtocolTraits::ProtocolType,
                  typename TTransport = typename TTransportTraits::TransportType,
                  typename TRecipeColor = typename TProtocol::ColorType>
        struct ConfiguredBusRecipe final : IBusRecipe
        {
            ConfiguredBusRecipe(TProtocolConfig protocolConfig,
                                TTransportConfig transportConfig)
                : _protocolConfig(std::move(protocolConfig)),
                  _transportConfig(std::move(transportConfig))
            {
            }

            const void *requiredColorToken() const override
            {
                return DynamicBusBuilderColorRequirement::colorToken<TRecipeColor>();
            }

            size_t shaderPixelCount(uint16_t) const override
            {
                return 0;
            }

            bool appendErased(const void *requestedColorToken,
                              uint16_t pixelCount,
                              void *strandsOpaque) const override
            {
                if (requestedColorToken != requiredColorToken())
                {
                    return false;
                }

                auto *strands = static_cast<std::vector<StrandExtent<TRecipeColor>> *>(strandsOpaque);
                auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(_protocolConfig);
                auto transportSettings = resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                                                                             protocolSettings,
                                                                                                             _transportConfig);

                if constexpr (BusDriverProtocolTransportCompatible<TProtocol, TTransport>)
                {
                    auto transport = std::make_unique<TTransport>(std::move(transportSettings));
                    auto protocol = makeOwningBusProtocol<TProtocol, TTransport>(pixelCount,
                                                                                 *transport,
                                                                                 std::move(protocolSettings));
                    auto protocolPtr = std::make_unique<TProtocol>(std::move(protocol));
                    auto shaderPtr = std::make_unique<NilShader<TRecipeColor>>();

                    strands->push_back(StrandExtent<TRecipeColor>{protocolPtr.release(),
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

            TProtocolConfig _protocolConfig;
            TTransportConfig _transportConfig;
        };

        template <typename TProtocolDesc,
                  typename TTransportDesc,
                  typename TTransportConfig,
                  typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
                  typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
                  typename TProtocol = typename TProtocolTraits::ProtocolType,
                  typename TTransport = typename TTransportTraits::TransportType,
                  typename TRecipeColor = typename TProtocol::ColorType>
        struct TimingFirstBusRecipe final : IBusRecipe
        {
            TimingFirstBusRecipe(OneWireTiming timing,
                                 TTransportConfig transportConfig)
                : _timing(timing),
                  _transportConfig(std::move(transportConfig))
            {
            }

            const void *requiredColorToken() const override
            {
                return DynamicBusBuilderColorRequirement::colorToken<TRecipeColor>();
            }

            size_t shaderPixelCount(uint16_t) const override
            {
                return 0;
            }

            bool appendErased(const void *requestedColorToken,
                              uint16_t pixelCount,
                              void *strandsOpaque) const override
            {
                if (requestedColorToken != requiredColorToken())
                {
                    return false;
                }

                auto *strands = static_cast<std::vector<StrandExtent<TRecipeColor>> *>(strandsOpaque);
                auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(TProtocolTraits::defaultSettings());
                assignProtocolTimingIfPresent(protocolSettings, _timing);
                auto transportSettings = resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                                                                             protocolSettings,
                                                                                                             &_timing,
                                                                                                             _transportConfig);

                if constexpr (BusDriverProtocolTransportCompatible<TProtocol, TTransport>)
                {
                    auto transport = std::make_unique<TTransport>(std::move(transportSettings));
                    auto protocol = makeOwningBusProtocol<TProtocol, TTransport>(pixelCount,
                                                                                 *transport,
                                                                                 std::move(protocolSettings));
                    auto protocolPtr = std::make_unique<TProtocol>(std::move(protocol));
                    auto shaderPtr = std::make_unique<NilShader<TRecipeColor>>();

                    strands->push_back(StrandExtent<TRecipeColor>{protocolPtr.release(),
                                                                  transport.release(),
                                                                  shaderPtr.release(),
                                                                  0,
                                                                  static_cast<size_t>(pixelCount)});
                    return true;
                }

                return false;
            }

            OneWireTiming _timing;
            TTransportConfig _transportConfig;
        };

        template <typename TProtocolDesc,
                  typename TTransportDesc,
                  typename TProtocolConfig,
                  typename TTransportConfig,
                  typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
                  typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
                  typename TProtocol = typename TProtocolTraits::ProtocolType,
                  typename TTransport = typename TTransportTraits::TransportType,
                  typename TRecipeColor = typename TProtocol::ColorType>
        struct TimingConfiguredBusRecipe final : IBusRecipe
        {
            TimingConfiguredBusRecipe(TProtocolConfig protocolConfig,
                                      OneWireTiming timing,
                                      TTransportConfig transportConfig)
                : _protocolConfig(std::move(protocolConfig)),
                  _timing(timing),
                  _transportConfig(std::move(transportConfig))
            {
            }

            const void *requiredColorToken() const override
            {
                return DynamicBusBuilderColorRequirement::colorToken<TRecipeColor>();
            }

            size_t shaderPixelCount(uint16_t) const override
            {
                return 0;
            }

            bool appendErased(const void *requestedColorToken,
                              uint16_t pixelCount,
                              void *strandsOpaque) const override
            {
                if (requestedColorToken != requiredColorToken())
                {
                    return false;
                }

                auto *strands = static_cast<std::vector<StrandExtent<TRecipeColor>> *>(strandsOpaque);
                auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(_protocolConfig);
                assignProtocolTimingIfPresent(protocolSettings, _timing);
                auto transportSettings = resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                                                                             protocolSettings,
                                                                                                             &_timing,
                                                                                                             _transportConfig);

                if constexpr (BusDriverProtocolTransportCompatible<TProtocol, TTransport>)
                {
                    auto transport = std::make_unique<TTransport>(std::move(transportSettings));
                    auto protocol = makeOwningBusProtocol<TProtocol, TTransport>(pixelCount,
                                                                                 *transport,
                                                                                 std::move(protocolSettings));
                    auto protocolPtr = std::make_unique<TProtocol>(std::move(protocol));
                    auto shaderPtr = std::make_unique<NilShader<TRecipeColor>>();

                    strands->push_back(StrandExtent<TRecipeColor>{protocolPtr.release(),
                                                                  transport.release(),
                                                                  shaderPtr.release(),
                                                                  0,
                                                                  static_cast<size_t>(pixelCount)});
                    return true;
                }

                return false;
            }

            TProtocolConfig _protocolConfig;
            OneWireTiming _timing;
            TTransportConfig _transportConfig;
        };

        template <typename TProtocolDesc,
                  typename TTransportDesc,
                  typename TShaderDesc,
                  typename TProtocolConfig,
                  typename TTransportConfig,
                  typename TShaderConfig,
                  typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
                  typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
                  typename TShaderTraits = ShaderDescriptorTraits<TShaderDesc>,
                  typename TProtocol = typename TProtocolTraits::ProtocolType,
                  typename TTransport = typename TTransportTraits::TransportType,
                  typename TShader = typename TShaderTraits::ShaderType,
                  typename TRecipeColor = typename TProtocol::ColorType>
        struct ConfiguredShaderBusRecipe final : IBusRecipe
        {
            ConfiguredShaderBusRecipe(TProtocolConfig protocolConfig,
                                      TTransportConfig transportConfig,
                                      TShaderConfig shaderConfig)
                : _protocolConfig(std::move(protocolConfig)),
                  _transportConfig(std::move(transportConfig)),
                  _shaderConfig(std::move(shaderConfig))
            {
            }

            const void *requiredColorToken() const override
            {
                return DynamicBusBuilderColorRequirement::colorToken<TRecipeColor>();
            }

            size_t shaderPixelCount(uint16_t pixelCount) const override
            {
                return static_cast<size_t>(pixelCount);
            }

            bool appendErased(const void *requestedColorToken,
                              uint16_t pixelCount,
                              void *strandsOpaque) const override
            {
                if (requestedColorToken != requiredColorToken())
                {
                    return false;
                }

                static_assert(std::is_same<typename TShader::ColorType, TRecipeColor>::value,
                              "Shader and protocol colors must match");

                auto *strands = static_cast<std::vector<StrandExtent<TRecipeColor>> *>(strandsOpaque);
                auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(_protocolConfig);
                auto transportSettings = resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                                                                             protocolSettings,
                                                                                                             _transportConfig);

                if constexpr (BusDriverProtocolTransportCompatible<TProtocol, TTransport>)
                {
                    auto transport = std::make_unique<TTransport>(std::move(transportSettings));
                    auto protocol = makeOwningBusProtocol<TProtocol, TTransport>(pixelCount,
                                                                                 *transport,
                                                                                 std::move(protocolSettings));
                    auto protocolPtr = std::make_unique<TProtocol>(std::move(protocol));
                    auto shader = makeShader<TShaderDesc>(_shaderConfig);
                    auto shaderPtr = std::make_unique<TShader>(std::move(shader));

                    strands->push_back(StrandExtent<TRecipeColor>{protocolPtr.release(),
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

            TProtocolConfig _protocolConfig;
            TTransportConfig _transportConfig;
            TShaderConfig _shaderConfig;
        };

        template <typename TProtocolDesc,
                  typename TTransportDesc,
                  typename TShaderDesc,
                  typename TTransportConfig,
                  typename TShaderConfig,
                  typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
                  typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
                  typename TShaderTraits = ShaderDescriptorTraits<TShaderDesc>,
                  typename TProtocol = typename TProtocolTraits::ProtocolType,
                  typename TTransport = typename TTransportTraits::TransportType,
                  typename TShader = typename TShaderTraits::ShaderType,
                  typename TRecipeColor = typename TProtocol::ColorType>
        struct TimingFirstShaderBusRecipe final : IBusRecipe
        {
            TimingFirstShaderBusRecipe(OneWireTiming timing,
                                       TTransportConfig transportConfig,
                                       TShaderConfig shaderConfig)
                : _timing(timing),
                  _transportConfig(std::move(transportConfig)),
                  _shaderConfig(std::move(shaderConfig))
            {
            }

            const void *requiredColorToken() const override
            {
                return DynamicBusBuilderColorRequirement::colorToken<TRecipeColor>();
            }

            size_t shaderPixelCount(uint16_t pixelCount) const override
            {
                return static_cast<size_t>(pixelCount);
            }

            bool appendErased(const void *requestedColorToken,
                              uint16_t pixelCount,
                              void *strandsOpaque) const override
            {
                if (requestedColorToken != requiredColorToken())
                {
                    return false;
                }

                static_assert(std::is_same<typename TShader::ColorType, TRecipeColor>::value,
                              "Shader and protocol colors must match");

                auto *strands = static_cast<std::vector<StrandExtent<TRecipeColor>> *>(strandsOpaque);
                auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(TProtocolTraits::defaultSettings());
                assignProtocolTimingIfPresent(protocolSettings, _timing);
                auto transportSettings = resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                                                                             protocolSettings,
                                                                                                             &_timing,
                                                                                                             _transportConfig);

                if constexpr (BusDriverProtocolTransportCompatible<TProtocol, TTransport>)
                {
                    auto transport = std::make_unique<TTransport>(std::move(transportSettings));
                    auto protocol = makeOwningBusProtocol<TProtocol, TTransport>(pixelCount,
                                                                                 *transport,
                                                                                 std::move(protocolSettings));
                    auto protocolPtr = std::make_unique<TProtocol>(std::move(protocol));
                    auto shader = makeShader<TShaderDesc>(_shaderConfig);
                    auto shaderPtr = std::make_unique<TShader>(std::move(shader));

                    strands->push_back(StrandExtent<TRecipeColor>{protocolPtr.release(),
                                                                  transport.release(),
                                                                  shaderPtr.release(),
                                                                  0,
                                                                  static_cast<size_t>(pixelCount)});
                    return true;
                }

                return false;
            }

            OneWireTiming _timing;
            TTransportConfig _transportConfig;
            TShaderConfig _shaderConfig;
        };

        template <typename TProtocolDesc,
                  typename TTransportDesc,
                  typename TShaderDesc,
                  typename TProtocolConfig,
                  typename TTransportConfig,
                  typename TShaderConfig,
                  typename TProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>,
                  typename TTransportTraits = TransportDescriptorTraits<TTransportDesc>,
                  typename TShaderTraits = ShaderDescriptorTraits<TShaderDesc>,
                  typename TProtocol = typename TProtocolTraits::ProtocolType,
                  typename TTransport = typename TTransportTraits::TransportType,
                  typename TShader = typename TShaderTraits::ShaderType,
                  typename TRecipeColor = typename TProtocol::ColorType>
        struct TimingConfiguredShaderBusRecipe final : IBusRecipe
        {
            TimingConfiguredShaderBusRecipe(TProtocolConfig protocolConfig,
                                            OneWireTiming timing,
                                            TTransportConfig transportConfig,
                                            TShaderConfig shaderConfig)
                : _protocolConfig(std::move(protocolConfig)),
                  _timing(timing),
                  _transportConfig(std::move(transportConfig)),
                  _shaderConfig(std::move(shaderConfig))
            {
            }

            const void *requiredColorToken() const override
            {
                return DynamicBusBuilderColorRequirement::colorToken<TRecipeColor>();
            }

            size_t shaderPixelCount(uint16_t pixelCount) const override
            {
                return static_cast<size_t>(pixelCount);
            }

            bool appendErased(const void *requestedColorToken,
                              uint16_t pixelCount,
                              void *strandsOpaque) const override
            {
                if (requestedColorToken != requiredColorToken())
                {
                    return false;
                }

                static_assert(std::is_same<typename TShader::ColorType, TRecipeColor>::value,
                              "Shader and protocol colors must match");

                auto *strands = static_cast<std::vector<StrandExtent<TRecipeColor>> *>(strandsOpaque);
                auto protocolSettings = resolveProtocolSettings<TProtocolDesc>(_protocolConfig);
                assignProtocolTimingIfPresent(protocolSettings, _timing);
                auto transportSettings = resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                                                                             protocolSettings,
                                                                                                             &_timing,
                                                                                                             _transportConfig);

                if constexpr (BusDriverProtocolTransportCompatible<TProtocol, TTransport>)
                {
                    auto transport = std::make_unique<TTransport>(std::move(transportSettings));
                    auto protocol = makeOwningBusProtocol<TProtocol, TTransport>(pixelCount,
                                                                                 *transport,
                                                                                 std::move(protocolSettings));
                    auto protocolPtr = std::make_unique<TProtocol>(std::move(protocol));
                    auto shader = makeShader<TShaderDesc>(_shaderConfig);
                    auto shaderPtr = std::make_unique<TShader>(std::move(shader));

                    strands->push_back(StrandExtent<TRecipeColor>{protocolPtr.release(),
                                                                  transport.release(),
                                                                  shaderPtr.release(),
                                                                  0,
                                                                  static_cast<size_t>(pixelCount)});
                    return true;
                }

                return false;
            }

            TProtocolConfig _protocolConfig;
            OneWireTiming _timing;
            TTransportConfig _transportConfig;
            TShaderConfig _shaderConfig;
        };

        enum class NodeKind : uint8_t
        {
            Bus,
            Aggregate
        };

        struct AggregateData
        {
            std::array<NameToken, TMaxChildren> children{};
            uint8_t childCount{0};
            TopologySettings topologySettings{};
            bool hasCustomTopology{false};
        };

        struct BusData
        {
            std::unique_ptr<IBusRecipe> recipe{};
            uint16_t pixelCount{0};
        };

        struct Node
        {
            NameToken name{};
            NodeKind kind{NodeKind::Bus};
            BusData asBus{};
            AggregateData asAggregate{};
        };

        bool reserveNode(const char *name,
                         NodeKind kind,
                         size_t &reservedIndex)
        {
            NameToken token{};
            DynamicBusBuilderError tokenError = DynamicBusBuilderError::None;
            if (!assignName(token, name, tokenError))
            {
                _lastError = tokenError;
                return false;
            }

            if (findNode(token) != npos)
            {
                _lastError = DynamicBusBuilderError::DuplicateName;
                return false;
            }

            if (_nodeCount >= TMaxNodes)
            {
                _lastError = DynamicBusBuilderError::TooManyNodes;
                return false;
            }

            reservedIndex = _nodeCount;
            _nodes[reservedIndex] = Node{};
            _nodes[reservedIndex].name = token;
            _nodes[reservedIndex].kind = kind;
            ++_nodeCount;
            _lastError = DynamicBusBuilderError::None;
            return true;
        }

        bool addAggregate(const char *name,
                          TopologySettings topology,
                          bool hasCustomTopology,
                          span<const char *const> children)
        {
            size_t nameIndex = 0;
            if (!reserveNode(name, NodeKind::Aggregate, nameIndex))
            {
                return false;
            }

            if (children.size() > TMaxChildren)
            {
                _lastError = DynamicBusBuilderError::TooManyChildren;
                return false;
            }

            auto &aggregate = _nodes[nameIndex].asAggregate;
            aggregate.childCount = 0;
            aggregate.topologySettings = topology;
            aggregate.hasCustomTopology = hasCustomTopology;

            for (size_t childIndex = 0; childIndex < children.size(); ++childIndex)
            {
                const char *child = children[childIndex];
                NameToken token{};
                DynamicBusBuilderError error = DynamicBusBuilderError::None;
                if (!assignName(token, child, error))
                {
                    _lastError = error;
                    return false;
                }

                aggregate.children[aggregate.childCount++] = token;
            }

            _lastError = DynamicBusBuilderError::None;
            return true;
        }

        static bool isAsciiSpace(char value)
        {
            return value == ' ' || value == '\t' || value == '\r' || value == '\n';
        }

        static char asciiLower(char value)
        {
            if (value >= 'A' && value <= 'Z')
            {
                return static_cast<char>(value - 'A' + 'a');
            }

            return value;
        }

        static bool assignName(NameToken &token,
                               const char *name,
                               DynamicBusBuilderError &error)
        {
            if (name == nullptr)
            {
                error = DynamicBusBuilderError::EmptyName;
                return false;
            }

            const char *start = name;
            while (*start != '\0' && isAsciiSpace(*start))
            {
                ++start;
            }

            const char *end = start + std::strlen(start);
            while (end > start && isAsciiSpace(*(end - 1)))
            {
                --end;
            }

            const size_t length = static_cast<size_t>(end - start);
            if (length == 0)
            {
                error = DynamicBusBuilderError::EmptyName;
                return false;
            }

            if (length > NameToken::MaxLength)
            {
                error = DynamicBusBuilderError::NameTooLong;
                return false;
            }

            token.length = static_cast<uint8_t>(length);
            for (size_t index = 0; index < length; ++index)
            {
                token.value[index] = asciiLower(start[index]);
            }
            token.value[length] = '\0';

            error = DynamicBusBuilderError::None;
            return true;
        }

        static bool nameEquals(const NameToken &lhs,
                               const NameToken &rhs)
        {
            if (lhs.length != rhs.length)
            {
                return false;
            }

            for (size_t index = 0; index < lhs.length; ++index)
            {
                if (lhs.value[index] != rhs.value[index])
                {
                    return false;
                }
            }

            return true;
        }

        size_t findNode(const NameToken &token) const
        {
            for (size_t index = 0; index < _nodeCount; ++index)
            {
                if (nameEquals(_nodes[index].name, token))
                {
                    return index;
                }
            }

            return npos;
        }

        bool resolveNodeColor(size_t nodeIndex,
                              std::array<uint8_t, TMaxNodes> &visiting,
                              const void *&resolvedToken,
                              DynamicBusBuilderError &error,
                              size_t &failingChildIndex) const
        {
            if (nodeIndex >= _nodeCount)
            {
                error = DynamicBusBuilderError::UnknownName;
                return false;
            }

            if (visiting[nodeIndex] != 0)
            {
                error = DynamicBusBuilderError::CycleDetected;
                return false;
            }

            visiting[nodeIndex] = 1;

            const Node &node = _nodes[nodeIndex];
            if (node.kind == NodeKind::Bus)
            {
                if (node.asBus.recipe == nullptr)
                {
                    error = DynamicBusBuilderError::BuildFailed;
                    visiting[nodeIndex] = 0;
                    return false;
                }

                resolvedToken = node.asBus.recipe->requiredColorToken();
                visiting[nodeIndex] = 0;
                error = DynamicBusBuilderError::None;
                return true;
            }

            if (node.asAggregate.childCount == 0)
            {
                error = DynamicBusBuilderError::InvalidAggregateRef;
                visiting[nodeIndex] = 0;
                return false;
            }

            const void *aggregateColor = nullptr;
            for (uint8_t child = 0; child < node.asAggregate.childCount; ++child)
            {
                const size_t childIndex = findNode(node.asAggregate.children[child]);
                if (childIndex == npos)
                {
                    error = DynamicBusBuilderError::InvalidAggregateRef;
                    failingChildIndex = child;
                    visiting[nodeIndex] = 0;
                    return false;
                }

                const void *childColor = nullptr;
                if (!resolveNodeColor(childIndex, visiting, childColor, error, failingChildIndex))
                {
                    visiting[nodeIndex] = 0;
                    return false;
                }

                if (aggregateColor == nullptr)
                {
                    aggregateColor = childColor;
                }
                else if (aggregateColor != childColor)
                {
                    error = DynamicBusBuilderError::IncompatibleAggregateColor;
                    failingChildIndex = child;
                    visiting[nodeIndex] = 0;
                    return false;
                }
            }

            resolvedToken = aggregateColor;
            visiting[nodeIndex] = 0;
            error = DynamicBusBuilderError::None;
            return true;
        }

        template <typename TColor>
        bool appendNode(size_t nodeIndex,
                        std::vector<StrandExtent<TColor>> &strands,
                        std::array<uint8_t, TMaxNodes> &visiting,
                        size_t &maxShaderPixels,
                        DynamicBusBuilderError &error,
                        size_t &failingChildIndex) const
        {
            if (nodeIndex >= _nodeCount)
            {
                error = DynamicBusBuilderError::UnknownName;
                return false;
            }

            if (visiting[nodeIndex] != 0)
            {
                error = DynamicBusBuilderError::CycleDetected;
                return false;
            }

            visiting[nodeIndex] = 1;

            const Node &node = _nodes[nodeIndex];
            if (node.kind == NodeKind::Bus)
            {
                if (node.asBus.recipe == nullptr)
                {
                    error = DynamicBusBuilderError::BuildFailed;
                    visiting[nodeIndex] = 0;
                    return false;
                }

                maxShaderPixels = std::max(maxShaderPixels,
                                           node.asBus.recipe->shaderPixelCount(node.asBus.pixelCount));

                if (!node.asBus.recipe->appendErased(DynamicBusBuilderColorRequirement::colorToken<TColor>(),
                                                     node.asBus.pixelCount,
                                                     &strands))
                {
                    error = DynamicBusBuilderError::UnsupportedProtocolTransport;
                    visiting[nodeIndex] = 0;
                    return false;
                }
            }
            else
            {
                for (uint8_t child = 0; child < node.asAggregate.childCount; ++child)
                {
                    const size_t childIndex = findNode(node.asAggregate.children[child]);
                    if (childIndex == npos)
                    {
                        error = DynamicBusBuilderError::InvalidAggregateRef;
                        failingChildIndex = child;
                        visiting[nodeIndex] = 0;
                        return false;
                    }

                    if (!appendNode<TColor>(childIndex, strands, visiting, maxShaderPixels, error, failingChildIndex))
                    {
                        visiting[nodeIndex] = 0;
                        return false;
                    }
                }
            }

            visiting[nodeIndex] = 0;
            error = DynamicBusBuilderError::None;
            return true;
        }

        template <typename TColor>
        static void cleanupOwnedStrands(std::vector<StrandExtent<TColor>> &strands)
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

        std::array<Node, TMaxNodes> _nodes{};
        size_t _nodeCount{0};
        DynamicBusBuilderError _lastError{DynamicBusBuilderError::None};
    };

} // namespace factory
} // namespace lw
