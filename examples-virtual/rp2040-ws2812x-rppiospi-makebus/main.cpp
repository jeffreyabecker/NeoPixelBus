#include <Arduino.h>
#include <NeoPixelBus.h>

#ifdef ARDUINO_ARCH_RP2040

namespace
{

    static constexpr uint16_t PixelCount = 60;
    static constexpr uint8_t DataPin = 16;
    static constexpr uint8_t ClockPin = 17;

    using TransportConfig = OneWire<RpPioSpiTransport>;
    using BusType = Bus<Ws2812, TransportConfig>;

    static TransportConfig makeTransportConfig()
    {
        TransportConfig config{};
        config.settings.pin = DataPin;
        config.settings.clockPin = static_cast<int8_t>(ClockPin);
        config.settings.pioIndex = 1;
        config.settings.frameBytes = PixelCount * 3;
        config.settings.invert = false;
        config.settings.clockDataBitRateHz = static_cast<uint32_t>(OneWireTiming::Ws2812x.bitRateHz()) * 4U;
        config.settings.manageTransaction = true;
        config.settings.bitPattern = OneWireTiming::Ws2812x.bitPattern();
        config.settings.timing = OneWireTiming::Ws2812x;
        return config;
    }

    static TransportConfig transportConfig = makeTransportConfig();

    static BusType leds = makeBus(
        PixelCount,
        Ws2812{.colorOrder = "GRB"},
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
