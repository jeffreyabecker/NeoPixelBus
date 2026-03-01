#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "core/Compat.h"

namespace lw
{
namespace factory
{

    enum class DynamicBusProtocolKind : uint8_t
    {
        DotStar,
        Ws2812,
        Ws2811,
        Sk6812
    };

    enum class DynamicBusTransportKind : uint8_t
    {
        PlatformDefault,
        Nil
    };

    struct DynamicBusConfig
    {
        uint16_t pixelCount{0};
        DynamicBusProtocolKind protocol{DynamicBusProtocolKind::Ws2812};
        DynamicBusTransportKind transport{DynamicBusTransportKind::PlatformDefault};
    };

    enum class DynamicBusConfigParseError : uint8_t
    {
        None,
        NullInput,
        EmptyInput,
        EmptyName,
        MissingNamedBus,
        MissingAggregateChildren,
        MissingEquals,
        EmptyKey,
        EmptyValue,
        UnknownKey,
        InvalidPixels,
        PixelsOutOfRange,
        UnknownProtocol,
        UnknownTransport,
        NameTooLong,
        TooManyNames,
        DuplicateName,
        UnknownChildName,
        MissingPixels,
        MissingProtocol
    };

    struct DynamicBusName
    {
        static constexpr size_t MaxLength = 31;
        char value[MaxLength + 1]{};
        uint8_t length{0};
    };

    struct DynamicBusAggregateConfig
    {
        static constexpr size_t MaxChildren = 16;
        DynamicBusName children[MaxChildren]{};
        uint8_t childCount{0};
    };

    struct DynamicBusConfigParseResult
    {
        DynamicBusConfig config{};
        DynamicBusConfigParseError error{DynamicBusConfigParseError::None};
        size_t segmentIndex{0};

        bool ok() const
        {
            return error == DynamicBusConfigParseError::None;
        }

        bool failed() const
        {
            return !ok();
        }
    };

    struct DynamicBusAggregateParseResult
    {
        DynamicBusAggregateConfig config{};
        DynamicBusConfigParseError error{DynamicBusConfigParseError::None};
        size_t segmentIndex{0};

        bool ok() const
        {
            return error == DynamicBusConfigParseError::None;
        }

        bool failed() const
        {
            return !ok();
        }
    };

    namespace detail
    {

        inline bool isAsciiSpace(char value)
        {
            return value == ' ' || value == '\t' || value == '\r' || value == '\n';
        }

        inline char asciiLower(char value)
        {
            if (value >= 'A' && value <= 'Z')
            {
                return static_cast<char>(value - 'A' + 'a');
            }

            return value;
        }

        inline span<const char> trimAscii(span<const char> value)
        {
            size_t start = 0;
            size_t end = value.size();

            while (start < end && isAsciiSpace(value[start]))
            {
                ++start;
            }

            while (end > start && isAsciiSpace(value[end - 1]))
            {
                --end;
            }

            return span<const char>{value.data() + start, end - start};
        }

        inline bool equalsIgnoreCase(span<const char> lhs,
                                     const char *rhs)
        {
            if (rhs == nullptr)
            {
                return false;
            }

            const size_t rhsLength = std::strlen(rhs);
            if (lhs.size() != rhsLength)
            {
                return false;
            }

            for (size_t index = 0; index < rhsLength; ++index)
            {
                if (asciiLower(lhs[index]) != asciiLower(rhs[index]))
                {
                    return false;
                }
            }

            return true;
        }

        inline bool equalsIgnoreCase(span<const char> lhs,
                                     span<const char> rhs)
        {
            if (lhs.size() != rhs.size())
            {
                return false;
            }

            for (size_t index = 0; index < lhs.size(); ++index)
            {
                if (asciiLower(lhs[index]) != asciiLower(rhs[index]))
                {
                    return false;
                }
            }

            return true;
        }

