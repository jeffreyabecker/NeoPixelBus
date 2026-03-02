#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>
#include <vector>

#if __has_include(<Arduino.h>)
#include <Arduino.h>
#define LW_INI_READER_HAS_ARDUINO_STRING 1
#endif

#include "core/Compat.h"

namespace lw
{
namespace factory
{

    class IniReader;

    namespace detail
    {
        inline span<char> iniSpanFromCStr(const char *value)
        {
            if (value == nullptr)
            {
                return span<char>{};
            }

            return span<char>{const_cast<char *>(value), std::strlen(value)};
        }

#if defined(LW_INI_READER_HAS_ARDUINO_STRING)
        inline span<char> iniSpanFromArduinoString(const String &value)
        {
            return span<char>{const_cast<char *>(value.c_str()), static_cast<size_t>(value.length())};
        }
#endif

        inline bool iniIsAsciiSpace(char value)
        {
            return value == ' ' || value == '\t' || value == '\r' || value == '\n';
        }

        inline char iniAsciiLower(char value)
        {
            if (value >= 'A' && value <= 'Z')
            {
                return static_cast<char>(value - 'A' + 'a');
            }

            return value;
        }

        inline span<char> iniTrimAscii(span<char> value)
        {
            size_t start = 0;
            size_t end = value.size();

            while (start < end && iniIsAsciiSpace(value[start]))
            {
                ++start;
            }

            while (end > start && iniIsAsciiSpace(value[end - 1]))
            {
                --end;
            }

            return span<char>{value.data() + start, end - start};
        }

        inline bool iniEqualsIgnoreCase(span<char> lhs,
                                        span<char> rhs)
        {
            if (lhs.size() != rhs.size())
            {
                return false;
            }

            for (size_t index = 0; index < lhs.size(); ++index)
            {
                if (iniAsciiLower(lhs[index]) != iniAsciiLower(rhs[index]))
                {
                    return false;
                }
            }

            return true;
        }

        template <typename TResult,
                  typename = void>
        struct IniValueParser
        {
            static TResult parse(span<char>)
            {
                return TResult{};
            }
        };

        template <>
        struct IniValueParser<span<char>, void>
        {
            static span<char> parse(span<char> value)
            {
                return value;
            }
        };

        template <>
        struct IniValueParser<bool, void>
        {
            static bool parse(span<char> value)
            {
                value = iniTrimAscii(value);

                static char trueText[] = "true";
                static char yesText[] = "yes";
                static char onText[] = "on";
                static char oneText[] = "1";

                static char falseText[] = "false";
                static char noText[] = "no";
                static char offText[] = "off";
                static char zeroText[] = "0";

                if (iniEqualsIgnoreCase(value, span<char>{trueText, 4}) ||
                    iniEqualsIgnoreCase(value, span<char>{yesText, 3}) ||
                    iniEqualsIgnoreCase(value, span<char>{onText, 2}) ||
                    iniEqualsIgnoreCase(value, span<char>{oneText, 1}))
                {
                    return true;
                }

                if (iniEqualsIgnoreCase(value, span<char>{falseText, 5}) ||
                    iniEqualsIgnoreCase(value, span<char>{noText, 2}) ||
                    iniEqualsIgnoreCase(value, span<char>{offText, 3}) ||
                    iniEqualsIgnoreCase(value, span<char>{zeroText, 1}))
                {
                    return false;
                }

                return false;
            }
        };

