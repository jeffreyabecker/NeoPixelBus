#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "core/Compat.h"
#include "colors/Palette.h"

namespace lw
{
    enum class PaletteCodecError : uint8_t
    {
        None = 0,
        InputTooSmall,
        InvalidMagic,
        InvalidLength,
        UnsupportedVersion,
        InvalidFlags,
        InvalidComponentBytes,
        InvalidChannelCount,
        InvalidStopCount,
        OutputTooSmall,
        InvalidChecksum,
        InvalidStopOrder,
        UnsupportedColorType,
        EmptyPalette
    };

    struct PaletteCodecConstants
    {
        static constexpr uint8_t Magic0 = 0x4Cu;
        static constexpr uint8_t Magic1 = 0x50u;
        static constexpr uint16_t Version1 = 0x0001u;

        static constexpr size_t OffsetMagic = 0;
        static constexpr size_t OffsetLength = 2;
        static constexpr size_t OffsetVersion = 4;
        static constexpr size_t OffsetFlags = 6;
        static constexpr size_t OffsetStopCount = 8;
        static constexpr size_t HeaderSize = 9;
        static constexpr size_t CrcSize = 2;

        static constexpr uint16_t ComponentBytesMask = 0x000Fu;
        static constexpr uint16_t ChannelCountMask = 0x0070u;
        static constexpr uint16_t ReservedMask = 0xFF80u;
    };

    namespace detail
    {
        constexpr uint16_t readU16LE(span<const uint8_t> bytes, size_t offset)
        {
            return static_cast<uint16_t>(
                static_cast<uint16_t>(bytes[offset]) |
                (static_cast<uint16_t>(bytes[offset + 1]) << 8u));
        }

        constexpr void writeU16LE(span<uint8_t> bytes, size_t offset, uint16_t value)
        {
            bytes[offset] = static_cast<uint8_t>(value & 0xFFu);
            bytes[offset + 1] = static_cast<uint8_t>((value >> 8u) & 0xFFu);
        }

        constexpr uint16_t crc16CcittFalse(span<const uint8_t> bytes)
        {
            uint16_t crc = 0xFFFFu;

            for (size_t i = 0; i < bytes.size(); ++i)
            {
                crc ^= static_cast<uint16_t>(bytes[i]) << 8u;
                for (uint8_t bit = 0; bit < 8u; ++bit)
                {
                    if ((crc & 0x8000u) != 0u)
                    {
                        crc = static_cast<uint16_t>((crc << 1u) ^ 0x1021u);
                    }
                    else
                    {
                        crc = static_cast<uint16_t>(crc << 1u);
                    }
                }
            }

            return crc;
        }

        constexpr bool validComponentBytes(uint8_t componentBytes)
        {
            return componentBytes == 1u || componentBytes == 2u;
        }

        constexpr bool validChannelCount(uint8_t channelCount)
        {
            return channelCount == 3u || channelCount == 4u || channelCount == 5u;
        }
    } // namespace detail

    constexpr size_t encodedPaletteBinarySize(size_t stopCount,
                                              uint8_t channelCount,
                                              uint8_t componentBytes)
    {
        return PaletteCodecConstants::HeaderSize +
               (stopCount * (1u + (static_cast<size_t>(channelCount) * static_cast<size_t>(componentBytes)))) +
               PaletteCodecConstants::CrcSize;
    }

    template <typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr PaletteCodecError encodePaletteBinary(const Palette<TColor> &palette,
                                                    span<uint8_t> output,
                                                    size_t &bytesWritten)
    {
        bytesWritten = 0;

        const auto stops = palette.stops();
        if (stops.empty())
        {
            return PaletteCodecError::EmptyPalette;
        }

        if (stops.size() < 2u || stops.size() > 255u)
        {
            return PaletteCodecError::InvalidStopCount;
        }

        constexpr uint8_t channelCount = static_cast<uint8_t>(TColor::ChannelCount);
        constexpr uint8_t componentBytes = static_cast<uint8_t>(sizeof(typename TColor::ComponentType));

        if (!detail::validChannelCount(channelCount))
        {
            return PaletteCodecError::InvalidChannelCount;
        }

        if (!detail::validComponentBytes(componentBytes))
        {
            return PaletteCodecError::InvalidComponentBytes;
        }

        const size_t expectedSize = encodedPaletteBinarySize(stops.size(), channelCount, componentBytes);
        if (output.size() < expectedSize)
        {
            return PaletteCodecError::OutputTooSmall;
        }

        output[PaletteCodecConstants::OffsetMagic + 0] = PaletteCodecConstants::Magic0;
        output[PaletteCodecConstants::OffsetMagic + 1] = PaletteCodecConstants::Magic1;
        detail::writeU16LE(output, PaletteCodecConstants::OffsetLength, static_cast<uint16_t>(expectedSize));
        detail::writeU16LE(output, PaletteCodecConstants::OffsetVersion, PaletteCodecConstants::Version1);

        const uint16_t flags = static_cast<uint16_t>(componentBytes) |
                               static_cast<uint16_t>(channelCount << 4u);
        detail::writeU16LE(output, PaletteCodecConstants::OffsetFlags, flags);
        output[PaletteCodecConstants::OffsetStopCount] = static_cast<uint8_t>(stops.size());

        size_t cursor = PaletteCodecConstants::HeaderSize;
        uint8_t previousIndex = 0;
        for (size_t i = 0; i < stops.size(); ++i)
        {
            const auto &stop = stops[i];
            if (i > 0u && stop.index <= previousIndex)
            {
                return PaletteCodecError::InvalidStopOrder;
            }

            output[cursor++] = stop.index;

            for (size_t channel = 0; channel < static_cast<size_t>(channelCount); ++channel)
            {
                const char tag = TColor::ChannelIndexIterator::channelAt(channel);
                const auto componentValue = static_cast<uint32_t>(stop.color[tag]);

                if (componentBytes == 1u)
                {
                    output[cursor++] = static_cast<uint8_t>(componentValue & 0xFFu);
                }
                else
                {
                    output[cursor++] = static_cast<uint8_t>(componentValue & 0xFFu);
                    output[cursor++] = static_cast<uint8_t>((componentValue >> 8u) & 0xFFu);
                }
            }

            previousIndex = stop.index;
        }

        const uint16_t crc = detail::crc16CcittFalse(span<const uint8_t>(output.data(), expectedSize - PaletteCodecConstants::CrcSize));
        detail::writeU16LE(output, expectedSize - PaletteCodecConstants::CrcSize, crc);

        bytesWritten = expectedSize;
        return PaletteCodecError::None;
    }

