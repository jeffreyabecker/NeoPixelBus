#pragma once

#include <cstddef>
#include <cstdint>

#include "core/Compat.h"
#include "colors/ChannelOrder.h"
#include "colors/ColorChannelIndexIterator.h"

namespace npb
{

    class ColorHexCodec
    {
    public:
        template <typename TColor>
        static TColor parseHex(const char *str,
                               const char *colorOrder = nullptr)
        {
            TColor result{};
            if (str == nullptr)
            {
                return result;
            }

            if (colorOrder == nullptr)
            {
                colorOrder = defaultColorOrder<TColor>();
            }

            if (colorOrder == nullptr)
            {
                return result;
            }

            const char *cursor = str;
            if (*cursor == '#')
            {
                ++cursor;
            }
            else if (cursor[0] == '0' && (cursor[1] == 'x' || cursor[1] == 'X'))
            {
                cursor += 2;
            }

            constexpr size_t DigitsPerComponent = sizeof(typename TColor::ComponentType) * 2;
            constexpr size_t ChannelCount = static_cast<size_t>(TColor::ChannelCount);

            for (size_t logicalChannel = 0; logicalChannel < ChannelCount; ++logicalChannel)
            {
                const char channelTag = colorOrder[logicalChannel];
                if (channelTag == '\0' || !ColorChannelIndexRange<ChannelCount>::isSupportedChannelTag(channelTag))
                {
                    return TColor{};
                }

                const size_t channelIndex = ColorChannelIndexRange<ChannelCount>::indexFromChannel(channelTag);
                if (channelIndex >= ChannelCount)
                {
                    return TColor{};
                }

                typename TColor::ComponentType value = 0;
                for (size_t digit = 0; digit < DigitsPerComponent; ++digit)
                {
                    while (isHexSeparator(*cursor))
                    {
                        ++cursor;
                    }

                    const int nibble = hexNibble(*cursor);
                    if (nibble < 0)
                    {
                        return TColor{};
                    }

                    value = static_cast<typename TColor::ComponentType>(
                        (value << 4) | static_cast<typename TColor::ComponentType>(nibble));
                    ++cursor;
                }

                result[channelTag] = value;
            }

            return result;
        }

        template <typename TColor>
        static void fillHex(const TColor &color,
                            span<uint8_t> resultBuffer,
                            const char *colorOrder = nullptr,
                            const char *prefix = nullptr)
        {
            if (resultBuffer.empty())
            {
                return;
            }

            for (size_t idx = 0; idx < resultBuffer.size(); ++idx)
            {
                resultBuffer[idx] = 0;
            }

            if (colorOrder == nullptr)
            {
                colorOrder = defaultColorOrder<TColor>();
            }

            if (colorOrder == nullptr)
            {
                return;
            }

            size_t out = 0;

            if (prefix != nullptr)
            {
                for (size_t idx = 0; prefix[idx] != '\0' && out < resultBuffer.size(); ++idx)
                {
                    resultBuffer[out++] = static_cast<uint8_t>(prefix[idx]);
                }
            }

            constexpr size_t DigitsPerComponent = sizeof(typename TColor::ComponentType) * 2;
            constexpr size_t ChannelCount = static_cast<size_t>(TColor::ChannelCount);

            for (size_t logicalChannel = 0; logicalChannel < ChannelCount; ++logicalChannel)
            {
                const char channelTag = colorOrder[logicalChannel];
                if (channelTag == '\0' || !ColorChannelIndexRange<ChannelCount>::isSupportedChannelTag(channelTag))
                {
                    return;
                }

                const size_t channelIndex = ColorChannelIndexRange<ChannelCount>::indexFromChannel(channelTag);
                if (channelIndex >= ChannelCount)
                {
                    return;
                }

                const typename TColor::ComponentType value = color[channelTag];
                (void)channelIndex;

                for (size_t digit = 0; digit < DigitsPerComponent; ++digit)
                {
                    if (out >= resultBuffer.size())
                    {
                        return;
                    }

                    const size_t nibbleShift = (DigitsPerComponent - 1 - digit) * 4;
                    const uint8_t nibble = static_cast<uint8_t>((value >> nibbleShift) & static_cast<typename TColor::ComponentType>(0x0F));
                    resultBuffer[out++] = static_cast<uint8_t>(hexChar(nibble));
                }
            }
        }

    private:
        template <typename TColor>
        static constexpr const char *defaultColorOrder()
        {
            constexpr size_t ChannelCount = static_cast<size_t>(TColor::ChannelCount);

            if constexpr (ChannelCount >= 5)
            {
                return ChannelOrder::RGBCW::value;
            }

            if constexpr (ChannelCount == 4)
            {
                return ChannelOrder::RGBW::value;
            }

            return ChannelOrder::RGB::value;
        }

        static constexpr char hexChar(uint8_t nibble)
        {
            return (nibble < 10) ? static_cast<char>('0' + nibble) : static_cast<char>('A' + (nibble - 10));
        }

        static constexpr bool isHexSeparator(char value)
        {
            return value == ' ' || value == '_' || value == ':' || value == '-';
        }

        static constexpr int hexNibble(char value)
        {
            if (value >= '0' && value <= '9')
            {
                return value - '0';
            }

            if (value >= 'a' && value <= 'f')
            {
                return 10 + (value - 'a');
            }

            if (value >= 'A' && value <= 'F')
            {
                return 10 + (value - 'A');
            }

            return -1;
        }
    };

} // namespace npb
