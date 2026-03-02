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
    struct Hd108Serializer
    {
        static_assert(std::is_same<typename TColor::ComponentType, uint16_t>::value &&
                      (TColor::ChannelCount >= 3),
                      "Hd108Serializer requires uint16_t components and at least 3 channels.");

        using ColorType = TColor;
        static constexpr size_t ChannelCount = ColorType::ChannelCount;
        static constexpr size_t BytesPerPixel = 2 + (ChannelCount * 2);
        static constexpr size_t StartFrameSize = 16;
        static constexpr size_t EndFrameSize = 4;

        static size_t getBufferSize(uint16_t pixelCount)
        {
            return StartFrameSize + (static_cast<size_t>(pixelCount) * BytesPerPixel) + EndFrameSize;
        }

        static void initialize(span<uint8_t> buffer)
        {
            std::fill(buffer.begin(), buffer.begin() + StartFrameSize, 0x00);
            std::fill(buffer.end() - EndFrameSize, buffer.end(), 0xFF);
        }

        template <typename TInterfaceColor>
        static void serialize(span<uint8_t> buffer,
                              span<const TInterfaceColor> colors)
        {
            static_assert(TInterfaceColor::ChannelCount >= ChannelCount,
                          "Hd108Serializer source color must provide enough channels.");

            const uint16_t pixelCount = inferPixelCount(buffer.size());
            serialize(buffer,
                      colors,
                      pixelCount,
                      ChannelOrder::BGR::value);
        }

        template <typename TInterfaceColor>
        static void serialize(span<uint8_t> buffer,
                              span<const TInterfaceColor> colors,
                              uint16_t pixelCount,
                              const char *channelOrder)
        {
            static_assert(TInterfaceColor::ChannelCount >= ChannelCount,
                          "Hd108Serializer source color must provide enough channels.");

            size_t offset = StartFrameSize;
            const size_t pixelLimit = std::min(colors.size(), static_cast<size_t>(pixelCount));
            const char *effectiveChannelOrder = (nullptr != channelOrder) ? channelOrder : ChannelOrder::BGR::value;

            for (size_t index = 0; index < pixelLimit; ++index)
            {
                const auto &color = colors[index];
                buffer[offset++] = 0xFF;
                buffer[offset++] = 0xFF;

                for (size_t channel = 0; channel < ChannelCount; ++channel)
                {
                    const auto sourceValue = color[effectiveChannelOrder[channel]];
                    const uint16_t val = convertComponent(sourceValue);
                    buffer[offset++] = static_cast<uint8_t>(val >> 8);
                    buffer[offset++] = static_cast<uint8_t>(val & 0xFF);
                }
            }
        }

    private:
        template <typename TSourceComponent>
        static constexpr uint16_t convertComponent(TSourceComponent value)
        {
            if constexpr (std::is_same<TSourceComponent, uint16_t>::value)
            {
                return value;
            }
            else
            {
                return static_cast<uint16_t>((static_cast<uint16_t>(value) << 8) | static_cast<uint16_t>(value));
            }
        }

        static uint16_t inferPixelCount(size_t bufferSize)
        {
            if (bufferSize < (StartFrameSize + EndFrameSize))
            {
                return 0;
            }

            const size_t payloadSize = bufferSize - (StartFrameSize + EndFrameSize);
            if (payloadSize % BytesPerPixel != 0)
            {
                return 0;
            }

            const size_t pixelCount = payloadSize / BytesPerPixel;
            if (pixelCount > 0xFFFFu)
            {
                return 0;
            }

            return static_cast<uint16_t>(pixelCount);
        }
    };

} // namespace lw
