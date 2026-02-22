#pragma once

#include <cstdint>

#include "SettingsData.h"

namespace npb
{

/// TM1914 operating-mode selection.
/// Encodes as 6 prepended bytes: 3 mode bytes (C1) + 3 complement bytes (C2).
/// Not channel-order dependent.
struct Tm1914Settings
{
    enum class Mode : uint8_t
    {
        DinFdinAutoSwitch = 0xFF,
        DinOnly           = 0xF5,
        FdinOnly          = 0xFA
    };

    Mode mode{Mode::DinOnly};

    constexpr SettingsData encode() const
    {
        SettingsData result{};
        result.size = 6;

        // C1
        result.bytes[0] = 0xFF;
        result.bytes[1] = 0xFF;
        result.bytes[2] = static_cast<uint8_t>(mode);

        // C2: ones' complement of C1
        result.bytes[3] = static_cast<uint8_t>(~result.bytes[0]);
        result.bytes[4] = static_cast<uint8_t>(~result.bytes[1]);
        result.bytes[5] = static_cast<uint8_t>(~result.bytes[2]);

        return result;
    }
};

} // namespace npb
