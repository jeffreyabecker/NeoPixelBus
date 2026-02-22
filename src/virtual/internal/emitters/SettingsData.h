#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>

namespace npb
{

/// Small inline byte buffer for fixed in-band settings.
/// Encoded once at construction, never changed afterward.
struct SettingsData
{
    static constexpr size_t MaxSize = 8;

    std::array<uint8_t, MaxSize> bytes{};
    uint8_t size{0};

    constexpr std::span<const uint8_t> span() const
    {
        return {bytes.data(), size};
    }

    constexpr bool empty() const
    {
        return size == 0;
    }
};

} // namespace npb
