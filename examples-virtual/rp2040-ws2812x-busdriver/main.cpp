#include <Arduino.h>
#include <NeoPixelBus.h>

#ifdef ARDUINO_ARCH_RP2040

static constexpr uint16_t PixelCount = 8;
static constexpr uint8_t DataPin = 16;

static RpPioOneWire transportConfig{
    .settings = {
        .pin = DataPin,
        .pioIndex = 1,
        .frameBytes = PixelCount * 3,
        .invert = false,
        .timing = OneWireTiming::Ws2812x}};

static auto leds = makeBus(
    PixelCount,
    Ws2812{.colorOrder = "GRB"},
    transportConfig);

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }

    Serial.println("RP2040 WS2812x BusDriver pixel bus example");
    leds.begin();
}

void loop()
{
    static uint8_t value = 0;

    for (size_t i = 0; i < PixelCount; ++i)
    {
        leds.setPixelColor(i, Rgb8Color{0, 0, 0});
    }

    const size_t index = (value / 16) % PixelCount;
    leds.setPixelColor(index, Rgb8Color{value, static_cast<uint8_t>(255 - value), 32});
    leds.show();

    value += 4;
    delay(40);
}

#else

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }

    Serial.println("This example requires ARDUINO_ARCH_RP2040");
}

void loop()
{
    delay(1000);
}

#endif
