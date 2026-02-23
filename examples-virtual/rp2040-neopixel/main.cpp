/// Phase 6 integration test — Ws2812xProtocol + RpPioSelfClockingTransport on Pico 2 W.
///
/// Drives a WS2812x strip on GPIO 16, PIO1, using direct protocol + transport wiring.
/// Cycles a single red pixel through increasing brightness.

#include <Arduino.h>
#include <memory>
#include <VirtualNeoPixelBus.h>

static constexpr uint16_t PixelCount = 8;
static constexpr uint8_t  DataPin    = 16;

static std::unique_ptr<npb::PixelBus> bus;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }

    Serial.println("Phase 6 — Ws2812xProtocol + RpPioSelfClockingTransport test");

    npb::RpPioSelfClockingTransportConfig transportConfig{};
    transportConfig.pin = DataPin;
    transportConfig.pioIndex = 1;
    transportConfig.timing = npb::timing::Ws2812x;
    transportConfig.invert = false;
    transportConfig.frameBytes = PixelCount * 3;

    // Construct protocol: WS2812x timing, GRB channel order, PIO1, no shader
    auto protocol = std::make_unique<npb::Ws2812xProtocol>(
        PixelCount,
        nullptr,
        npb::ChannelOrder::GRB,
        std::make_unique<npb::RpPioSelfClockingTransport>(transportConfig));

    bus = std::make_unique<npb::PixelBus>(PixelCount, std::move(protocol));
    bus->begin();

    Serial.println("Bus initialised — starting animation");
}

void loop()
{
    static uint8_t hue = 0;

    // Clear all pixels
    for (size_t i = 0; i < PixelCount; ++i)
    {
        bus->setPixelColor(i, npb::Color{0, 0, 0});
    }

    // Light one pixel based on hue rotation
    size_t idx = (hue / 32) % PixelCount;
    bus->setPixelColor(idx, npb::Color{hue, static_cast<uint8_t>(255 - hue), 128});

    bus->show();

    hue += 4;
    delay(50);
}
