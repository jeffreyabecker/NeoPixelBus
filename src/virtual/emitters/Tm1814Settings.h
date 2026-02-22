#pragma once

#include <cstdint>
#include <algorithm>
#include <array>

#include "../colors/Color.h"
#include "SettingsData.h"

namespace npb
{

/// TM1814 per-channel current-limit settings.
/// Encodes as 8 prepended bytes: 4 gain bytes (C1) + 4 complement bytes (C2).
/// Current values are in tenths of milliamps, clamped to [65, 380].
struct Tm1814Settings
{
    static constexpr uint16_t MinCurrent = 65;
    static constexpr uint16_t MaxCurrent = 380;
    static constexpr uint16_t EncodeDivisor = 5;

    uint16_t redCurrent{MinCurrent};
    uint16_t greenCurrent{MinCurrent};
    uint16_t blueCurrent{MinCurrent};
    uint16_t whiteCurrent{MinCurrent};

    /// Encode header bytes in the given channel output order.
    /// channelOrder maps output position → Color channel index
    /// (0=R, 1=G, 2=B, 3=WW/W).
    constexpr SettingsData encode(
        const std::array<uint8_t, Color::ChannelCount>& channelOrder) const
    {
        // Current values indexed by Color channel
        const uint16_t currentByChannel[Color::ChannelCount] = {
            redCurrent, greenCurrent, blueCurrent, whiteCurrent, 0
        };

        SettingsData result{};
        result.size = 8;

        // C1: encoded gain per output channel
        for (uint8_t i = 0; i < 4; ++i)
        {
            uint16_t clamped = std::clamp(
                currentByChannel[channelOrder[i]],
                MinCurrent, MaxCurrent);
            result.bytes[i] = static_cast<uint8_t>(
                (clamped - MinCurrent) / EncodeDivisor);
        }

        // C2: ones' complement of C1
        for (uint8_t i = 0; i < 4; ++i)
        {
            result.bytes[i + 4] = static_cast<uint8_t>(~result.bytes[i]);
        }

        return result;
    }
};

} // namespace npb
