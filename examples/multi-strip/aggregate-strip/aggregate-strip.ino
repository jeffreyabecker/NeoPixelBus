#include <Arduino.h>
#include <LumaWave.h>

/*
Target: Arduino platforms with one-wire output support.
Requires: Two valid data pins and non-owning child references.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Aggregate strip composition is non-owning; combine existing strip instances by pointer.
*/

constexpr uint16_t leftCount = 12;
constexpr uint16_t rightCount = 12;
constexpr int leftDataPin = 2;
constexpr int rightDataPin = 3;

Strip<Protocols::Ws2812<Rgb8Color>> leftStrip(leftCount, Transport::DefaultSettings{{.dataPin = leftDataPin}});
Strip<Protocols::Ws2812<Rgb8Color>> rightStrip(rightCount, Transport::DefaultSettings{{.dataPin = rightDataPin}});

IStrip<Rgb8Color>* childBuses[] = {&leftStrip, &rightStrip};
AggregateStrip<Rgb8Color> aggregate(lw::span<IStrip<Rgb8Color>*>{childBuses,
                                                                 sizeof(childBuses) / sizeof(childBuses[0])});

uint16_t frame = 0;

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
