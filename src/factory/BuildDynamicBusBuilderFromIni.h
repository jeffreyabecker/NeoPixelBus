#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

#include "colors/ChannelOrder.h"
#include "factory/DynamicBusBuilder.h"
#include "factory/IniReader.h"
#include "factory/Traits.h"
#include "protocols/PixieProtocol.h"

namespace lw
{
namespace factory
{

    enum class DynamicBusBuilderIniError : uint8_t
    {
        None,
        MissingPixels,
        MissingProtocol,
        UnknownProtocol,
        UnknownTransport,
        MissingAggregateChildren,
        InvalidTopology,
        BuilderRejectedNode
    };

    template <size_t TMaxNodes = 32,
              size_t TMaxChildren = 16>
    struct DynamicBusBuilderIniParseResult
    {
        DynamicBusBuilder<TMaxNodes, TMaxChildren> builder{};
        DynamicBusBuilderIniError error{DynamicBusBuilderIniError::None};
        DynamicBusBuilderError builderError{DynamicBusBuilderError::None};
        size_t sectionIndex{0};

        bool ok() const
        {
            return error == DynamicBusBuilderIniError::None;
        }

        bool failed() const
        {
            return !ok();
        }
    };

    namespace detail
    {
        constexpr const char *SectionPrefixBus = "bus:";

        constexpr const char *KeyKind = "kind";
        constexpr const char *KeyPixels = "pixels";
        constexpr const char *KeyProtocol = "protocol";
        constexpr const char *KeyTransport = "transport";
        constexpr const char *KeyChildren = "children";

        constexpr const char *KindBus = "bus";
        constexpr const char *KindAggregate = "aggregate";

        constexpr const char *TokenPlatformDefault = "platformdefault";
        constexpr const char *TokenDefault = "default";
        constexpr const char *TokenNil = "nil";
        constexpr const char *TokenPixie = "pixie";

        constexpr const char *KeyTransportDataPin = "transport:dataPin";
        constexpr const char *KeyTransportClockPin = "transport:clockPin";
        constexpr const char *KeyTransportClockRateHz = "transport:clockRateHz";
        constexpr const char *KeyTransportInvert = "transport:invert";

        constexpr const char *TokenCadence4Step = "4step";
        constexpr const char *TokenCadence3Step = "3step";

        constexpr const char *KeyProtocolTimingT0hNs = "protocol:timing.t0hNs";
        constexpr const char *KeyProtocolTimingT0lNs = "protocol:timing.t0lNs";
        constexpr const char *KeyProtocolTimingT1hNs = "protocol:timing.t1hNs";
        constexpr const char *KeyProtocolTimingT1lNs = "protocol:timing.t1lNs";
        constexpr const char *KeyProtocolTimingResetNs = "protocol:timing.resetNs";
        constexpr const char *KeyProtocolTimingCadence = "protocol:timing.cadence";

        constexpr const char *TokenRowMajor = "rowmajor";
        constexpr const char *TokenColumnMajor = "columnmajor";

        constexpr const char *KeyProtocolChannelOrder = "protocol:channelOrder";

        constexpr const char *KeyTopology = "topology";
        constexpr const char *TokenLinear = "linear";
        constexpr const char *TokenTiled = "tiled";
        constexpr const char *KeyPanelWidth = "panelWidth";
        constexpr const char *KeyPanelHeight = "panelHeight";
        constexpr const char *KeyLayout = "layout";
        constexpr const char *KeyTilesWide = "tilesWide";
        constexpr const char *KeyTilesHigh = "tilesHigh";
        constexpr const char *KeyTileLayout = "tileLayout";
        constexpr const char *KeyMosaicRotation = "mosaicRotation";

        inline bool iniStartsWith(span<char> value,
                                  const char *prefix)
        {
            if (prefix == nullptr)
            {
                return false;
            }

            const size_t prefixLength = std::strlen(prefix);
            if (value.size() < prefixLength)
            {
                return false;
            }

            for (size_t index = 0; index < prefixLength; ++index)
            {
                if (iniAsciiLower(value[index]) != iniAsciiLower(prefix[index]))
                {
                    return false;
                }
            }

            return true;
        }

