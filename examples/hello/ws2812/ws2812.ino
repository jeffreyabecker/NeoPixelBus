#include <Arduino.h>
#include <LumaWave.h>

/*
Target: Arduino platforms with one-wire output support (RP2040 default path).
Requires: Default transport with valid data pin.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Uses `lw::busses::PixelBus<Protocol>` default protocol settings constructor.
*/

namespace
{
using Protocol = lw::protocols::Ws2812x<>;
using StripType = lw::busses::PixelBus<Protocol>;

constexpr lw::PixelCount LedCount = 30;
constexpr int DataPin = 2;

StripType::TransportSettingsType makeTransportSettings()
{
    StripType::TransportSettingsType settings{};
    settings.dataPin = DataPin;
    settings.invert = false;
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
    const size_t count = pixels.size();

    for (size_t i = 0; i < count; ++i)
    {
        auto color = pixels[i];
        color['R'] = 0;
        color['G'] = 8;
        color['B'] = 24;
        pixels[i] = color;
    }

    const size_t head = (count > 0) ? static_cast<size_t>(frame % count) : 0;
    if (count > 0)
    {
        auto lead = pixels[head];
        lead['R'] = 64;
        lead['G'] = 16;
        lead['B'] = 0;
        pixels[head] = lead;
    }

    strip.show();
    ++frame;
    delay(16);
}
