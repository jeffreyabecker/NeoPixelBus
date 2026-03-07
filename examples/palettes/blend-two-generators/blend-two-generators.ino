#include <Arduino.h>
#include <LumaWave.h>

constexpr uint16_t ledCount = 30;
Strip<Protocols::Ws2812> strip(ledCount, Transport::DefaultSettings{{.dataPin = 2}});
uint16_t frame = 0;

Generator::RandomSmooth<Color, 16> fromGenerator(0x1234ABCDu, 9);
Generator::RandomSmooth<Color, 16> toGenerator(0x98765432u, 11);

uint8_t triangleWave(uint32_t t, uint32_t periodMs)
{
    const uint32_t wrapped = (periodMs > 0U) ? (t % periodMs) : 0U;
    const uint32_t half = periodMs / 2U;
    if (half == 0U)
    {
        return 0U;
    }

    const uint32_t rising = (wrapped < half) ? wrapped : (periodMs - wrapped);
    return static_cast<uint8_t>((rising * 255U) / half);
}

void setup()
{
    strip.begin();
}

void loop()
{
    while (true)
    {

        fromGenerator.update();
        toGenerator.update();

        const uint8_t blend8 = triangleWave(millis(), 5000U);
        samplePalette(fromGenerator, toGenerator, static_cast<size_t>(frame), strip.pixels(), blend8);

        strip.show();
        frame = static_cast<uint16_t>(frame + 3U);
        delay(20);
    }
}
