#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <algorithm>

#include "core/Compat.h"
#include "colors/Color.h"

namespace lw
{

    template <typename TColor>
    struct GeneralDotStarSerializer
    {
        static_assert(std::is_same<typename TColor::ComponentType, uint8_t>::value,
                      "GeneralDotStarSerializer requires uint8_t color components.");
        static_assert(TColor::ChannelCount >= 3 && TColor::ChannelCount <= 5,
                      "GeneralDotStarSerializer requires color channel count in [3, 5].");

        using ColorType = TColor;
        static constexpr size_t ChannelCount = ColorType::ChannelCount;
        static constexpr size_t BytesPerPixel = 1 + ChannelCount;
        static constexpr size_t StartFrameSize = 4;
        static constexpr size_t EndFrameFixedSize = 4;

        static size_t getBufferSize(uint16_t pixelCount)
        {
            const size_t extraEndBytes = static_cast<size_t>((pixelCount + 15u) / 16u);
            return StartFrameSize +
                   (static_cast<size_t>(pixelCount) * BytesPerPixel) +
                   EndFrameFixedSize +
                   extraEndBytes;
        }

        static void initialize(span<uint8_t> buffer)
        {
            const uint16_t pixelCount = inferPixelCount(buffer.size());
            initialize(buffer, pixelCount);
        }

        static void initialize(span<uint8_t> buffer,
                               uint16_t pixelCount)
        {
            const size_t extraEndBytes = static_cast<size_t>((pixelCount + 15u) / 16u);
            std::fill(buffer.begin(), buffer.begin() + StartFrameSize, 0x00);
            std::fill(buffer.end() - (EndFrameFixedSize + extraEndBytes), buffer.end(), 0x00);
        }

        static void serialize(span<uint8_t> buffer,
                              span<const ColorType> colors)
        {
            const uint16_t pixelCount = inferPixelCount(buffer.size());
            serialize(buffer,
                      colors,
                      pixelCount,
                      ChannelOrder::BGR::value);
        }

        static void serialize(span<uint8_t> buffer,
                              span<const ColorType> colors,
                              uint16_t pixelCount,
                              const char *channelOrder)
        {
            size_t offset = StartFrameSize;
            const size_t pixelLimit = std::min(colors.size(), static_cast<size_t>(pixelCount));
            const char *effectiveChannelOrder = (nullptr != channelOrder) ? channelOrder : ChannelOrder::BGR::value;

            for (size_t index = 0; index < pixelLimit; ++index)
            {
                const auto &color = colors[index];
                buffer[offset++] = 0xFF;
                for (size_t channel = 0; channel < ChannelCount; ++channel)
                {
                    buffer[offset++] = color[effectiveChannelOrder[channel]];
                }
            }
        }

    private:
        static uint16_t inferPixelCount(size_t bufferSize)
        {
            if (bufferSize < (StartFrameSize + EndFrameFixedSize))
            {
                return 0;
            }

            for (size_t candidate = 0; candidate <= 0xFFFFu; ++candidate)
            {
                if (getBufferSize(static_cast<uint16_t>(candidate)) == bufferSize)
                {
                    return static_cast<uint16_t>(candidate);
                }

                if (getBufferSize(static_cast<uint16_t>(candidate)) > bufferSize)
                {
                    break;
                }
            }

            return 0;
        }
    };

} // namespace lw
