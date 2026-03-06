#include <Arduino.h>
#include <LumaWave.h>

/*
Target: ESP32 with IDF >= 4.4.1.
Requires: `ARDUINO_ARCH_ESP32` and SPI-capable pins.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Demonstrates `lw::transports::esp32::Esp32DmaSpiTransport` with DotStar protocol.
*/

#if !defined(ARDUINO_ARCH_ESP32)
#error "This example requires ARDUINO_ARCH_ESP32."
#endif

namespace
{
using Protocol = lw::protocols::DotStar<>;
using Transport = lw::transports::esp32::Esp32DmaSpiTransport;
using StripType = lw::busses::PixelBus<Protocol, Transport>;

constexpr lw::PixelCount LedCount = 60;
#if defined(SCK)
constexpr int ClockPin = SCK;
#else
constexpr int ClockPin = 18;
#endif

#if defined(MOSI)
constexpr int DataPin = MOSI;
#else
constexpr int DataPin = 23;
#endif

StripType::TransportSettingsType makeTransportSettings()
{
    StripType::TransportSettingsType settings{};
    settings.clockPin = ClockPin;
    settings.dataPin = DataPin;
    settings.clockRateHz = 10000000UL;
    return settings;
}

StripType strip(LedCount, makeTransportSettings());
uint16_t frame = 0;
} // namespace

void setup()
{
    strip.begin();
}

void loop()
{
    auto& pixels = strip.pixels();
    for (size_t i = 0; i < pixels.size(); ++i)
    {
        auto color = pixels[i];
        color['R'] = static_cast<uint8_t>((i + frame) & 0x3F);
        color['G'] = static_cast<uint8_t>((2U * i + frame) & 0x3F);
        color['B'] = static_cast<uint8_t>((3U * i + frame) & 0x3F);
        pixels[i] = color;
    }

    strip.show();
    ++frame;
    delay(20);
}