        inline bool extractNamedBusKey(span<const char> key,
                                       span<const char> busName,
                                       span<const char> &unscopedKey)
        {
            key = trimAscii(key);
            busName = trimAscii(busName);

            if (key.size() < 6 || busName.empty())
            {
                return false;
            }

            if (!(asciiLower(key[0]) == 'b' && asciiLower(key[1]) == 'u' && asciiLower(key[2]) == 's' && key[3] == '.'))
            {
                return false;
            }

            const char *nameStart = key.data() + 4;
            const char *end = key.data() + key.size();
            const char *dotAfterName = nameStart;
            while (dotAfterName < end && *dotAfterName != '.')
            {
                ++dotAfterName;
            }

            if (dotAfterName == end)
            {
                return false;
            }

            const span<const char> declaredName{nameStart, static_cast<size_t>(dotAfterName - nameStart)};
            if (!equalsIgnoreCase(declaredName, busName))
            {
                return false;
            }

            const char *suffixStart = dotAfterName + 1;
            if (suffixStart >= end)
            {
                return false;
            }

            unscopedKey = span<const char>{suffixStart, static_cast<size_t>(end - suffixStart)};
            return true;
        }

        inline bool extractNamedBusDeclaration(span<const char> key,
                                               span<const char> &busName,
                                               span<const char> &unscopedKey)
        {
            key = trimAscii(key);

            if (key.size() < 6)
            {
                return false;
            }

            if (!(asciiLower(key[0]) == 'b' && asciiLower(key[1]) == 'u' && asciiLower(key[2]) == 's' && key[3] == '.'))
            {
                return false;
            }

            const char *nameStart = key.data() + 4;
            const char *end = key.data() + key.size();
            const char *dotAfterName = nameStart;
            while (dotAfterName < end && *dotAfterName != '.')
            {
                ++dotAfterName;
            }

            if (dotAfterName == end)
            {
                return false;
            }

            const char *suffixStart = dotAfterName + 1;
            if (suffixStart >= end)
            {
                return false;
            }

            busName = span<const char>{nameStart, static_cast<size_t>(dotAfterName - nameStart)};
            unscopedKey = span<const char>{suffixStart, static_cast<size_t>(end - suffixStart)};
            return true;
        }

        inline bool assignNameToken(DynamicBusName &token,
                                    span<const char> name,
                                    DynamicBusConfigParseError &error)
        {
            name = trimAscii(name);
            if (name.empty())
            {
                error = DynamicBusConfigParseError::EmptyName;
                return false;
            }

            if (name.size() > DynamicBusName::MaxLength)
            {
                error = DynamicBusConfigParseError::NameTooLong;
                return false;
            }

            token.length = static_cast<uint8_t>(name.size());
            for (size_t index = 0; index < name.size(); ++index)
            {
                token.value[index] = asciiLower(name[index]);
            }
            token.value[name.size()] = '\0';
            return true;
        }

        inline bool namesEqual(const DynamicBusName &lhs,
                               const DynamicBusName &rhs)
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

        inline bool containsName(const DynamicBusName *values,
                                 uint8_t count,
                                 const DynamicBusName &needle)
        {
            for (uint8_t index = 0; index < count; ++index)
            {
                if (namesEqual(values[index], needle))
                {
                    return true;
                }
            }

            return false;
        }

        inline bool addUniqueName(DynamicBusName *values,
                                  uint8_t &count,
                                  size_t capacity,
                                  span<const char> name,
                                  DynamicBusConfigParseError &error)
        {
            DynamicBusName token{};
            if (!assignNameToken(token, name, error))
            {
                return false;
            }

            if (containsName(values, count, token))
            {
                return true;
            }

            if (count >= capacity)
            {
                error = DynamicBusConfigParseError::TooManyNames;
                return false;
            }

            values[count++] = token;
            return true;
        }

        inline bool parseChildrenList(span<const char> value,
                                      DynamicBusName *children,
                                      uint8_t &childCount,
                                      size_t maxChildren,
                                      DynamicBusConfigParseError &error)
        {
            value = trimAscii(value);
            if (value.empty())
            {
                error = DynamicBusConfigParseError::MissingAggregateChildren;
                return false;
            }

            const char *cursor = value.data();
            const char *end = value.data() + value.size();

            while (cursor < end)
            {
                const char *nameStart = cursor;
                while (cursor < end && *cursor != '|')
                {
                    ++cursor;
                }

                span<const char> name{nameStart, static_cast<size_t>(cursor - nameStart)};
                name = trimAscii(name);

                DynamicBusName token{};
                if (!assignNameToken(token, name, error))
                {
                    return false;
                }

                if (containsName(children, childCount, token))
                {
                    error = DynamicBusConfigParseError::DuplicateName;
                    return false;
                }

                if (childCount >= maxChildren)
                {
                    error = DynamicBusConfigParseError::TooManyNames;
                    return false;
                }

                children[childCount++] = token;

                if (cursor < end && *cursor == '|')
                {
                    ++cursor;
                }
            }

            return childCount > 0;
        }

