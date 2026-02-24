#include <Arduino.h>
#include <memory>
#include <VirtualNeoPixelBus.h>

#if defined(ARDUINO_ARCH_ESP32) && (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 1))
#include "driver/spi_master.h"

static constexpr uint16_t PixelCount = 8;

// Adjust pins for your board wiring.
static constexpr int8_t ClockPin = 18;
static constexpr int8_t DataPin = 23;

static std::unique_ptr<npb::PixelBusT<npb::Rgb8Color>> bus;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }

    Serial.println("ESP32 DMA SPI ClockData transport smoke test");

    npb::Esp32DmaSpiClockDataTransportConfig transportConfig{};
    transportConfig.spiHost = SPI2_HOST;
    transportConfig.clockPin = ClockPin;
    transportConfig.dataPin = DataPin;
    transportConfig.ssPin = -1;
    transportConfig.clockDataBitRateHz = 10000000UL;

    npb::DotStarProtocolSettings settings{
        std::make_unique<npb::Esp32DmaSpiClockDataTransport>(transportConfig)};

    auto protocol = std::make_unique<npb::DotStarProtocol>(PixelCount, settings);
    bus = std::make_unique<npb::PixelBusT<npb::Rgb8Color>>(PixelCount, std::move(protocol));
    bus->begin();
}

void loop()
{
    static uint8_t phase = 0;

    for (uint16_t i = 0; i < PixelCount; ++i)
    {
        const uint8_t value = static_cast<uint8_t>(phase + (i * 32));
        bus->setPixelColor(i, npb::Rgb8Color(value, static_cast<uint8_t>(255 - value), 32));
    }

    bus->show();
    phase += 8;
    delay(50);
}

#else

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }
    Serial.println("This example requires ESP32 with ESP-IDF >= 4.4.1");
}

void loop()
{
    delay(1000);
}

#endif
