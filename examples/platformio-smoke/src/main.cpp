#include <Arduino.h>
#include <NeoPixelBus.h>

static constexpr uint16_t PixelCount = 8;
static constexpr uint8_t PixelPin = 2;

NeoPixelBus<NeoRgbFeature, NeoWs2812xMethod> strip(PixelCount, PixelPin);

void setup()
{
    strip.Begin();
    strip.ClearTo(RgbColor(0));
    strip.Show();
}

void loop()
{
    static uint8_t value = 0;

    strip.SetPixelColor(0, RgbColor(value, 0, 0));
    strip.Show();

    value += 8;
    delay(50);
}
