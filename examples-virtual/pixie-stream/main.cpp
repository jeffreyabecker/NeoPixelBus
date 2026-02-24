#include <Arduino.h>
#include <memory>
#include <VirtualNeoPixelBus.h>

static constexpr uint16_t PixelCount = 4;

static std::unique_ptr<npb::PixelBusT<npb::Rgb8Color>> bus;

void setup()
{
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    Serial.println("Pixie stream protocol smoke test");

    auto protocol = std::make_unique<npb::PixieProtocol>(
        PixelCount,
        npb::PixieProtocolSettings{
            std::make_unique<npb::PrintTransport>(Serial),
            npb::ChannelOrder::RGB});

    bus = std::make_unique<npb::PixelBusT<npb::Rgb8Color>>(PixelCount, std::move(protocol));
    bus->begin();
}

void loop()
{
    static uint8_t phase = 0;

    for (uint16_t i = 0; i < PixelCount; ++i)
    {
        uint8_t v = static_cast<uint8_t>((i * 255) / (PixelCount - 1));

        if (phase == 0)
        {
            bus->setPixelColor(i, npb::Rgb8Color(v, 0, 255 - v));
        }
        else
        {
            bus->setPixelColor(i, npb::Rgb8Color(255 - v, v, 0));
        }
    }

    bus->show();

    phase ^= 1;
    delay(500);
}
