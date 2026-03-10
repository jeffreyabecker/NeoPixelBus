#include <Arduino.h>
#include <LumaWave.h>

constexpr pixel_count_t ledCount = 30;
Strip<Protocols::Ws2812> strip(ledCount, Transport::DefaultSettings{{.dataPin = 2}});
uint16_t frame = 0;

lw::colors::palettes::RandomCyclePaletteGenerator<Color> generator(8, 0xC001D00Du, 12);

void setup()
{
    strip.begin();
}

void loop()
{
    while (true)
    {
        generator.update();
        samplePalette(generator, static_cast<size_t>(frame), strip.pixels());

        strip.show();
        frame = static_cast<uint16_t>(frame + 4U);
        delay(20);
    }
}
