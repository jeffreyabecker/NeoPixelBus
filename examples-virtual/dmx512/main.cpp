#include <Arduino.h>
#include <memory>
#include <VirtualNeoPixelBus.h>

static constexpr uint16_t PixelCount = 4;

static std::unique_ptr<npb::PixelBus> bus;

void setup()
{
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    npb::Esp8266I2SSelfClockingTransportConfig transportConfig{};
    transportConfig.timing = npb::timing::Generic400;

    auto protocol = std::make_unique<npb::Dmx512Protocol>(
        PixelCount,
        npb::Dmx512ProtocolSettings{
            std::make_unique<npb::Esp8266I2SSelfClockingTransport>(transportConfig)});

    bus = std::make_unique<npb::PixelBus>(PixelCount, std::move(protocol));
    bus->begin();

    Serial.println("DMX512 protocol smoke test");
}

void loop()
{
    static uint8_t phase = 0;

    for (uint16_t i = 0; i < PixelCount; ++i)
    {
        const uint8_t v = static_cast<uint8_t>((i * 255) / (PixelCount - 1));
        if (phase == 0)
        {
            bus->setPixelColor(i, npb::Color(v, 0, 255 - v));
        }
        else
        {
            bus->setPixelColor(i, npb::Color(255 - v, v, 0));
        }
    }

    bus->show();
    phase ^= 1;
    delay(100);
}
