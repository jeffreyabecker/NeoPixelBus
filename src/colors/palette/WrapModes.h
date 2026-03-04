#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace lw
{
    struct WrapClamp
    {
        static constexpr uint16_t distance(uint8_t left, uint8_t right)
        {
            return (left >= right)
                       ? static_cast<uint16_t>(left - right)
                       : static_cast<uint16_t>(right - left);
        }

        static constexpr uint8_t mapPositionToPaletteIndex(size_t pixelIndex,
                                                           size_t pixelCount)
        {
            if (pixelCount == 0)
            {
                return 0;
            }

            if (pixelCount <= 1)
            {
                return 0;
            }

            const size_t clamped = (pixelIndex >= pixelCount) ? (pixelCount - 1) : pixelIndex;
            return static_cast<uint8_t>((clamped * 255ull) / (pixelCount - 1));
        }
    };

    struct WrapCircular
    {
        static constexpr uint16_t distance(uint8_t left, uint8_t right)
        {
            const uint16_t direct = WrapClamp::distance(left, right);
            return std::min<uint16_t>(direct, static_cast<uint16_t>(256 - direct));
        }

        static constexpr uint8_t mapPositionToPaletteIndex(size_t pixelIndex,
                                                           size_t pixelCount)
        {
            if (pixelCount == 0)
            {
                return 0;
            }

            const size_t wrapped = pixelIndex % pixelCount;
            return static_cast<uint8_t>((wrapped * 256ull) / pixelCount);
        }
    };

    struct WrapMirror
    {
        static constexpr uint16_t distance(uint8_t left, uint8_t right)
        {
            return WrapClamp::distance(left, right);
        }

        static constexpr uint8_t mapPositionToPaletteIndex(size_t pixelIndex,
                                                           size_t pixelCount)
        {
            if (pixelCount <= 1)
            {
                return 0;
            }

            const size_t period = (pixelCount - 1) * 2;
            const size_t pos = (period == 0) ? 0 : (pixelIndex % period);
            const size_t mirrored = (pos <= (pixelCount - 1)) ? pos : (period - pos);
            return static_cast<uint8_t>((mirrored * 255ull) / (pixelCount - 1));
        }
    };

    struct WrapHoldFirst
    {
        static constexpr uint16_t distance(uint8_t left, uint8_t right)
        {
            return WrapClamp::distance(left, right);
        }

        static constexpr uint8_t mapPositionToPaletteIndex(size_t pixelIndex,
                                                           size_t pixelCount)
        {
            if (pixelCount == 0 || pixelIndex >= pixelCount)
            {
                return 0;
            }

            if (pixelCount <= 1)
            {
                return 0;
            }

            return static_cast<uint8_t>((pixelIndex * 255ull) / (pixelCount - 1));
        }
    };

    struct WrapHoldLast
    {
        static constexpr uint16_t distance(uint8_t left, uint8_t right)
        {
            return WrapClamp::distance(left, right);
        }

        static constexpr uint8_t mapPositionToPaletteIndex(size_t pixelIndex,
                                                           size_t pixelCount)
        {
            if (pixelCount == 0)
            {
                return 255;
            }

            if (pixelCount <= 1)
            {
                return 255;
            }

            if (pixelIndex >= pixelCount)
            {
                return 255;
            }

            return static_cast<uint8_t>((pixelIndex * 255ull) / (pixelCount - 1));
        }
    };

    struct WrapBlackout
    {
        static constexpr uint16_t distance(uint8_t left, uint8_t right)
        {
            return WrapClamp::distance(left, right);
        }

        static constexpr bool isOutOfRange(size_t pixelIndex,
                                           size_t pixelCount)
        {
            return pixelCount == 0 || pixelIndex >= pixelCount;
        }

        static constexpr uint8_t mapPositionToPaletteIndex(size_t pixelIndex,
                                                           size_t pixelCount)
        {
            if (isOutOfRange(pixelIndex, pixelCount))
            {
                return 0;
            }

            if (pixelCount <= 1)
            {
                return 0;
            }

            return static_cast<uint8_t>((pixelIndex * 255ull) / (pixelCount - 1));
        }
    };

    template <uint8_t TStart = 0,
              uint8_t TEnd = 255>
    struct WrapWindow
    {
        static_assert(TStart <= TEnd, "WrapWindow requires TStart <= TEnd");

        static constexpr uint16_t distance(uint8_t left, uint8_t right)
        {
            return WrapClamp::distance(left, right);
        }

        static constexpr uint8_t mapPositionToPaletteIndex(size_t pixelIndex,
                                                           size_t pixelCount)
        {
            if (pixelCount == 0)
            {
                return TStart;
            }

            if (pixelCount <= 1)
            {
                return TStart;
            }

            const size_t clamped = (pixelIndex >= pixelCount) ? (pixelCount - 1) : pixelIndex;
            const uint16_t span = static_cast<uint16_t>(TEnd - TStart);
            const uint16_t mapped = static_cast<uint16_t>((clamped * span) / (pixelCount - 1));
            return static_cast<uint8_t>(TStart + mapped);
        }
    };

    template <uint8_t TStart = 0,
              uint8_t TEnd = 255>
    struct WrapModuloSpan
    {
        static_assert(TStart <= TEnd, "WrapModuloSpan requires TStart <= TEnd");

        static constexpr uint16_t distance(uint8_t left, uint8_t right)
        {
            return WrapClamp::distance(left, right);
        }

        static constexpr uint8_t mapPositionToPaletteIndex(size_t pixelIndex,
                                                           size_t)
        {
            const uint16_t span = static_cast<uint16_t>(TEnd - TStart + 1u);
            const uint16_t offset = static_cast<uint16_t>(pixelIndex % span);
            return static_cast<uint8_t>(TStart + offset);
        }
    };

    template <uint8_t TOffset = 0>
    struct WrapOffsetCircular
    {
        static constexpr uint16_t distance(uint8_t left, uint8_t right)
        {
            return WrapCircular::distance(left, right);
        }

        static constexpr uint8_t mapPositionToPaletteIndex(size_t pixelIndex,
                                                           size_t pixelCount)
        {
            return static_cast<uint8_t>(WrapCircular::mapPositionToPaletteIndex(pixelIndex, pixelCount) + TOffset);
        }
    };

} // namespace lw