        template <typename TResult>
        struct IniValueParser<TResult,
                              std::enable_if_t<std::is_integral<TResult>::value && !std::is_same<TResult, bool>::value>>
        {
            static TResult parse(span<char> value)
            {
                value = iniTrimAscii(value);
                if (value.empty())
                {
                    return TResult{};
                }

                bool negative = false;
                size_t index = 0;
                if (value[index] == '+' || value[index] == '-')
                {
                    negative = (value[index] == '-');
                    ++index;
                }

                if (index >= value.size())
                {
                    return TResult{};
                }

                uint64_t accumulator = 0;
                for (; index < value.size(); ++index)
                {
                    const char ch = value[index];
                    if (ch < '0' || ch > '9')
                    {
                        return TResult{};
                    }

                    accumulator = accumulator * 10ULL + static_cast<uint64_t>(ch - '0');
                }

                if constexpr (std::is_signed<TResult>::value)
                {
                    if (negative)
                    {
                        const uint64_t maxMagnitude = static_cast<uint64_t>(std::numeric_limits<TResult>::max()) + 1ULL;
                        if (accumulator > maxMagnitude)
                        {
                            return std::numeric_limits<TResult>::min();
                        }

                        const int64_t signedValue = -static_cast<int64_t>(accumulator);
                        return static_cast<TResult>(signedValue);
                    }

                    if (accumulator > static_cast<uint64_t>(std::numeric_limits<TResult>::max()))
                    {
                        return std::numeric_limits<TResult>::max();
                    }

                    return static_cast<TResult>(accumulator);
                }
                else
                {
                    if (negative)
                    {
                        return TResult{};
                    }

                    if (accumulator > static_cast<uint64_t>(std::numeric_limits<TResult>::max()))
                    {
                        return std::numeric_limits<TResult>::max();
                    }

                    return static_cast<TResult>(accumulator);
                }
            }
        };

        template <typename TResult>
        struct IniValueParser<TResult,
                              std::enable_if_t<std::is_floating_point<TResult>::value>>
        {
            static TResult parse(span<char> value)
            {
                value = iniTrimAscii(value);
                if (value.empty())
                {
                    return static_cast<TResult>(0);
                }

                bool negative = false;
                size_t index = 0;
                if (value[index] == '+' || value[index] == '-')
                {
                    negative = (value[index] == '-');
                    ++index;
                }

                if (index >= value.size())
                {
                    return static_cast<TResult>(0);
                }

                long double integralPart = 0.0L;
                bool sawDigit = false;
                while (index < value.size() && value[index] >= '0' && value[index] <= '9')
                {
                    sawDigit = true;
                    integralPart = integralPart * 10.0L + static_cast<long double>(value[index] - '0');
                    ++index;
                }

                long double fractionalPart = 0.0L;
                long double fractionalScale = 1.0L;
                if (index < value.size() && value[index] == '.')
                {
                    ++index;
                    while (index < value.size() && value[index] >= '0' && value[index] <= '9')
                    {
                        sawDigit = true;
                        fractionalPart = fractionalPart * 10.0L + static_cast<long double>(value[index] - '0');
                        fractionalScale *= 10.0L;
                        ++index;
                    }
                }

                if (!sawDigit)
                {
                    return static_cast<TResult>(0);
                }

                int exponent = 0;
                bool exponentNegative = false;
                if (index < value.size() && (value[index] == 'e' || value[index] == 'E'))
                {
                    ++index;
                    if (index < value.size() && (value[index] == '+' || value[index] == '-'))
                    {
                        exponentNegative = (value[index] == '-');
                        ++index;
                    }

                    if (index >= value.size() || value[index] < '0' || value[index] > '9')
                    {
                        return static_cast<TResult>(0);
                    }

                    while (index < value.size() && value[index] >= '0' && value[index] <= '9')
                    {
                        exponent = exponent * 10 + static_cast<int>(value[index] - '0');
                        ++index;
                    }
                }

                if (index != value.size())
                {
                    return static_cast<TResult>(0);
                }

                long double parsed = integralPart + (fractionalPart / fractionalScale);
                if (exponent != 0)
                {
                    int power = exponent;
                    while (power > 0)
                    {
                        parsed = exponentNegative ? (parsed / 10.0L) : (parsed * 10.0L);
                        --power;
                    }
                }

                if (negative)
                {
                    parsed = -parsed;
                }

                return static_cast<TResult>(parsed);
            }
        };
    } // namespace detail

    class IniSection
    {
    public:
        IniSection() = default;

        bool exists(span<char> key) const;
        bool exists(const char *key) const;
#if defined(LW_INI_READER_HAS_ARDUINO_STRING)
        bool exists(const String &key) const;
#endif

