#pragma once

#include <cstdint>

#if defined(ARDUINO_ARCH_ESP32)
#include <esp_system.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#include <osapi.h>
#elif defined(ARDUINO_ARCH_RP2040)
#if defined(__has_include)
#if __has_include(<pico/rand.h>)
#define LW_HAS_PICO_RAND 1
#include <pico/rand.h>
#elif __has_include(<Arduino.h>)
#define LW_HAS_ARDUINO_RANDOM 1
#include <Arduino.h>
#endif
#endif
#endif

namespace lw::colors::palettes::detail::palettegen
{
struct XorShift32RandomBackend
{
    static uint32_t next(uint32_t& state)
    {
        if (state == 0u)
        {
            state = 0x6D2B79F5u;
        }

        state ^= (state << 13u);
        state ^= (state >> 17u);
        state ^= (state << 5u);
        return state;
    }
};

#if defined(ARDUINO_ARCH_ESP32)
struct Esp32RandomBackend
{
    static uint32_t next(uint32_t& state)
    {
        state = esp_random();
        return state;
    }
};
#endif

#if defined(ARDUINO_ARCH_ESP8266)
struct Esp8266RandomBackend
{
    static uint32_t next(uint32_t& state)
    {
        state = static_cast<uint32_t>(os_random());
        return state;
    }
};
#endif

#if defined(ARDUINO_ARCH_RP2040) && defined(LW_HAS_PICO_RAND)
struct Rp2040RandomBackend
{
    static uint32_t next(uint32_t& state)
    {
        state = get_rand_32();
        return state;
    }
};
#elif defined(ARDUINO_ARCH_RP2040) && defined(LW_HAS_ARDUINO_RANDOM)
struct Rp2040RandomBackend
{
    static uint32_t next(uint32_t& state)
    {
        state = static_cast<uint32_t>(random(static_cast<long>(0x7FFFFFFFL)));
        return state;
    }
};
#elif defined(ARDUINO_ARCH_RP2040)
struct Rp2040RandomBackend : XorShift32RandomBackend
{
};
#endif
} // namespace lw::colors::palettes::detail::palettegen
