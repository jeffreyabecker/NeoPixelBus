#include <Arduino.h>
#include <VirtualNeoPixelBus.h>

static constexpr uint16_t PixelCount = 8;

static npb::PrintEmitter emitter(Serial, nullptr,
    npb::ColorOrderTransformConfig{
        .channelCount = 3,
        .channelOrder = {1, 0, 2, 0, 0}  // GRB
    });

static npb::PixelBus bus(PixelCount, emitter);

void setup()
{
    Serial.begin(115200);
    bus.begin();
}

void loop()
{
    static uint8_t value = 0;

    bus.setPixelColor(0, npb::Color(value, 0, 0));
    bus.show();

    value += 8;
    delay(500);
}