        inline bool parseUint16(span<const char> token,
                                uint16_t &value,
                                DynamicBusConfigParseError &error)
        {
            token = trimAscii(token);
            if (token.empty())
            {
                error = DynamicBusConfigParseError::InvalidPixels;
                return false;
            }

            uint32_t accumulator = 0;
            for (size_t index = 0; index < token.size(); ++index)
            {
                const char ch = token[index];
                if (ch < '0' || ch > '9')
                {
                    error = DynamicBusConfigParseError::InvalidPixels;
                    return false;
                }

                accumulator = accumulator * 10u + static_cast<uint32_t>(ch - '0');
                if (accumulator > 65535u)
                {
                    error = DynamicBusConfigParseError::PixelsOutOfRange;
                    return false;
                }
            }

            if (accumulator == 0u)
            {
                error = DynamicBusConfigParseError::PixelsOutOfRange;
                return false;
            }

            value = static_cast<uint16_t>(accumulator);
            return true;
        }

        inline bool parseProtocol(span<const char> token,
                                  DynamicBusProtocolKind &protocol)
        {
            token = trimAscii(token);

            if (equalsIgnoreCase(token, "dotstar") || equalsIgnoreCase(token, "apa102"))
            {
                protocol = DynamicBusProtocolKind::DotStar;
                return true;
            }

            if (equalsIgnoreCase(token, "ws2812"))
            {
                protocol = DynamicBusProtocolKind::Ws2812;
                return true;
            }

            if (equalsIgnoreCase(token, "ws2811"))
            {
                protocol = DynamicBusProtocolKind::Ws2811;
                return true;
            }

            if (equalsIgnoreCase(token, "sk6812"))
            {
                protocol = DynamicBusProtocolKind::Sk6812;
                return true;
            }

            return false;
        }

        inline bool parseTransport(span<const char> token,
                                   DynamicBusTransportKind &transport)
        {
            token = trimAscii(token);

            if (equalsIgnoreCase(token, "platformdefault") || equalsIgnoreCase(token, "default"))
            {
                transport = DynamicBusTransportKind::PlatformDefault;
                return true;
            }

            if (equalsIgnoreCase(token, "nil"))
            {
                transport = DynamicBusTransportKind::Nil;
                return true;
            }

            return false;
        }

        inline bool parseKeyValue(span<const char> key,
                                  span<const char> value,
                                  DynamicBusConfig &config,
                                  bool &seenPixels,
                                  bool &seenProtocol,
                                  DynamicBusConfigParseError &error)
        {
            key = trimAscii(key);
            value = trimAscii(value);

            if (key.empty())
            {
                error = DynamicBusConfigParseError::EmptyKey;
                return false;
            }

            if (value.empty())
            {
                error = DynamicBusConfigParseError::EmptyValue;
                return false;
            }

            if (equalsIgnoreCase(key, "pixels"))
            {
                seenPixels = true;
                return parseUint16(value, config.pixelCount, error);
            }

            if (equalsIgnoreCase(key, "protocol"))
            {
                seenProtocol = true;
                if (!parseProtocol(value, config.protocol))
                {
                    error = DynamicBusConfigParseError::UnknownProtocol;
                    return false;
                }
                return true;
            }

            if (equalsIgnoreCase(key, "transport"))
            {
                if (!parseTransport(value, config.transport))
                {
                    error = DynamicBusConfigParseError::UnknownTransport;
                    return false;
                }
                return true;
            }

            error = DynamicBusConfigParseError::UnknownKey;
            return false;
        }

    } // namespace detail

