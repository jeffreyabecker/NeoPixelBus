#pragma once

#include <cstddef>
#include <cstring>
#include <memory>

#include "core/IPixelBus.h"
#include "factory/BuildDynamicBusBuilderFromIni.h"
#include "factory/IniReader.h"

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

    using DynamicBusIniParseResult = DynamicBusBuilderIniParseResult<>;

    struct DynamicBusFactoryResult
    {
        std::unique_ptr<IPixelBus<Rgb8Color>> bus{};
        DynamicBusFactoryError error{DynamicBusFactoryError::None};
        DynamicBusIniParseResult parse{};
        DynamicBusBuilderError builderError{DynamicBusBuilderError::None};

        bool ok() const
        {
            return bus != nullptr && error == DynamicBusFactoryError::None && parse.ok() && builderError == DynamicBusBuilderError::None;
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
        DynamicBusIniParseResult parse{};
        DynamicBusBuilderError builderError{DynamicBusBuilderError::None};

        bool ok() const
        {
            return bus != nullptr && error == DynamicBusFactoryError::None && parse.ok() && builderError == DynamicBusBuilderError::None;
        }

        bool failed() const
        {
            return !ok();
        }
    };

    namespace dynamic_make_detail
    {
        constexpr const char *SectionPrefixBus = "bus:";
        constexpr const char *KeyKind = "kind";
        constexpr const char *KindAggregate = "aggregate";

        inline bool startsWithIgnoreCase(span<char> value,
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
                if (detail::iniAsciiLower(value[index]) != detail::iniAsciiLower(prefix[index]))
                {
                    return false;
                }
            }

            return true;
        }

        inline bool equalsIgnoreCase(span<char> value,
                                     const char *text)
        {
            if (text == nullptr)
            {
                return false;
            }

            return detail::iniEqualsIgnoreCase(detail::iniTrimAscii(value),
                                               detail::iniSpanFromCStr(text));
        }

        inline bool copyToCStr(span<char> source,
                               char *destination,
                               size_t destinationSize)
        {
            source = detail::iniTrimAscii(source);
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

        inline bool tryExtractBusName(span<char> sectionName,
                                      char *name,
                                      size_t nameSize)
        {
            if (!startsWithIgnoreCase(sectionName, SectionPrefixBus))
            {
                return false;
            }

            span<char> nodeName{sectionName.data() + 4, sectionName.size() - 4};
            return copyToCStr(nodeName, name, nameSize);
        }

        inline bool isAggregateSection(const IniSection &section)
        {
            const span<char> kindToken = detail::iniTrimAscii(section.getRaw(KeyKind));
            return equalsIgnoreCase(kindToken, KindAggregate);
        }

        inline bool tryFindFirstBusName(const char *configText,
                                        char *name,
                                        size_t nameSize)
        {
            if (configText == nullptr)
            {
                return false;
            }

            auto reader = IniReader::parse(span<char>{const_cast<char *>(configText), std::strlen(configText)});
            for (const auto sectionName : reader.sectionNames())
            {
                if (!tryExtractBusName(sectionName, name, nameSize))
                {
                    continue;
                }

                const auto section = reader.get(sectionName);
                if (!isAggregateSection(section))
                {
                    return true;
                }
            }

            return false;
        }

        inline bool tryFindFirstAggregateName(const char *configText,
                                              char *name,
                                              size_t nameSize)
        {
            if (configText == nullptr)
            {
                return false;
            }

            auto reader = IniReader::parse(span<char>{const_cast<char *>(configText), std::strlen(configText)});
            for (const auto sectionName : reader.sectionNames())
            {
                if (!tryExtractBusName(sectionName, name, nameSize))
                {
                    continue;
                }

                const auto section = reader.get(sectionName);
                if (isAggregateSection(section))
                {
                    return true;
                }
            }

            return false;
        }

        inline DynamicBusFactoryError mapBuildError(DynamicBusBuilderError builderError)
        {
            switch (builderError)
            {
            case DynamicBusBuilderError::EmptyName:
            case DynamicBusBuilderError::NameTooLong:
            case DynamicBusBuilderError::UnknownName:
            case DynamicBusBuilderError::InvalidAggregateRef:
            case DynamicBusBuilderError::CycleDetected:
                return DynamicBusFactoryError::ParseFailed;
            case DynamicBusBuilderError::UnsupportedProtocolTransport:
                return DynamicBusFactoryError::UnsupportedProtocolTransport;
            default:
                return DynamicBusFactoryError::UnsupportedProtocolTransport;
            }
        }

    } // namespace dynamic_make_detail

    inline DynamicBusFactoryResult tryMakeDynamicBus(const char *configText)
    {
        DynamicBusFactoryResult result{};
        result.parse = tryBuildDynamicBusBuilderFromIni<>(configText);

        if (result.parse.failed())
        {
            result.error = DynamicBusFactoryError::ParseFailed;
            return result;
        }

        char name[32]{};
        if (!dynamic_make_detail::tryFindFirstBusName(configText, name, sizeof(name)))
        {
            result.error = DynamicBusFactoryError::ParseFailed;
            result.builderError = DynamicBusBuilderError::UnknownName;
            return result;
        }

        auto build = result.parse.builder.template tryBuild<Rgb8Color>(name);
        result.builderError = build.error;
        result.bus = std::move(build.bus);

        if (result.bus == nullptr)
        {
            result.error = dynamic_make_detail::mapBuildError(build.error);
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
        result.parse = tryBuildDynamicBusBuilderFromIni<>(configText);

        if (result.parse.failed())
        {
            result.error = DynamicBusFactoryError::ParseFailed;
            return result;
        }

        auto build = result.parse.builder.template tryBuild<Rgb8Color>(busName);
        result.builderError = build.error;
        result.bus = std::move(build.bus);

        if (result.bus == nullptr)
        {
            result.error = dynamic_make_detail::mapBuildError(build.error);
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
        result.parse = tryBuildDynamicBusBuilderFromIni<>(configText);

        if (result.parse.failed())
        {
            result.error = DynamicBusFactoryError::ParseFailed;
            return result;
        }

        char name[32]{};
        if (!dynamic_make_detail::tryFindFirstAggregateName(configText, name, sizeof(name)))
        {
            result.error = DynamicBusFactoryError::ParseFailed;
            result.builderError = DynamicBusBuilderError::UnknownName;
            return result;
        }

        auto build = result.parse.builder.template tryBuild<Rgb8Color>(name);
        result.builderError = build.error;
        result.bus = std::move(build.bus);

        if (result.bus == nullptr)
        {
            result.error = dynamic_make_detail::mapBuildError(build.error);
            return result;
        }

        return result;
    }

    inline std::unique_ptr<IPixelBus<Rgb8Color>> makeDynamicAggregateBus(const char *configText)
    {
        auto result = tryMakeDynamicAggregateBus(configText);
        return std::move(result.bus);
    }

} // namespace factory
} // namespace lw
