#include <Arduino.h>
#include <LumaWave.h>

/*
Target: ESP8266 only.
Requires: `ARDUINO_ARCH_ESP8266`.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Demonstrates `lw::transports::esp8266::Esp8266DmaUartTransport`.
*/

#if !defined(ARDUINO_ARCH_ESP8266)
#error "This example requires ARDUINO_ARCH_ESP8266."
#endif

namespace
{
using Protocol = lw::protocols::Ws2812x<>;
using Transport = lw::transports::esp8266::Esp8266DmaUartTransport;
using StripType = lw::busses::PixelBus<Protocol, Transport>;

constexpr lw::PixelCount LedCount = 24;

StripType::TransportSettingsType makeTransportSettings()
{
    StripType::TransportSettingsType settings{};
    settings.uartNumber = 1;
    settings.baudRate = 3200000UL;
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
        color['R'] = static_cast<uint8_t>((frame + i * 3U) & 0x7F);
        color['G'] = 0;
        color['B'] = static_cast<uint8_t>((frame + i * 6U) & 0x7F);
        pixels[i] = color;
    }

    strip.show();
    ++frame;
    delay(20);
}
