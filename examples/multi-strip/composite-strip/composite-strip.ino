#include <Arduino.h>
#include <LumaWave.h>

/*
Target: Arduino platforms with one-wire output support.
Requires: Two valid data pins for independent child strips.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Composite strip composition owns child strips; use for ownership-composed multi-strip layouts.
*/

constexpr uint16_t leftCount = 16;
constexpr uint16_t rightCount = 16;
constexpr int leftDataPin = 2;
constexpr int rightDataPin = 3;

auto composite = CompositeStrip<Strip<Protocols::Ws2812>, Strip<Protocols::Ws2812>>(
    Strip<Protocols::Ws2812>(leftCount, Transport::DefaultSettings{{.dataPin = leftDataPin}}),
    Strip<Protocols::Ws2812>(rightCount, Transport::DefaultSettings{{.dataPin = rightDataPin}}));
uint16_t frame = 0;

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
        pixels[i] = Color(static_cast<uint8_t>((i + frame) & 0x3F), static_cast<uint8_t>((2U * i + frame) & 0x3F),
                          static_cast<uint8_t>((3U * i + frame) & 0x3F));
    }

    composite.show();
    ++frame;
    delay(20);
}
