#pragma once

namespace lw
{
namespace factory
{
namespace descriptors
{

    struct NeoPrint
    {
        static constexpr const char *PrimaryToken = "neoprint";
        static constexpr const char *const Tokens[4] = {
            "neoprint",
            "print",
            "serial",
            "debug"};
    };

    struct Nil
    {
        static constexpr const char *PrimaryToken = "nil";
        static constexpr const char *const Tokens[1] = {
            "nil"};
    };

    struct NeoSpi
    {
    };

    struct RpPio
    {
    };

    struct RpSpi
    {
    };

    struct RpUart
    {
    };

    struct Esp32Rmt
    {
        static constexpr const char *PrimaryToken = "esp32-rmt";
        static constexpr const char *const Tokens[1] = {
            "esp32-rmt"};
    };

    struct Esp32I2s
    {
    };

    struct Esp32DmaSpi
    {
    };

    struct Esp8266DmaI2s
    {
    };

    struct Esp8266DmaUart
    {
    };

#if defined(ARDUINO_ARCH_ESP32)
    using PlatformDefault = Esp32I2s;
#elif defined(ARDUINO_ARCH_ESP8266)
    using PlatformDefault = Esp8266DmaI2s;
#elif defined(ARDUINO_ARCH_RP2040)
    using PlatformDefault = RpPio;
#elif defined(ARDUINO_ARCH_NATIVE) || !defined(ARDUINO) || !defined(LW_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS)
    using PlatformDefault = Nil;
#else
    using PlatformDefault = NeoSpi;
#endif

} // namespace descriptors
} // namespace factory
} // namespace lw
