#include <Arduino.h>
#include <LumaWave.h>

/*
Target: Arduino platforms with one-wire output support.
Requires: Two valid data pins and non-owning child references.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Aggregate strip composition is non-owning; combine existing strip instances by pointer.
*/

namespace
{
using ColorType = lw::Rgb8Color;
using Protocol = lw::protocols::Ws2812x<ColorType>;
using StripType = lw::busses::PixelBus<Protocol>;

constexpr lw::PixelCount LeftCount = 12;
constexpr lw::PixelCount RightCount = 12;
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

lw::IPixelBus<ColorType>* childBuses[] = {&leftStrip, &rightStrip};
lw::busses::AggregateBus<ColorType> aggregate(lw::span<lw::IPixelBus<ColorType>*>{
    childBuses, sizeof(childBuses) / sizeof(childBuses[0])});

uint16_t frame = 0;
} // namespace

void setup()
{
    aggregate.begin();
}

void loop()
{
    auto& pixels = aggregate.pixels();
    const size_t count = pixels.size();

    for (size_t i = 0; i < count; ++i)
    {
        auto color = pixels[i];
        color['R'] = static_cast<uint8_t>((frame + i * 5U) & 0x7F);
        color['G'] = static_cast<uint8_t>((frame + i * 3U) & 0x7F);
        color['B'] = static_cast<uint8_t>((frame + i) & 0x7F);
        pixels[i] = color;
    }

    aggregate.show();
    ++frame;
    delay(20);
}
