#include <Arduino.h>
#include <LumaWave.h>

/*
Target: Arduino platforms with one-wire output support.
Requires: Two valid data pins for independent child strips.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Composite strip composition owns child strips; use for ownership-composed multi-strip layouts.
*/

namespace
{
using Protocol = lw::protocols::Ws2812x<>;
using StripType = lw::busses::PixelBus<Protocol>;
using CompositeType = lw::busses::CompositeBus<StripType, StripType>;

constexpr lw::PixelCount LeftCount = 16;
constexpr lw::PixelCount RightCount = 16;
constexpr int LeftDataPin = 2;
constexpr int RightDataPin = 3;

StripType::TransportSettingsType makeTransportSettings(int dataPin)
{
    StripType::TransportSettingsType settings{};
    settings.dataPin = dataPin;
    settings.invert = false;
    return settings;
}

StripType leftStrip(LeftCount, makeTransportSettings(LeftDataPin));
StripType rightStrip(RightCount, makeTransportSettings(RightDataPin));
CompositeType composite(std::move(leftStrip), std::move(rightStrip));
uint16_t frame = 0;
} // namespace

void setup()
{
    composite.begin();
}

void loop()
{
    auto& pixels = composite.pixels();
    const size_t count = pixels.size();

    for (size_t i = 0; i < count; ++i)
    {
        auto color = pixels[i];
        color['R'] = static_cast<uint8_t>((i + frame) & 0xFF);
        color['G'] = static_cast<uint8_t>((i * 3U) & 0x3F);
        color['B'] = 0;
        pixels[i] = color;
    }

    composite.show();
    ++frame;
    delay(20);
}
