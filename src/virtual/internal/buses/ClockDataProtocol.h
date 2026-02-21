#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <array>

namespace npb
{

// Describes the framing protocol for a two-wire (clock+data) LED strip.
// Used by ClockDataEmitter to wrap pixel data with start/end frames.
struct ClockDataProtocol
{
    std::span<const uint8_t> startFrame;
    std::span<const uint8_t> endFrame;

    // Additional end-frame bits calculated from pixel count:
    // e.g., DotStar APA102 requires ceil(pixelCount / 16) bytes of 0x00
    // after the fixed endFrame.
    // Set to 0 if no per-pixel end bits are needed.
    size_t endFrameBitsPerPixel{0};
    uint8_t endFrameFillByte{0x00};

    // Optional latch delay after transmission (e.g., WS2801 needs 500 µs)
    uint32_t latchDelayUs{0};
};

// --- Pre-defined protocol constants ---

namespace protocol
{

// APA102 / DotStar:
//   Start: 4 × 0x00
//   End:   4 × 0x00 (reset) + ceil(N/16) bytes of 0x00 (1 bit per 2 pixels)
inline constexpr std::array<uint8_t, 4> DotStarStartFrame = {0x00, 0x00, 0x00, 0x00};
inline constexpr std::array<uint8_t, 4> DotStarEndFrame   = {0x00, 0x00, 0x00, 0x00};

inline const ClockDataProtocol DotStar
{
    .startFrame = DotStarStartFrame,
    .endFrame   = DotStarEndFrame,
    // 1 bit per 2 pixels → 1 bit per pixel / 2, but stored as bits-per-pixel
    // ClockDataEmitter calculates: ceil(pixelCount * endFrameBitsPerPixel / 8)
    // For DotStar: 1 bit per 2 pixels = 0.5 bits per pixel
    // We represent this as: endFrameBitsPerPixel=1, and the emitter divides
    // the pixel count by 2 before multiplying.  Actually, let's keep it simple:
    // endFrameBytesForPixels(n) = (n + 15) / 16
    // We store the divisor directly.
    .endFrameBitsPerPixel = 1,  // 1 bit per 2 pixels
    .endFrameFillByte = 0x00,
    .latchDelayUs = 0,
};

// HD108:
//   Start: 16 × 0x00
//   End:   4 × 0xFF
inline constexpr std::array<uint8_t, 16> Hd108StartFrame = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};
inline constexpr std::array<uint8_t, 4> Hd108EndFrame = {0xFF, 0xFF, 0xFF, 0xFF};

inline const ClockDataProtocol Hd108
{
    .startFrame = Hd108StartFrame,
    .endFrame   = Hd108EndFrame,
    .endFrameBitsPerPixel = 0,
    .endFrameFillByte = 0x00,
    .latchDelayUs = 0,
};

// WS2801:
//   No framing, 500 µs latch
inline const ClockDataProtocol Ws2801
{
    .startFrame = {},
    .endFrame   = {},
    .endFrameBitsPerPixel = 0,
    .endFrameFillByte = 0x00,
    .latchDelayUs = 500,
};

// LPD8806:
//   Start: ceil(N/32) × 0x00
//   End:   ceil(N/32) × 0x00
//   (variable-length framing — handled separately in the emitter)
// Represented here with a fixed empty start/end; the emitter uses
// endFrameBitsPerPixel for the per-pixel overhead.
inline const ClockDataProtocol Lpd8806
{
    .startFrame = {},
    .endFrame   = {},
    .endFrameBitsPerPixel = 1,  // 1 bit per pixel → ceil(N/8) bytes
    .endFrameFillByte = 0x00,
    .latchDelayUs = 0,
};

// LPD6803:
//   Start: 4 × 0x00
//   End:   ceil(N/8) × 0x00 (1 bit per pixel)
inline constexpr std::array<uint8_t, 4> Lpd6803StartFrame = {0x00, 0x00, 0x00, 0x00};

inline const ClockDataProtocol Lpd6803
{
    .startFrame = Lpd6803StartFrame,
    .endFrame   = {},
    .endFrameBitsPerPixel = 1,
    .endFrameFillByte = 0x00,
    .latchDelayUs = 0,
};

// P9813:
//   Start: 4 × 0x00
//   End:   4 × 0x00
inline constexpr std::array<uint8_t, 4> P9813StartFrame = {0x00, 0x00, 0x00, 0x00};
inline constexpr std::array<uint8_t, 4> P9813EndFrame   = {0x00, 0x00, 0x00, 0x00};

inline const ClockDataProtocol P9813
{
    .startFrame = P9813StartFrame,
    .endFrame   = P9813EndFrame,
    .endFrameBitsPerPixel = 0,
    .endFrameFillByte = 0x00,
    .latchDelayUs = 0,
};

} // namespace protocol
} // namespace npb