        inline bool iniEqualsText(span<char> value,
                                  const char *text)
        {
            if (text == nullptr)
            {
                return false;
            }

            return iniEqualsIgnoreCase(value, iniSpanFromCStr(text));
        }

        template <typename TDescriptor,
                  typename = void>
        struct DescriptorHasTokens : std::false_type
        {
        };

        template <typename TDescriptor>
        struct DescriptorHasTokens<TDescriptor,
                                   std::void_t<decltype(TDescriptor::Tokens)>> : std::true_type
        {
        };

        template <typename TDescriptor>
        bool descriptorTokenMatches(span<char> token)
        {
            token = iniTrimAscii(token);
            if (token.empty())
            {
                return false;
            }

            if constexpr (DescriptorHasTokens<TDescriptor>::value)
            {
                for (const char *candidate : TDescriptor::Tokens)
                {
                    if (candidate != nullptr && iniEqualsText(token, candidate))
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        inline bool copyToCStr(span<char> source,
                               char *destination,
                               size_t destinationSize)
        {
            source = iniTrimAscii(source);
            if (destination == nullptr || destinationSize == 0 || source.size() + 1 > destinationSize)
            {
                return false;
            }

            for (size_t index = 0; index < source.size(); ++index)
            {
                destination[index] = source[index];
            }

            destination[source.size()] = '\0';
            return true;
        }

        inline bool isValidChannelOrderToken(span<char> token)
        {
            token = iniTrimAscii(token);
            if (token.empty())
            {
                return false;
            }

            if (token.size() < 3 || token.size() > 5)
            {
                return false;
            }

            bool seenR = false;
            bool seenG = false;
            bool seenB = false;
            bool seenC = false;
            bool seenW = false;

            for (size_t index = 0; index < token.size(); ++index)
            {
                const char lower = iniAsciiLower(token[index]);
                bool *slot = nullptr;

                switch (lower)
                {
                case 'r':
                    slot = &seenR;
                    break;
                case 'g':
                    slot = &seenG;
                    break;
                case 'b':
                    slot = &seenB;
                    break;
                case 'c':
                    slot = &seenC;
                    break;
                case 'w':
                    slot = &seenW;
                    break;
                default:
                    return false;
                }

                if (*slot)
                {
                    return false;
                }

                *slot = true;
            }

            return true;
        }

        inline const char *resolveChannelOrderToken(span<char> token)
        {
            token = iniTrimAscii(token);
            if (!isValidChannelOrderToken(token))
            {
                return nullptr;
            }

            std::array<char, 6> normalized{};
            for (size_t index = 0; index < token.size(); ++index)
            {
                const char lower = iniAsciiLower(token[index]);
                if (lower == 'r')
                {
                    normalized[index] = 'R';
                }
                else if (lower == 'g')
                {
                    normalized[index] = 'G';
                }
                else if (lower == 'b')
                {
                    normalized[index] = 'B';
                }
                else if (lower == 'c')
                {
                    normalized[index] = 'C';
                }
                else
                {
                    normalized[index] = 'W';
                }
            }
            normalized[token.size()] = '\0';

            static std::vector<std::array<char, 6>> interned{};
            for (const auto &candidate : interned)
            {
                if (std::strcmp(candidate.data(), normalized.data()) == 0)
                {
                    return candidate.data();
                }
            }

            interned.push_back(normalized);
            return interned.back().data();
        }

        template <typename TOptions,
                  typename = void>
        struct TransportHasDataPin : std::false_type
        {
        };

        template <typename TOptions>
        struct TransportHasDataPin<TOptions,
                                   std::void_t<decltype(std::declval<TOptions &>().dataPin)>> : std::true_type
        {
        };

        template <typename TOptions,
                  typename = void>
        struct TransportHasClockPin : std::false_type
        {
        };

        template <typename TOptions>
        struct TransportHasClockPin<TOptions,
                                    std::void_t<decltype(std::declval<TOptions &>().clockPin)>> : std::true_type
        {
        };

        template <typename TOptions,
                  typename = void>
        struct TransportHasClockRateHz : std::false_type
        {
        };

        template <typename TOptions>
        struct TransportHasClockRateHz<TOptions,
                                       std::void_t<decltype(std::declval<TOptions &>().clockRateHz)>> : std::true_type
        {
        };

        template <typename TOptions,
                  typename = void>
        struct TransportHasInvert : std::false_type
        {
        };

        template <typename TOptions>
        struct TransportHasInvert<TOptions,
                                  std::void_t<decltype(std::declval<TOptions &>().invert)>> : std::true_type
        {
        };

        struct ParsedTransportOptions
        {
            bool hasDataPin{false};
            int dataPin{-1};
            bool hasClockPin{false};
            int clockPin{-1};
            bool hasClockRateHz{false};
            uint32_t clockRateHz{0};
            bool hasInvert{false};
            bool invert{false};
        };

        inline ParsedTransportOptions parseTransportOptions(const IniSection &section)
        {
            ParsedTransportOptions parsed{};

            if (section.exists(KeyTransportDataPin))
            {
                parsed.hasDataPin = true;
                parsed.dataPin = section.get<int>(KeyTransportDataPin);
            }

            if (section.exists(KeyTransportClockPin))
            {
                parsed.hasClockPin = true;
                parsed.clockPin = section.get<int>(KeyTransportClockPin);
            }

            if (section.exists(KeyTransportClockRateHz))
            {
                parsed.hasClockRateHz = true;
                parsed.clockRateHz = section.get<uint32_t>(KeyTransportClockRateHz);
            }

            if (section.exists(KeyTransportInvert))
            {
                parsed.hasInvert = true;
                parsed.invert = section.get<bool>(KeyTransportInvert);
            }

            return parsed;
        }

        template <typename TOptions>
        void applyTransportOptionValues(TOptions &options,
                                        const ParsedTransportOptions &parsed)
        {
            if constexpr (TransportHasDataPin<TOptions>::value)
            {
                if (parsed.hasDataPin)
                {
                    options.dataPin = static_cast<decltype(options.dataPin)>(parsed.dataPin);
                }
            }

            if constexpr (TransportHasClockPin<TOptions>::value)
            {
                if (parsed.hasClockPin)
                {
                    options.clockPin = static_cast<decltype(options.clockPin)>(parsed.clockPin);
                }
            }

            if constexpr (TransportHasClockRateHz<TOptions>::value)
            {
                if (parsed.hasClockRateHz)
                {
                    options.clockRateHz = static_cast<decltype(options.clockRateHz)>(parsed.clockRateHz);
                }
            }

            if constexpr (TransportHasInvert<TOptions>::value)
            {
                if (parsed.hasInvert)
                {
                    options.invert = parsed.invert;
                }
            }
        }

        inline bool parseOneWireCadence(span<char> token,
                                        EncodedClockDataBitPattern &cadence)
        {
            token = iniTrimAscii(token);
            if (token.empty())
            {
                return false;
            }

            if (iniEqualsText(token, TokenCadence4Step))
            {
                cadence = EncodedClockDataBitPattern::FourStep;
                return true;
            }

            if (iniEqualsText(token, TokenCadence3Step))
            {
                cadence = EncodedClockDataBitPattern::ThreeStep;
                return true;
            }

            return false;
        }

        inline bool parseOneWireTimingOverride(const IniSection &section,
                                               OneWireTiming &timing)
        {
            bool hasTiming = false;

            if (section.exists(KeyProtocolTimingT0hNs))
            {
                timing.t0hNs = section.get<uint32_t>(KeyProtocolTimingT0hNs);
                hasTiming = true;
            }

            if (section.exists(KeyProtocolTimingT0lNs))
            {
                timing.t0lNs = section.get<uint32_t>(KeyProtocolTimingT0lNs);
                hasTiming = true;
            }

            if (section.exists(KeyProtocolTimingT1hNs))
            {
                timing.t1hNs = section.get<uint32_t>(KeyProtocolTimingT1hNs);
                hasTiming = true;
            }

            if (section.exists(KeyProtocolTimingT1lNs))
            {
                timing.t1lNs = section.get<uint32_t>(KeyProtocolTimingT1lNs);
                hasTiming = true;
            }

            if (section.exists(KeyProtocolTimingResetNs))
            {
                timing.resetNs = section.get<uint32_t>(KeyProtocolTimingResetNs);
                hasTiming = true;
            }

            if (section.exists(KeyProtocolTimingCadence))
            {
                EncodedClockDataBitPattern cadence = timing.cadence;
                if (parseOneWireCadence(section.getRaw(KeyProtocolTimingCadence), cadence))
                {
                    timing.cadence = cadence;
                    hasTiming = true;
                }
            }

            return hasTiming;
        }

        inline bool parsePanelLayout(span<char> token,
                                     PanelLayout &layout)
        {
            token = iniTrimAscii(token);
            if (token.empty())
            {
                return false;
            }

            if (iniEqualsText(token, TokenRowMajor))
            {
                layout = PanelLayout::RowMajor;
                return true;
            }

            if (iniEqualsText(token, TokenColumnMajor))
            {
                layout = PanelLayout::ColumnMajor;
                return true;
            }

            return false;
        }

        template <size_t TMaxNodes,
                  size_t TMaxChildren,
                  typename TProtocolDesc,
                  typename TProtocolConfig>
        bool addTypedBusWithTransport(DynamicBusBuilder<TMaxNodes, TMaxChildren> &builder,
                                      const char *name,
                                      uint16_t pixels,
                                      const TProtocolConfig &protocolConfig,
                                      bool hasTimingOverride,
                                      const OneWireTiming &timingOverride,
                                      span<char> transportToken,
                                      const ParsedTransportOptions &parsedTransport)
        {
            if (iniEqualsText(transportToken, TokenPlatformDefault) ||
                iniEqualsText(transportToken, TokenDefault))
            {
                PlatformDefaultOptions transportOptions{};
                applyTransportOptionValues(transportOptions, parsedTransport);

                if (hasTimingOverride)
                {
                    return builder.template addBus<TProtocolDesc,
                                                   descriptors::PlatformDefault>(name,
                                                                                 pixels,
                                                                                 protocolConfig,
                                                                                 timingOverride,
                                                                                 std::move(transportOptions));
                }

                return builder.template addBus<TProtocolDesc,
                                               descriptors::PlatformDefault>(name,
                                                                             pixels,
                                                                             protocolConfig,
                                                                             std::move(transportOptions));
            }

            if (descriptorTokenMatches<descriptors::Nil>(transportToken))
            {
                NilOptions transportOptions{};
                applyTransportOptionValues(transportOptions, parsedTransport);

                if (hasTimingOverride)
                {
                    return builder.template addBus<TProtocolDesc,
                                                   descriptors::Nil>(name,
                                                                     pixels,
                                                                     protocolConfig,
                                                                     timingOverride,
                                                                     std::move(transportOptions));
                }

                return builder.template addBus<TProtocolDesc,
                                               descriptors::Nil>(name,
                                                                 pixels,
                                                                 protocolConfig,
                                                                 std::move(transportOptions));
            }

            return false;
        }

        template <size_t TMaxNodes,
                  size_t TMaxChildren,
                  typename TProtocolDesc>
        bool addWsLikeBus(DynamicBusBuilder<TMaxNodes, TMaxChildren> &builder,
                          const char *name,
                          uint16_t pixels,
                          span<char> transportToken,
                          const IniSection &section,
                          const ParsedTransportOptions &parsedTransport)
        {
            Ws2812xOptions protocolConfig{};
            const char *channelOrder = resolveChannelOrderToken(section.getRaw(KeyProtocolChannelOrder));
            if (channelOrder != nullptr)
            {
                protocolConfig.channelOrder = channelOrder;
            }

            OneWireTiming timingOverride = protocolConfig.timing;
            const bool hasTimingOverride = parseOneWireTimingOverride(section, timingOverride);

            if (hasTimingOverride)
            {
                protocolConfig.timing = timingOverride;
            }

            return addTypedBusWithTransport<TMaxNodes,
                                            TMaxChildren,
                                            TProtocolDesc>(builder,
                                                           name,
                                                           pixels,
                                                           protocolConfig,
                                                           hasTimingOverride,
                                                           timingOverride,
                                                           transportToken,
                                                           parsedTransport);
        }

        template <size_t TMaxNodes,
                  size_t TMaxChildren,
                  typename TProtocolDesc,
                  typename TProtocolConfig>
        bool addDotStarLikeBus(DynamicBusBuilder<TMaxNodes, TMaxChildren> &builder,
                               const char *name,
                               uint16_t pixels,
                               span<char> transportToken,
                               const IniSection &section,
                               const ParsedTransportOptions &parsedTransport)
        {
            TProtocolConfig protocolConfig{};
            const char *channelOrder = resolveChannelOrderToken(section.getRaw(KeyProtocolChannelOrder));
            if (channelOrder != nullptr)
            {
                protocolConfig.channelOrder = channelOrder;
            }

            return addTypedBusWithTransport<TMaxNodes,
                                            TMaxChildren,
                                            TProtocolDesc>(builder,
                                                           name,
                                                           pixels,
                                                           protocolConfig,
                                                           false,
                                                           OneWireTiming::Ws2812x,
                                                           transportToken,
                                                           parsedTransport);
        }

        template <size_t TMaxNodes,
                  size_t TMaxChildren>
        bool addPixieBus(DynamicBusBuilder<TMaxNodes, TMaxChildren> &builder,
                         const char *name,
                         uint16_t pixels,
                         span<char> transportToken,
                         const ParsedTransportOptions &parsedTransport)
        {
            auto protocolConfig = ProtocolDescriptorTraits<lw::PixieProtocol>::defaultSettings();
            return addTypedBusWithTransport<TMaxNodes,
                                            TMaxChildren,
                                            lw::PixieProtocol>(builder,
                                                               name,
                                                               pixels,
                                                               protocolConfig,
                                                               false,
                                                               OneWireTiming::Ws2812x,
                                                               transportToken,
                                                               parsedTransport);
        }

        template <size_t TMaxChildren>
        bool parseAggregateChildren(const IniSection &section,
                                    std::array<std::array<char, 32>, TMaxChildren> &childStorage,
                                    std::array<const char *, TMaxChildren> &childPointers,
                                    size_t &childCount)
        {
            childCount = 0;
            span<char> children = iniTrimAscii(section.getRaw(KeyChildren));
            if (children.empty())
            {
                return false;
            }

            const char *cursor = children.data();
            const char *end = children.data() + children.size();

            while (cursor < end)
            {
                const char *begin = cursor;
                while (cursor < end && *cursor != '|')
                {
                    ++cursor;
                }

                span<char> childName{const_cast<char *>(begin), static_cast<size_t>(cursor - begin)};
                childName = iniTrimAscii(childName);

                if (childName.empty() || childCount >= TMaxChildren)
                {
                    return false;
                }

                if (!copyToCStr(childName,
                                childStorage[childCount].data(),
                                childStorage[childCount].size()))
                {
                    return false;
                }

                childPointers[childCount] = childStorage[childCount].data();
                ++childCount;

                if (cursor < end && *cursor == '|')
                {
                    ++cursor;
                }
            }

            return childCount > 0;
        }

        inline bool parseTopology(const IniSection &section,
                                  TopologySettings &topology,
                                  bool &hasCustomTopology)
        {
            hasCustomTopology = false;
            if (!section.exists(KeyTopology))
            {
                return true;
            }

            const span<char> topologyToken = iniTrimAscii(section.getRaw(KeyTopology));
            if (topologyToken.empty() || iniEqualsText(topologyToken, TokenLinear))
            {
                return true;
            }

            if (!iniEqualsText(topologyToken, TokenTiled))
            {
                return false;
            }

            if (!section.exists(KeyPanelWidth) ||
                !section.exists(KeyPanelHeight) ||
                !section.exists(KeyLayout) ||
                !section.exists(KeyTilesWide) ||
                !section.exists(KeyTilesHigh) ||
                !section.exists(KeyTileLayout) ||
                !section.exists(KeyMosaicRotation))
            {
                return false;
            }

            topology.panelWidth = section.get<uint16_t>(KeyPanelWidth);
            topology.panelHeight = section.get<uint16_t>(KeyPanelHeight);
            topology.tilesWide = section.get<uint16_t>(KeyTilesWide);
            topology.tilesHigh = section.get<uint16_t>(KeyTilesHigh);
            topology.mosaicRotation = section.get<bool>(KeyMosaicRotation);

            if (!parsePanelLayout(section.getRaw(KeyLayout), topology.layout) ||
                !parsePanelLayout(section.getRaw(KeyTileLayout), topology.tileLayout))
            {
                return false;
            }

            hasCustomTopology = true;
            return true;
        }

    } // namespace detail

    template <size_t TMaxNodes = 32,
              size_t TMaxChildren = 16>
    DynamicBusBuilderIniParseResult<TMaxNodes, TMaxChildren> tryBuildDynamicBusBuilderFromIni(span<char> input)
    {
        DynamicBusBuilderIniParseResult<TMaxNodes, TMaxChildren> result{};
        auto reader = IniReader::parse(input);

        size_t sectionIndex = 0;
        for (const auto sectionName : reader.sectionNames())
        {
            if (!detail::iniStartsWith(sectionName, detail::SectionPrefixBus))
            {
                ++sectionIndex;
                continue;
            }

            char name[32]{};
            span<char> nodeName{sectionName.data() + 4, sectionName.size() - 4};
            nodeName = detail::iniTrimAscii(nodeName);
            if (!detail::copyToCStr(nodeName, name, sizeof(name)))
            {
                result.error = DynamicBusBuilderIniError::BuilderRejectedNode;
                result.builderError = DynamicBusBuilderError::NameTooLong;
                result.sectionIndex = sectionIndex;
                return result;
            }

            const IniSection section = reader.get(sectionName);
            const span<char> kindToken = detail::iniTrimAscii(section.getRaw(detail::KeyKind));

            if (kindToken.empty() || detail::iniEqualsText(kindToken, detail::KindBus))
            {
                if (!section.exists(detail::KeyPixels))
                {
                    result.error = DynamicBusBuilderIniError::MissingPixels;
                    result.sectionIndex = sectionIndex;
                    return result;
                }

                const uint16_t pixels = section.get<uint16_t>(detail::KeyPixels);

                if (!section.exists(detail::KeyProtocol))
                {
                    result.error = DynamicBusBuilderIniError::MissingProtocol;
                    result.sectionIndex = sectionIndex;
                    return result;
                }

                span<char> protocolToken = detail::iniTrimAscii(section.getRaw(detail::KeyProtocol));
                span<char> transportToken = detail::iniTrimAscii(section.getRaw(detail::KeyTransport));
                if (transportToken.empty())
                {
                    transportToken = detail::iniSpanFromCStr(detail::TokenPlatformDefault);
                }

                const auto parsedTransport = detail::parseTransportOptions(section);

                bool added = false;
                if (detail::descriptorTokenMatches<descriptors::APA102>(protocolToken))
                {
                    added = detail::addDotStarLikeBus<TMaxNodes,
                                                      TMaxChildren,
                                                      descriptors::APA102,
                                                      DotStarOptions>(result.builder,
                                                                      name,
                                                                      pixels,
                                                                      transportToken,
                                                                      section,
                                                                      parsedTransport);
                }
                else if (detail::descriptorTokenMatches<descriptors::HD108>(protocolToken))
                {
                    added = detail::addDotStarLikeBus<TMaxNodes,
                                                      TMaxChildren,
                                                      descriptors::HD108,
                                                      Hd108Options>(result.builder,
                                                                    name,
                                                                    pixels,
                                                                    transportToken,
                                                                    section,
                                                                    parsedTransport);
                }
                else if (detail::descriptorTokenMatches<descriptors::Ws2812T<lw::Rgb8Color>>(protocolToken))
                {
                    added = detail::addWsLikeBus<TMaxNodes,
                                                 TMaxChildren,
                                                 descriptors::Ws2812T<lw::Rgb8Color>>(result.builder,
                                                                      name,
                                                                      pixels,
                                                                      transportToken,
                                                                      section,
                                                                      parsedTransport);
                }
                else if (detail::descriptorTokenMatches<descriptors::Ws2811T<lw::Rgb8Color>>(protocolToken))
                {
                    added = detail::addWsLikeBus<TMaxNodes,
                                                 TMaxChildren,
                                                 descriptors::Ws2811T<lw::Rgb8Color>>(result.builder,
                                                                      name,
                                                                      pixels,
                                                                      transportToken,
                                                                      section,
                                                                      parsedTransport);
                }
                else if (detail::descriptorTokenMatches<descriptors::Sk6812T<lw::Rgb8Color>>(protocolToken))
                {
                    added = detail::addWsLikeBus<TMaxNodes,
                                                 TMaxChildren,
                                                 descriptors::Sk6812T<lw::Rgb8Color>>(result.builder,
                                                                      name,
                                                                      pixels,
                                                                      transportToken,
                                                                      section,
                                                                      parsedTransport);
                }
                else if (detail::descriptorTokenMatches<descriptors::Ws2813T<lw::Rgb8Color>>(protocolToken))
                {
                    added = detail::addWsLikeBus<TMaxNodes,
                                                 TMaxChildren,
                                                 descriptors::Ws2813T<lw::Rgb8Color>>(result.builder,
                                                                      name,
                                                                      pixels,
                                                                      transportToken,
                                                                      section,
                                                                      parsedTransport);
                }
                else if (detail::descriptorTokenMatches<descriptors::Ucs8903T<lw::Rgb8Color>>(protocolToken))
                {
                    added = detail::addWsLikeBus<TMaxNodes,
                                                 TMaxChildren,
                                                 descriptors::Ucs8903T<lw::Rgb8Color>>(result.builder,
                                                                       name,
                                                                       pixels,
                                                                       transportToken,
                                                                       section,
                                                                       parsedTransport);
                }
                else if (detail::descriptorTokenMatches<descriptors::Ucs8904T<lw::Rgb8Color>>(protocolToken))
                {
                    added = detail::addWsLikeBus<TMaxNodes,
                                                 TMaxChildren,
                                                 descriptors::Ucs8904T<lw::Rgb8Color>>(result.builder,
                                                                       name,
                                                                       pixels,
                                                                       transportToken,
                                                                       section,
                                                                       parsedTransport);
                }
                else if (detail::iniEqualsText(protocolToken, detail::TokenPixie))
                {
                    added = detail::addPixieBus(result.builder,
                                                name,
                                                pixels,
                                                transportToken,
                                                parsedTransport);
                }
                else
                {
                    result.error = DynamicBusBuilderIniError::UnknownProtocol;
                    result.sectionIndex = sectionIndex;
                    return result;
                }

                if (!added)
                {
                      if (!(detail::iniEqualsText(transportToken, detail::TokenPlatformDefault) ||
                          detail::iniEqualsText(transportToken, detail::TokenDefault) ||
                          detail::iniEqualsText(transportToken, detail::TokenNil)))
                    {
                        result.error = DynamicBusBuilderIniError::UnknownTransport;
                        result.sectionIndex = sectionIndex;
                        return result;
                    }

                    result.error = DynamicBusBuilderIniError::BuilderRejectedNode;
                    result.builderError = result.builder.lastError();
                    result.sectionIndex = sectionIndex;
                    return result;
                }

                ++sectionIndex;
                continue;
            }

            if (detail::iniEqualsText(kindToken, detail::KindAggregate))
            {
                std::array<std::array<char, 32>, TMaxChildren> childStorage{};
                std::array<const char *, TMaxChildren> childPointers{};
                size_t childCount = 0;
                if (!detail::parseAggregateChildren(section, childStorage, childPointers, childCount))
                {
                    result.error = DynamicBusBuilderIniError::MissingAggregateChildren;
                    result.sectionIndex = sectionIndex;
                    return result;
                }

                TopologySettings topology{};
                bool hasCustomTopology = false;
                if (!detail::parseTopology(section, topology, hasCustomTopology))
                {
                    result.error = DynamicBusBuilderIniError::InvalidTopology;
                    result.sectionIndex = sectionIndex;
                    return result;
                }

                const span<const char *const> children{childPointers.data(), childCount};
                const bool added = hasCustomTopology
                                       ? result.builder.addAggregate(name, topology, children)
                                       : result.builder.addAggregate(name, children);

                if (!added)
                {
                    result.error = DynamicBusBuilderIniError::BuilderRejectedNode;
                    result.builderError = result.builder.lastError();
                    result.sectionIndex = sectionIndex;
                    return result;
                }

                ++sectionIndex;
                continue;
            }

            result.error = DynamicBusBuilderIniError::BuilderRejectedNode;
            result.builderError = DynamicBusBuilderError::BuildFailed;
            result.sectionIndex = sectionIndex;
            return result;
        }

        return result;
    }

    template <size_t TMaxNodes = 32,
              size_t TMaxChildren = 16>
    DynamicBusBuilder<TMaxNodes, TMaxChildren> buildDynamicBusBuilderFromIni(span<char> input)
    {
        auto result = tryBuildDynamicBusBuilderFromIni<TMaxNodes, TMaxChildren>(input);
        return std::move(result.builder);
    }

    template <size_t TMaxNodes = 32,
              size_t TMaxChildren = 16>
    DynamicBusBuilderIniParseResult<TMaxNodes, TMaxChildren> tryBuildDynamicBusBuilderFromIni(char *input)
    {
        if (input == nullptr)
        {
            return DynamicBusBuilderIniParseResult<TMaxNodes, TMaxChildren>{};
        }

        return tryBuildDynamicBusBuilderFromIni<TMaxNodes, TMaxChildren>(span<char>{input, std::strlen(input)});
    }

    template <size_t TMaxNodes = 32,
              size_t TMaxChildren = 16>
    DynamicBusBuilder<TMaxNodes, TMaxChildren> buildDynamicBusBuilderFromIni(char *input)
    {
        auto result = tryBuildDynamicBusBuilderFromIni<TMaxNodes, TMaxChildren>(input);
        return std::move(result.builder);
    }

    template <size_t TMaxNodes = 32,
              size_t TMaxChildren = 16>
    DynamicBusBuilderIniParseResult<TMaxNodes, TMaxChildren> tryBuildDynamicBusBuilderFromIni(const char *input)
    {
        if (input == nullptr)
        {
            return DynamicBusBuilderIniParseResult<TMaxNodes, TMaxChildren>{};
        }

        return tryBuildDynamicBusBuilderFromIni<TMaxNodes, TMaxChildren>(span<char>{const_cast<char *>(input), std::strlen(input)});
    }

    template <size_t TMaxNodes = 32,
              size_t TMaxChildren = 16>
    DynamicBusBuilder<TMaxNodes, TMaxChildren> buildDynamicBusBuilderFromIni(const char *input)
    {
        auto result = tryBuildDynamicBusBuilderFromIni<TMaxNodes, TMaxChildren>(input);
        return std::move(result.builder);
    }

} // namespace factory
} // namespace lw
