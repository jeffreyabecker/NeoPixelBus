#include <Arduino.h>
#include <memory>
#include <VirtualNeoPixelBus.h>

static constexpr uint16_t PixelCount = 8;

static std::unique_ptr<npb::PixelBus> bus;

void setup()
{
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    auto emitter = std::make_unique<npb::PrintProtocol>(
        PixelCount,
        nullptr,
        npb::PrintProtocolSettings{
            Serial,
            npb::ColorOrderTransformConfig{
                .channelCount = 3,
                .channelOrder = {1, 0, 2, 0, 0}  // GRB
            }
        });
    bus = std::make_unique<npb::PixelBus>(PixelCount, std::move(emitter));
    bus->begin();
}

void loop()
{
    static uint8_t value = 0;

    bus->setPixelColor(0, npb::Color(value, 0, 0));
    bus->show();

    value += 8;
    delay(500);
}
