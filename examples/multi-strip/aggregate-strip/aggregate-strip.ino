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

Strip<Protocols::Ws2812> leftStrip(leftCount, Transport::DefaultSettings{{.dataPin = leftDataPin}});
Strip<Protocols::Ws2812> rightStrip(rightCount, Transport::DefaultSettings{{.dataPin = rightDataPin}});

IStrip<Color>* childBuses[] = {&leftStrip, &rightStrip};
AggregateStrip<Color> aggregate(lw::span<IStrip<Color>*>{childBuses, sizeof(childBuses) / sizeof(childBuses[0])});

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
        pixels[i] = Color(static_cast<uint8_t>((i + frame) & 0x3F), static_cast<uint8_t>((2U * i + frame) & 0x3F),
                          static_cast<uint8_t>((3U * i + frame) & 0x3F));
    }

    aggregate.show();
    ++frame;
    delay(20);
}
