#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace lw::colors::palettes
{
constexpr size_t absoluteDistance(size_t left, size_t right)
{
    return (left >= right) ? (left - right) : (right - left);
}

struct WrapClamp
{
    static constexpr bool isOutOfRange(size_t, size_t) { return false; }

    static constexpr size_t distance(size_t left, size_t right, size_t = 255) { return absoluteDistance(left, right); }

    static constexpr size_t mapPositionToPaletteIndex(size_t pixelIndex, size_t pixelCount, size_t maxIndex = 255)
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
        return (clamped * maxIndex) / (pixelCount - 1);
    }
};

struct WrapCircular
{
    static constexpr bool isOutOfRange(size_t, size_t) { return false; }

    static constexpr size_t distance(size_t left, size_t right, size_t maxIndex = 255)
    {
        const size_t direct = WrapClamp::distance(left, right);
        const size_t span = maxIndex + 1;

        if (span == 0)
        {
            return direct;
        }

        if (direct >= span)
        {
            return direct % span;
        }

        return std::min(direct, span - direct);
    }

    static constexpr size_t mapPositionToPaletteIndex(size_t pixelIndex, size_t pixelCount, size_t maxIndex = 255)
    {
        if (pixelCount == 0)
        {
            return 0;
        }

        const size_t wrapped = pixelIndex % pixelCount;
        return (wrapped * (maxIndex + 1)) / pixelCount;
    }
};

struct WrapMirror
{
    static constexpr bool isOutOfRange(size_t, size_t) { return false; }

    static constexpr size_t distance(size_t left, size_t right, size_t = 255)
    {
        return WrapClamp::distance(left, right);
    }

    static constexpr size_t mapPositionToPaletteIndex(size_t pixelIndex, size_t pixelCount, size_t maxIndex = 255)
    {
        if (pixelCount <= 1)
        {
            return 0;
        }

        const size_t period = (pixelCount - 1) * 2;
        const size_t pos = (period == 0) ? 0 : (pixelIndex % period);
        const size_t mirrored = (pos <= (pixelCount - 1)) ? pos : (period - pos);
        return (mirrored * maxIndex) / (pixelCount - 1);
    }
};

struct WrapHoldFirst
{
    static constexpr bool isOutOfRange(size_t, size_t) { return false; }

    static constexpr size_t distance(size_t left, size_t right, size_t = 255)
    {
        return WrapClamp::distance(left, right);
    }

    static constexpr size_t mapPositionToPaletteIndex(size_t pixelIndex, size_t pixelCount, size_t maxIndex = 255)
    {
        if (pixelCount == 0 || pixelIndex >= pixelCount)
        {
            return 0;
        }

        if (pixelCount <= 1)
        {
            return 0;
        }

        return (pixelIndex * maxIndex) / (pixelCount - 1);
    }
};

struct WrapHoldLast
{
    static constexpr bool isOutOfRange(size_t, size_t) { return false; }

    static constexpr size_t distance(size_t left, size_t right, size_t = 255)
    {
        return WrapClamp::distance(left, right);
    }

    static constexpr size_t mapPositionToPaletteIndex(size_t pixelIndex, size_t pixelCount, size_t maxIndex = 255)
    {
        if (pixelCount == 0)
        {
            return maxIndex;
        }

        if (pixelCount <= 1)
        {
            return maxIndex;
        }

        if (pixelIndex >= pixelCount)
        {
            return maxIndex;
        }

        return (pixelIndex * maxIndex) / (pixelCount - 1);
    }
};

struct WrapBlackout
{
    static constexpr size_t distance(size_t left, size_t right, size_t = 255)
    {
        return WrapClamp::distance(left, right);
    }

    static constexpr bool isOutOfRange(size_t pixelIndex, size_t pixelCount)
    {
        return pixelCount == 0 || pixelIndex >= pixelCount;
    }

    static constexpr size_t mapPositionToPaletteIndex(size_t pixelIndex, size_t pixelCount, size_t maxIndex = 255)
    {
        if (isOutOfRange(pixelIndex, pixelCount))
        {
            return 0;
        }

        if (pixelCount <= 1)
        {
            return 0;
        }

        return (pixelIndex * maxIndex) / (pixelCount - 1);
    }
};

template <uint8_t TStart = 0, uint8_t TEnd = 255> struct WrapWindow
{
    static_assert(TStart <= TEnd, "WrapWindow requires TStart <= TEnd");

    static constexpr bool isOutOfRange(size_t, size_t) { return false; }

    static constexpr size_t distance(size_t left, size_t right, size_t = 255)
    {
        return WrapClamp::distance(left, right);
    }

    static constexpr size_t mapPositionToPaletteIndex(size_t pixelIndex, size_t pixelCount, size_t = 255)
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
        return static_cast<size_t>(TStart + mapped);
    }
};

template <uint8_t TStart = 0, uint8_t TEnd = 255> struct WrapModuloSpan
{
    static_assert(TStart <= TEnd, "WrapModuloSpan requires TStart <= TEnd");

    static constexpr bool isOutOfRange(size_t, size_t) { return false; }

    static constexpr size_t distance(size_t left, size_t right, size_t = 255)
    {
        return WrapClamp::distance(left, right);
    }

    static constexpr size_t mapPositionToPaletteIndex(size_t pixelIndex, size_t, size_t = 255)
    {
        const uint16_t span = static_cast<uint16_t>(TEnd - TStart + 1u);
        const uint16_t offset = static_cast<uint16_t>(pixelIndex % span);
        return static_cast<size_t>(TStart + offset);
    }
};

template <uint8_t TOffset = 0> struct WrapOffsetCircular
{
    static constexpr bool isOutOfRange(size_t, size_t) { return false; }

    static constexpr size_t distance(size_t left, size_t right, size_t maxIndex = 255)
    {
        return WrapCircular::distance(left, right, maxIndex);
    }

    static constexpr size_t mapPositionToPaletteIndex(size_t pixelIndex, size_t pixelCount, size_t maxIndex = 255)
    {
        const size_t base = WrapCircular::mapPositionToPaletteIndex(pixelIndex, pixelCount, maxIndex);
        return (base + TOffset) % (maxIndex + 1);
    }
};

} // namespace lw::colors::palettes