        span<char> getRaw(span<char> key) const;
        span<char> getRaw(const char *key) const;
#if defined(LW_INI_READER_HAS_ARDUINO_STRING)
        span<char> getRaw(const String &key) const;
#endif

        template <typename TResult>
        TResult get(span<char> key) const
        {
            return detail::IniValueParser<TResult>::parse(getRaw(key));
        }

        template <typename TResult>
        bool test(span<char> key,
                  const TResult &expected) const
        {
            if (!exists(key))
            {
                return false;
            }

            return get<TResult>(key) == expected;
        }

        template <typename TResult>
        bool test(span<char> key) const
        {
            if (!exists(key))
            {
                return false;
            }

            return static_cast<bool>(get<TResult>(key));
        }

        template <typename TResult>
        TResult get(const char *key) const
        {
            return get<TResult>(detail::iniSpanFromCStr(key));
        }

        template <typename TResult>
        bool test(const char *key,
                  const TResult &expected) const
        {
            return test<TResult>(detail::iniSpanFromCStr(key), expected);
        }

        template <typename TResult>
        bool test(const char *key) const
        {
            return test<TResult>(detail::iniSpanFromCStr(key));
        }

#if defined(LW_INI_READER_HAS_ARDUINO_STRING)
        template <typename TResult>
        TResult get(const String &key) const
        {
            return get<TResult>(detail::iniSpanFromArduinoString(key));
        }

        template <typename TResult>
        bool test(const String &key,
                  const TResult &expected) const
        {
            return test<TResult>(detail::iniSpanFromArduinoString(key), expected);
        }

        template <typename TResult>
        bool test(const String &key) const
        {
            return test<TResult>(detail::iniSpanFromArduinoString(key));
        }
#endif

    private:
        friend class IniReader;

        explicit IniSection(const IniReader *reader,
                            size_t sectionIndex)
            : _reader(reader),
              _sectionIndex(sectionIndex)
        {
        }

        const IniReader *_reader{nullptr};
        size_t _sectionIndex{0};
    };

    class IniReader
    {
    public:
        using Reader = IniReader;
        using Section = IniSection;

        static Reader parse(span<char> input)
        {
            Reader reader{};
            reader._input = input;
            reader.parseIntoExtents();
            return reader;
        }

        bool exists(span<char> section) const
        {
            return findSectionIndex(resolveSectionName(section)) != npos;
        }

        bool exists(span<char> section,
                    span<char> key) const
        {
            return get(section).exists(key);
        }

        bool exists(const char *section) const
        {
            return exists(detail::iniSpanFromCStr(section));
        }

        bool exists(const char *section,
                    const char *key) const
        {
            return exists(detail::iniSpanFromCStr(section), detail::iniSpanFromCStr(key));
        }

#if defined(LW_INI_READER_HAS_ARDUINO_STRING)
        bool exists(const String &section) const
        {
            return exists(detail::iniSpanFromArduinoString(section));
        }

        bool exists(const String &section,
                    const String &key) const
        {
            return exists(detail::iniSpanFromArduinoString(section), detail::iniSpanFromArduinoString(key));
        }
#endif

        Section get(span<char> section) const
        {
            const size_t sectionIndex = findSectionIndex(resolveSectionName(section));
            if (sectionIndex == npos)
            {
                return Section{};
            }

            return Section(this, sectionIndex);
        }

        Section get(const char *section) const
        {
            return get(detail::iniSpanFromCStr(section));
        }

#if defined(LW_INI_READER_HAS_ARDUINO_STRING)
        Section get(const String &section) const
        {
            return get(detail::iniSpanFromArduinoString(section));
        }
#endif

        Reader getReader(span<char> sectionPrefix) const
        {
            Reader child = *this;
            child._readerPrefixStorage = child.resolveSectionName(sectionPrefix);
            child._readerPrefix = span<char>{child._readerPrefixStorage.data(), child._readerPrefixStorage.size()};
            return child;
        }

        Reader getReader(const char *sectionPrefix) const
        {
            return getReader(detail::iniSpanFromCStr(sectionPrefix));
        }

#if defined(LW_INI_READER_HAS_ARDUINO_STRING)
        Reader getReader(const String &sectionPrefix) const
        {
            return getReader(detail::iniSpanFromArduinoString(sectionPrefix));
        }
#endif