    template <typename TColor,
              typename = std::enable_if_t<ColorType<TColor>>>
    constexpr PaletteCodecError decodePaletteBinary(span<const uint8_t> input,
                                                    span<PaletteStop<TColor>> outputStops,
                                                    Palette<TColor> &decodedPalette,
                                                    size_t &decodedStopCount)
    {
        decodedStopCount = 0;
        decodedPalette = Palette<TColor>{};

        if (input.size() < PaletteCodecConstants::HeaderSize + PaletteCodecConstants::CrcSize)
        {
            return PaletteCodecError::InputTooSmall;
        }

        if (input[PaletteCodecConstants::OffsetMagic + 0] != PaletteCodecConstants::Magic0 ||
            input[PaletteCodecConstants::OffsetMagic + 1] != PaletteCodecConstants::Magic1)
        {
            return PaletteCodecError::InvalidMagic;
        }

        const uint16_t lengthBytes = detail::readU16LE(input, PaletteCodecConstants::OffsetLength);
        if (lengthBytes > input.size())
        {
            return PaletteCodecError::InvalidLength;
        }

        const uint16_t version = detail::readU16LE(input, PaletteCodecConstants::OffsetVersion);
        if (version != PaletteCodecConstants::Version1)
        {
            return PaletteCodecError::UnsupportedVersion;
        }

        const uint16_t flags = detail::readU16LE(input, PaletteCodecConstants::OffsetFlags);
        if ((flags & PaletteCodecConstants::ReservedMask) != 0u)
        {
            return PaletteCodecError::InvalidFlags;
        }

        const uint8_t componentBytes = static_cast<uint8_t>(flags & PaletteCodecConstants::ComponentBytesMask);
        const uint8_t channelCount = static_cast<uint8_t>((flags & PaletteCodecConstants::ChannelCountMask) >> 4u);

        if (!detail::validComponentBytes(componentBytes))
        {
            return PaletteCodecError::InvalidComponentBytes;
        }

        if (!detail::validChannelCount(channelCount))
        {
            return PaletteCodecError::InvalidChannelCount;
        }

        if (channelCount != static_cast<uint8_t>(TColor::ChannelCount) ||
            componentBytes != static_cast<uint8_t>(sizeof(typename TColor::ComponentType)))
        {
            return PaletteCodecError::UnsupportedColorType;
        }

        const uint8_t stopCount = input[PaletteCodecConstants::OffsetStopCount];
        if (stopCount < 2u)
        {
            return PaletteCodecError::InvalidStopCount;
        }

        const size_t expectedSize = encodedPaletteBinarySize(stopCount, channelCount, componentBytes);
        if (lengthBytes != expectedSize || input.size() < expectedSize)
        {
            return PaletteCodecError::InvalidLength;
        }

        if (outputStops.size() < stopCount)
        {
            return PaletteCodecError::OutputTooSmall;
        }

        const uint16_t expectedCrc = detail::readU16LE(input, expectedSize - PaletteCodecConstants::CrcSize);
        const uint16_t computedCrc = detail::crc16CcittFalse(span<const uint8_t>(input.data(), expectedSize - PaletteCodecConstants::CrcSize));
        if (computedCrc != expectedCrc)
        {
            return PaletteCodecError::InvalidChecksum;
        }

        size_t cursor = PaletteCodecConstants::HeaderSize;
        uint8_t previousIndex = 0;
        for (size_t i = 0; i < stopCount; ++i)
        {
            PaletteStop<TColor> stop{};
            stop.index = input[cursor++];

            if (i > 0u && stop.index <= previousIndex)
            {
                return PaletteCodecError::InvalidStopOrder;
            }

            for (size_t channel = 0; channel < static_cast<size_t>(channelCount); ++channel)
            {
                const char tag = TColor::ChannelIndexIterator::channelAt(channel);
                uint32_t value = input[cursor++];
                if (componentBytes == 2u)
                {
                    value |= static_cast<uint32_t>(input[cursor++]) << 8u;
                }

                stop.color[tag] = static_cast<typename TColor::ComponentType>(value);
            }

            outputStops[i] = stop;
            previousIndex = stop.index;
        }

        decodedStopCount = stopCount;
        decodedPalette = Palette<TColor>(span<const PaletteStop<TColor>>(outputStops.data(), stopCount));
        return PaletteCodecError::None;
    }
} // namespace lw
