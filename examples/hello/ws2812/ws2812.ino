#include <Arduino.h>
#include <LumaWave.h>

constexpr uint16_t ledCount = 30;
Strip<Protocols::Ws2812<>> strip(ledCount, Transport::DefaultSettings{{.dataPin = 2}});
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

        pixels[i] = Color(0, 8, 24);
    }

    const size_t head = (count > 0) ? static_cast<size_t>(frame % count) : 0;
    if (count > 0)
    {
        pixels[head] = Color(64, 16, 0);
    }

    strip.show();
    ++frame;
    delay(16);
}