        template <typename TResult>
        TResult get(span<char> section,
                    span<char> key) const
        {
            return get(section).template get<TResult>(key);
        }

        span<char> getRaw(span<char> section,
                          span<char> key) const
        {
            return get(section).getRaw(key);
        }

        template <typename TResult>
        bool test(span<char> section,
                  span<char> key,
                  const TResult &expected) const
        {
            return get(section).template test<TResult>(key, expected);
        }

        template <typename TResult>
        bool test(span<char> section,
                  span<char> key) const
        {
            return get(section).template test<TResult>(key);
        }

        template <typename TResult>
        TResult get(const char *section,
                    const char *key) const
        {
            return get<TResult>(detail::iniSpanFromCStr(section), detail::iniSpanFromCStr(key));
        }

        span<char> getRaw(const char *section,
                          const char *key) const
        {
            return getRaw(detail::iniSpanFromCStr(section), detail::iniSpanFromCStr(key));
        }

        template <typename TResult>
        bool test(const char *section,
                  const char *key,
                  const TResult &expected) const
        {
            return test<TResult>(detail::iniSpanFromCStr(section), detail::iniSpanFromCStr(key), expected);
        }

        template <typename TResult>
        bool test(const char *section,
                  const char *key) const
        {
            return test<TResult>(detail::iniSpanFromCStr(section), detail::iniSpanFromCStr(key));
        }

#if defined(LW_INI_READER_HAS_ARDUINO_STRING)
        template <typename TResult>
        TResult get(const String &section,
                    const String &key) const
        {
            return get<TResult>(detail::iniSpanFromArduinoString(section), detail::iniSpanFromArduinoString(key));
        }

        span<char> getRaw(const String &section,
                          const String &key) const
        {
            return getRaw(detail::iniSpanFromArduinoString(section), detail::iniSpanFromArduinoString(key));
        }

        template <typename TResult>
        bool test(const String &section,
                  const String &key,
                  const TResult &expected) const
        {
            return test<TResult>(detail::iniSpanFromArduinoString(section), detail::iniSpanFromArduinoString(key), expected);
        }

        template <typename TResult>
        bool test(const String &section,
                  const String &key) const
        {
            return test<TResult>(detail::iniSpanFromArduinoString(section), detail::iniSpanFromArduinoString(key));
        }
#endif

    private:
        friend class IniSection;

        static constexpr size_t npos = static_cast<size_t>(-1);

        struct Extent
        {
            size_t offset{0};
            size_t length{0};
        };

        struct KeyValueExtent
        {
            Extent key{};
            Extent value{};
        };

        struct SectionExtent
        {
            Extent name{};
            Extent parentName{};
            std::vector<KeyValueExtent> entries{};
        };

        // Internal parser/indexing intentionally operates on span<char>-derived
        // extents only; no std::string or const char* parser state is retained.

        span<char> _input{};
        std::vector<SectionExtent> _sections{};
        std::vector<char> _readerPrefixStorage{};
        span<char> _readerPrefix{};

        span<char> extentSpan(const Extent &extent) const
        {
            if (_input.data() == nullptr || extent.length == 0)
            {
                return span<char>{};
            }

            return span<char>{_input.data() + extent.offset, extent.length};
        }

        static Extent makeExtent(span<char> whole,
                                 span<char> part)
        {
            if (whole.data() == nullptr || part.data() == nullptr || part.empty())
            {
                return Extent{};
            }

            const ptrdiff_t delta = part.data() - whole.data();
            if (delta < 0)
            {
                return Extent{};
            }

            return Extent{static_cast<size_t>(delta), part.size()};
        }

        static void appendSpan(std::vector<char> &output,
                               span<char> value)
        {
            if (value.empty())
            {
                return;
            }

            const size_t previousSize = output.size();
            output.resize(previousSize + value.size());
            for (size_t index = 0; index < value.size(); ++index)
            {
                output[previousSize + index] = value[index];
            }
        }

