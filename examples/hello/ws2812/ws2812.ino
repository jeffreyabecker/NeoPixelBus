#include <Arduino.h>
#include <LumaWave.h>

constexpr uint16_t ledCount = 30;
Strip<Protocols::Ws2812> strip(ledCount, Transport::DefaultSettings{{.dataPin = 2}});
uint16_t frame = 0;

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
        pixels[i] = Color(static_cast<uint8_t>((i + frame) & 0x3F), static_cast<uint8_t>((2U * i + frame) & 0x3F),
                          static_cast<uint8_t>((3U * i + frame) & 0x3F));
    }

    strip.show();
    ++frame;
    delay(20);
}