    inline DynamicBusConfigParseResult parseDynamicBusConfig(span<const char> configText)
    {
        DynamicBusConfigParseResult result{};

        if (configText.data() == nullptr)
        {
            result.error = DynamicBusConfigParseError::NullInput;
            return result;
        }

        if (configText.empty())
        {
            result.error = DynamicBusConfigParseError::EmptyInput;
            return result;
        }

        bool seenPixels = false;
        bool seenProtocol = false;

        const char *cursor = configText.data();
        const char *end = configText.data() + configText.size();

        while (cursor < end)
        {
            const char *segmentStart = cursor;
            while (cursor < end && *cursor != ';')
            {
                ++cursor;
            }

            span<const char> segment{segmentStart, static_cast<size_t>(cursor - segmentStart)};
            segment = detail::trimAscii(segment);

            if (!segment.empty())
            {
                const char *equalsPos = segment.data();
                const char *segmentEnd = segment.data() + segment.size();
                while (equalsPos < segmentEnd && *equalsPos != '=')
                {
                    ++equalsPos;
                }

                if (equalsPos == segmentEnd)
                {
                    result.error = DynamicBusConfigParseError::MissingEquals;
                    return result;
                }

                span<const char> key{segment.data(), static_cast<size_t>(equalsPos - segment.data())};
                span<const char> value{equalsPos + 1, static_cast<size_t>(segmentEnd - (equalsPos + 1))};

                if (!detail::parseKeyValue(key,
                                           value,
                                           result.config,
                                           seenPixels,
                                           seenProtocol,
                                           result.error))
                {
                    return result;
                }
            }

            ++result.segmentIndex;
            if (cursor < end && *cursor == ';')
            {
                ++cursor;
            }
        }

        if (!seenPixels)
        {
            result.error = DynamicBusConfigParseError::MissingPixels;
            return result;
        }

        if (!seenProtocol)
        {
            result.error = DynamicBusConfigParseError::MissingProtocol;
            return result;
        }

        return result;
    }

    inline DynamicBusConfigParseResult parseDynamicBusConfig(span<const char> configText,
                                                             span<const char> busName)
    {
        DynamicBusConfigParseResult result{};

        if (configText.data() == nullptr)
        {
            result.error = DynamicBusConfigParseError::NullInput;
            return result;
        }

        if (configText.empty())
        {
            result.error = DynamicBusConfigParseError::EmptyInput;
            return result;
        }

        busName = detail::trimAscii(busName);
        if (busName.empty())
        {
            result.error = DynamicBusConfigParseError::EmptyName;
            return result;
        }

        bool seenPixels = false;
        bool seenProtocol = false;
        bool seenNamedEntry = false;

        const char *cursor = configText.data();
        const char *end = configText.data() + configText.size();

        while (cursor < end)
        {
            const char *segmentStart = cursor;
            while (cursor < end && *cursor != ';')
            {
                ++cursor;
            }

            span<const char> segment{segmentStart, static_cast<size_t>(cursor - segmentStart)};
            segment = detail::trimAscii(segment);

            if (!segment.empty())
            {
                const char *equalsPos = segment.data();
                const char *segmentEnd = segment.data() + segment.size();
                while (equalsPos < segmentEnd && *equalsPos != '=')
                {
                    ++equalsPos;
                }

                if (equalsPos != segmentEnd)
                {
                    span<const char> rawKey{segment.data(), static_cast<size_t>(equalsPos - segment.data())};
                    span<const char> value{equalsPos + 1, static_cast<size_t>(segmentEnd - (equalsPos + 1))};

                    span<const char> key{};
                    if (detail::extractNamedBusKey(rawKey, busName, key))
                    {
                        seenNamedEntry = true;
                        if (!detail::parseKeyValue(key,
                                                   value,
                                                   result.config,
                                                   seenPixels,
                                                   seenProtocol,
                                                   result.error))
                        {
                            return result;
                        }
                    }
                }
            }

            ++result.segmentIndex;
            if (cursor < end && *cursor == ';')
            {
                ++cursor;
            }
        }

        if (!seenNamedEntry)
        {
            result.error = DynamicBusConfigParseError::MissingNamedBus;
            return result;
        }

        if (!seenPixels)
        {
            result.error = DynamicBusConfigParseError::MissingPixels;
            return result;
        }

        if (!seenProtocol)
        {
            result.error = DynamicBusConfigParseError::MissingProtocol;
            return result;
        }

        return result;
    }

    inline DynamicBusConfigParseResult parseDynamicBusConfig(const char *configText)
    {
        if (configText == nullptr)
        {
            DynamicBusConfigParseResult result{};
            result.error = DynamicBusConfigParseError::NullInput;
            return result;
        }

        return parseDynamicBusConfig(span<const char>{configText, std::strlen(configText)});
    }