        std::vector<char> composePrefixedSectionName(span<char> section,
                                                     span<char> prefix) const
        {
            section = detail::iniTrimAscii(section);
            prefix = detail::iniTrimAscii(prefix);

            std::vector<char> composed{};
            appendSpan(composed, _readerPrefix);

            if (!prefix.empty())
            {
                if (!composed.empty())
                {
                    composed.push_back(':');
                }
                appendSpan(composed, prefix);
            }

            if (!section.empty())
            {
                if (!composed.empty())
                {
                    composed.push_back(':');
                }
                appendSpan(composed, section);
            }

            return composed;
        }

        std::vector<char> resolveSectionName(span<char> section) const
        {
            return composePrefixedSectionName(section, span<char>{});
        }

        size_t findSectionIndex(const std::vector<char> &name) const
        {
            if (name.empty())
            {
                return npos;
            }

            span<char> query{const_cast<char *>(name.data()), name.size()};
            return findSectionIndex(query);
        }

        size_t findSectionIndex(span<char> query) const
        {
            if (query.empty())
            {
                return npos;
            }

            for (size_t index = 0; index < _sections.size(); ++index)
            {
                if (detail::iniEqualsIgnoreCase(extentSpan(_sections[index].name), query))
                {
                    return index;
                }
            }

            return npos;
        }

        size_t findOrCreateSection(span<char> sectionName,
                                   span<char> parentName)
        {
            for (size_t index = 0; index < _sections.size(); ++index)
            {
                if (detail::iniEqualsIgnoreCase(extentSpan(_sections[index].name), sectionName))
                {
                    _sections[index].parentName = makeExtent(_input, parentName);
                    return index;
                }
            }

            SectionExtent section{};
            section.name = makeExtent(_input, sectionName);
            section.parentName = makeExtent(_input, parentName);
            _sections.push_back(section);
            return _sections.size() - 1;
        }

        size_t findKeyIndex(size_t sectionIndex,
                            span<char> key) const
        {
            if (sectionIndex >= _sections.size())
            {
                return npos;
            }

            key = detail::iniTrimAscii(key);
            if (key.empty())
            {
                return npos;
            }

            const auto &section = _sections[sectionIndex];
            for (size_t index = 0; index < section.entries.size(); ++index)
            {
                if (detail::iniEqualsIgnoreCase(extentSpan(section.entries[index].key), key))
                {
                    return index;
                }
            }

            return npos;
        }

        size_t findParentSectionIndex(size_t sectionIndex) const
        {
            if (sectionIndex >= _sections.size())
            {
                return npos;
            }

            const span<char> parentName = detail::iniTrimAscii(extentSpan(_sections[sectionIndex].parentName));
            if (parentName.empty())
            {
                return npos;
            }

            return findSectionIndex(parentName);
        }

        bool findKeyWithInheritance(size_t sectionIndex,
                                    span<char> key,
                                    size_t &resolvedSectionIndex,
                                    size_t &resolvedKeyIndex) const
        {
            if (sectionIndex >= _sections.size())
            {
                return false;
            }

            std::vector<uint8_t> visited(_sections.size(), 0);
            size_t currentSectionIndex = sectionIndex;

            while (currentSectionIndex != npos)
            {
                if (currentSectionIndex >= _sections.size())
                {
                    return false;
                }

                if (visited[currentSectionIndex] != 0)
                {
                    return false;
                }

                visited[currentSectionIndex] = 1;

                const size_t keyIndex = findKeyIndex(currentSectionIndex, key);
                if (keyIndex != npos)
                {
                    resolvedSectionIndex = currentSectionIndex;
                    resolvedKeyIndex = keyIndex;
                    return true;
                }

                currentSectionIndex = findParentSectionIndex(currentSectionIndex);
            }

            return false;
        }

