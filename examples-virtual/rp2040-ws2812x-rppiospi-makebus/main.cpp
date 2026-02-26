#include <Arduino.h>
#include <NeoPixelBus.h>

#ifdef ARDUINO_ARCH_RP2040

namespace
{

    static constexpr uint16_t PixelCount = 60;
    static constexpr uint8_t DataPin = 16;
    static constexpr uint8_t ClockPin = 17;

    using TransportConfig = RpPioSpi;
    using BusType = Bus<Ws2812, OneWire<RpPioSpiTransport>>;

    static TransportConfig transportConfig = {
        .settings = {
            .pin = DataPin,
            .clockPin = static_cast<int8_t>(ClockPin),
            .pioIndex = 1,
            .frameBytes = PixelCount * 3,
            .invert = false,
        }};

    static BusType leds = makeBus(
        PixelCount,
        Ws2812{.colorOrder = "GRB"},
        OneWireTiming::Ws2812x,
        transportConfig);

} // namespace

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }

    Serial.println("RP2040 makeBus WS2812x over RpPioSpiTransport (OneWireWrapper) - 60 pixels");
    leds.begin();
}

void loop()
{
    static uint8_t value = 0;

    for (size_t i = 0; i < PixelCount; ++i)
    {
        leds.setPixelColor(i, Rgb8Color{0, 0, 0});
    }

    const size_t index = (value / 8) % PixelCount;
    leds.setPixelColor(index, Rgb8Color{value, static_cast<uint8_t>(255 - value), 24});
    leds.show();

    value += 3;
    delay(30);
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