    inline DynamicBusConfigParseResult parseDynamicBusConfig(const char *configText,
                                                             const char *busName)
    {
        if (configText == nullptr)
        {
            DynamicBusConfigParseResult result{};
            result.error = DynamicBusConfigParseError::NullInput;
            return result;
        }

        if (busName == nullptr)
        {
            DynamicBusConfigParseResult result{};
            result.error = DynamicBusConfigParseError::EmptyName;
            return result;
        }

        return parseDynamicBusConfig(span<const char>{configText, std::strlen(configText)},
                                     span<const char>{busName, std::strlen(busName)});
    }

    inline DynamicBusAggregateParseResult parseDynamicAggregateConfig(span<const char> configText)
    {
        DynamicBusAggregateParseResult result{};

        if (configText.data() == nullptr)
        {
            result.error = DynamicBusConfigParseError::NullInput;
            return result;
        }

        if (configText.empty())
        {
            result.error = DynamicBusConfigParseError::EmptyInput;
            return result;
        }

        DynamicBusName declaredNames[DynamicBusAggregateConfig::MaxChildren]{};
        uint8_t declaredCount = 0;
        bool seenAggregateChildren = false;

        const char *cursor = configText.data();
        const char *end = configText.data() + configText.size();

        while (cursor < end)
        {
            const char *segmentStart = cursor;
            while (cursor < end && *cursor != ';')
            {
                ++cursor;
            }

            span<const char> segment{segmentStart, static_cast<size_t>(cursor - segmentStart)};
            segment = detail::trimAscii(segment);

            if (!segment.empty())
            {
                const char *equalsPos = segment.data();
                const char *segmentEnd = segment.data() + segment.size();
                while (equalsPos < segmentEnd && *equalsPos != '=')
                {
                    ++equalsPos;
                }

                if (equalsPos == segmentEnd)
                {
                    result.error = DynamicBusConfigParseError::MissingEquals;
                    return result;
                }

                span<const char> key{segment.data(), static_cast<size_t>(equalsPos - segment.data())};
                span<const char> value{equalsPos + 1, static_cast<size_t>(segmentEnd - (equalsPos + 1))};
                key = detail::trimAscii(key);
                value = detail::trimAscii(value);

                span<const char> declaredBusName{};
                span<const char> unscopedKey{};

                if (detail::extractNamedBusDeclaration(key, declaredBusName, unscopedKey))
                {
                    if (!detail::addUniqueName(declaredNames,
                                               declaredCount,
                                               DynamicBusAggregateConfig::MaxChildren,
                                               declaredBusName,
                                               result.error))
                    {
                        return result;
                    }
                }
                else if (detail::equalsIgnoreCase(key, "aggregate.children"))
                {
                    if (seenAggregateChildren)
                    {
                        result.error = DynamicBusConfigParseError::DuplicateName;
                        return result;
                    }

                    seenAggregateChildren = true;
                    if (!detail::parseChildrenList(value,
                                                   result.config.children,
                                                   result.config.childCount,
                                                   DynamicBusAggregateConfig::MaxChildren,
                                                   result.error))
                    {
                        return result;
                    }
                }
            }

            ++result.segmentIndex;
            if (cursor < end && *cursor == ';')
            {
                ++cursor;
            }
        }

        if (!seenAggregateChildren || result.config.childCount == 0)
        {
            result.error = DynamicBusConfigParseError::MissingAggregateChildren;
            return result;
        }

        for (uint8_t index = 0; index < result.config.childCount; ++index)
        {
            if (!detail::containsName(declaredNames, declaredCount, result.config.children[index]))
            {
                result.error = DynamicBusConfigParseError::UnknownChildName;
                return result;
            }
        }

        return result;
    }

    inline DynamicBusAggregateParseResult parseDynamicAggregateConfig(const char *configText)
    {
        if (configText == nullptr)
        {
            DynamicBusAggregateParseResult result{};
            result.error = DynamicBusConfigParseError::NullInput;
            return result;
        }

        return parseDynamicAggregateConfig(span<const char>{configText, std::strlen(configText)});
    }

} // namespace factory
} // namespace lw