        void parseIntoExtents()
        {
            if (_input.data() == nullptr || _input.empty())
            {
                return;
            }

            size_t currentSection = npos;
            size_t cursor = 0;

            while (cursor < _input.size())
            {
                const size_t lineStart = cursor;
                while (cursor < _input.size() && _input[cursor] != '\n')
                {
                    ++cursor;
                }

                span<char> line{_input.data() + lineStart, cursor - lineStart};
                if (!line.empty() && line[line.size() - 1] == '\r')
                {
                    line = span<char>{line.data(), line.size() - 1};
                }
                line = detail::iniTrimAscii(line);

                if (!line.empty() && line[0] != ';' && line[0] != '#')
                {
                    if (line.size() >= 2 && line[0] == '[' && line[line.size() - 1] == ']')
                    {
                        span<char> sectionName{line.data() + 1, line.size() - 2};
                        sectionName = detail::iniTrimAscii(sectionName);

                        span<char> parentName{};
                        if (!sectionName.empty())
                        {
                            char *delimiter = sectionName.data();
                            const char *sectionEnd = sectionName.data() + sectionName.size();
                            while (delimiter < sectionEnd && *delimiter != '&')
                            {
                                ++delimiter;
                            }

                            if (delimiter < sectionEnd)
                            {
                                parentName = span<char>{delimiter + 1, static_cast<size_t>(sectionEnd - (delimiter + 1))};
                                sectionName = span<char>{sectionName.data(), static_cast<size_t>(delimiter - sectionName.data())};
                                sectionName = detail::iniTrimAscii(sectionName);
                                parentName = detail::iniTrimAscii(parentName);
                            }
                        }

                        if (!sectionName.empty())
                        {
                            currentSection = findOrCreateSection(sectionName, parentName);
                        }
                    }
                    else if (currentSection != npos)
                    {
                        char *equals = line.data();
                        char *lineEnd = line.data() + line.size();
                        while (equals < lineEnd && *equals != '=')
                        {
                            ++equals;
                        }

                        if (equals < lineEnd)
                        {
                            span<char> key{line.data(), static_cast<size_t>(equals - line.data())};
                            span<char> value{equals + 1, static_cast<size_t>(lineEnd - (equals + 1))};
                            key = detail::iniTrimAscii(key);
                            value = detail::iniTrimAscii(value);

                            if (!key.empty())
                            {
                                KeyValueExtent kv{};
                                kv.key = makeExtent(_input, key);
                                kv.value = makeExtent(_input, value);
                                _sections[currentSection].entries.push_back(kv);
                            }
                        }
                    }
                }

                if (cursor < _input.size() && _input[cursor] == '\n')
                {
                    ++cursor;
                }
            }
        }
    };

    inline bool IniSection::exists(span<char> key) const
    {
        if (_reader == nullptr)
        {
            return false;
        }

        size_t resolvedSectionIndex = IniReader::npos;
        size_t resolvedKeyIndex = IniReader::npos;
        return _reader->findKeyWithInheritance(_sectionIndex, key, resolvedSectionIndex, resolvedKeyIndex);
    }

    inline bool IniSection::exists(const char *key) const
    {
        return exists(detail::iniSpanFromCStr(key));
    }

#if defined(LW_INI_READER_HAS_ARDUINO_STRING)
    inline bool IniSection::exists(const String &key) const
    {
        return exists(detail::iniSpanFromArduinoString(key));
    }
#endif

    inline span<char> IniSection::getRaw(span<char> key) const
    {
        if (_reader == nullptr)
        {
            return span<char>{};
        }

        size_t resolvedSectionIndex = IniReader::npos;
        size_t resolvedKeyIndex = IniReader::npos;
        if (!_reader->findKeyWithInheritance(_sectionIndex, key, resolvedSectionIndex, resolvedKeyIndex))
        {
            return span<char>{};
        }

        return _reader->extentSpan(_reader->_sections[resolvedSectionIndex].entries[resolvedKeyIndex].value);
    }

    inline span<char> IniSection::getRaw(const char *key) const
    {
        return getRaw(detail::iniSpanFromCStr(key));
    }

#if defined(LW_INI_READER_HAS_ARDUINO_STRING)
    inline span<char> IniSection::getRaw(const String &key) const
    {
        return getRaw(detail::iniSpanFromArduinoString(key));
    }
#endif

    using Reader = IniReader;
    using Section = IniSection;

} // namespace factory
} // namespace lw
