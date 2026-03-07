#include <Arduino.h>
#include <LumaWave.h>

#include <array>

constexpr pixel_count_t ledCount = 30;
Strip<Protocols::Ws2812> strip(ledCount, Transport::DefaultSettings{{.dataPin = 2}});
uint16_t frame = 0;

lw::colors::palettes::SolidPaletteGenerator<Color, 8> generator(Color(255, 0, 0));

constexpr std::array<Color, 4> kCycleColors = {
    Color(255, 0, 0),
    Color(0, 255, 0),
    Color(0, 0, 255),
    Color(255, 255, 255),
};

void setup()
{
    strip.begin();
}

void loop()
{
    while (true)
    {
        const size_t paletteColorIndex = static_cast<size_t>((frame / 100U) % kCycleColors.size());
        generator.setColor(kCycleColors[paletteColorIndex]);
        generator.update();

        samplePalette(generator, static_cast<size_t>(0), strip.pixels());

        strip.show();
        ++frame;
        delay(20);
    }
}
