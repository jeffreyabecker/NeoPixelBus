#pragma once

#include <cstdint>

namespace npb
{

    /// NRZ bit-encoding durations and reset interval for one-wire LED protocols.
    /// Signal inversion is NOT part of timing ? it is a separate hardware-output
    /// concern handled by each platform protocol.
    struct OneWireTiming
    {
        uint32_t t0hNs;   // T0H ? high time for a zero bit (nanoseconds)
        uint32_t t0lNs;   // T0L ? low  time for a zero bit (nanoseconds)
        uint32_t t1hNs;   // T1H ? high time for a one  bit (nanoseconds)
        uint32_t t1lNs;   // T1L ? low  time for a one  bit (nanoseconds)
        uint32_t resetUs; // reset / latch interval (microseconds)

        /// Bit period in nanoseconds, derived from the zero-bit timings.
        constexpr uint32_t bitPeriodNs() const
        {
            return t0hNs + t0lNs;
        }

        /// Bit rate in Hz, derived from the zero-bit timings.
        constexpr float bitRateHz() const
        {
            return 1.0e9f / static_cast<float>(bitPeriodNs());
        }
    };

    namespace timing
    {
        inline constexpr OneWireTiming Ws2812x = {400, 850, 800, 450, 300};
        inline constexpr OneWireTiming Ws2811 = {500, 2000, 1200, 1300, 50};
        inline constexpr OneWireTiming Ws2805 = {300, 790, 790, 300, 300};
        inline constexpr OneWireTiming Sk6812 = {400, 850, 800, 450, 80};
        inline constexpr OneWireTiming Tm1814 = {360, 720, 720, 360, 200};
        inline constexpr OneWireTiming Tm1914 = {360, 720, 720, 360, 200};
        inline constexpr OneWireTiming Tm1829 = {300, 800, 800, 300, 500};
        inline constexpr OneWireTiming Apa106 = {350, 1360, 1360, 350, 50};
        inline constexpr OneWireTiming Tx1812 = {300, 600, 600, 300, 80};
        inline constexpr OneWireTiming Gs1903 = {300, 900, 900, 300, 40};
        inline constexpr OneWireTiming Generic800 = {400, 850, 800, 450, 50};
        inline constexpr OneWireTiming Generic400 = {500, 2000, 1200, 1300, 50};

        // Aliases ? identical timing, different chip branding
        inline constexpr auto Ws2816 = Ws2812x;
        inline constexpr auto Ws2813 = Ws2812x;
        inline constexpr auto Ws2814 = Ws2805;
        inline constexpr auto Lc8812 = Sk6812;
    } // namespace timing

} // namespace npb

