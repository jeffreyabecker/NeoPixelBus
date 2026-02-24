#include <Arduino.h>
#include <memory>
#include <VirtualNeoPixelBus.h>

#if defined(ARDUINO_ARCH_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32S3) && !defined(CONFIG_IDF_TARGET_ESP32C3)

static constexpr uint16_t PixelCountPerLane = 16;
static constexpr uint8_t Lane0 = 0;
static constexpr uint8_t Lane1 = 1;

// Adjust these two pins to match your wiring.
static constexpr int8_t Lane0Pin = 18;
static constexpr int8_t Lane1Pin = 19;

static std::unique_ptr<npb::Esp32I2sParallelClockDataTransport> parallelTransport;
static std::unique_ptr<npb::PixelBusT<npb::Rgb8Color>> busLane0;
static std::unique_ptr<npb::PixelBusT<npb::Rgb8Color>> busLane1;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }

    npb::Esp32I2sParallelClockDataTransportConfig transportConfig{};
    transportConfig.busNumber = 1;
    transportConfig.bitSendTimeNs = static_cast<uint16_t>(npb::timing::Ws2812x.bitPeriodNs());
    transportConfig.laneMask = static_cast<uint8_t>((1u << Lane0) | (1u << Lane1));
    transportConfig.lanes[Lane0].pin = Lane0Pin;
    transportConfig.lanes[Lane0].invert = false;
    transportConfig.lanes[Lane1].pin = Lane1Pin;
    transportConfig.lanes[Lane1].invert = false;

    parallelTransport = std::make_unique<npb::Esp32I2sParallelClockDataTransport>(transportConfig);
    parallelTransport->begin();

    auto lane0Transport = parallelTransport->getLane(Lane0);
    auto lane1Transport = parallelTransport->getLane(Lane1);

    npb::EncodedClockDataSelfClockingTransportConfig encodedConfig{};
    encodedConfig.timing = npb::timing::Ws2812x;
    encodedConfig.clockDataBitRateHz = 2500000UL;
    encodedConfig.manageTransaction = true;
    encodedConfig.bitPattern = npb::EncodedClockDataBitPattern::ThreeStep;

    auto protocol0 = std::make_unique<npb::Ws2812xProtocol<npb::Rgb8Color>>(
        PixelCountPerLane,
        npb::ChannelOrder::GRB,
        std::make_unique<npb::EncodedClockDataSelfClockingTransport>(lane0Transport.get(), encodedConfig));

    auto protocol1 = std::make_unique<npb::Ws2812xProtocol<npb::Rgb8Color>>(
        PixelCountPerLane,
        npb::ChannelOrder::GRB,
        std::make_unique<npb::EncodedClockDataSelfClockingTransport>(lane1Transport.get(), encodedConfig));

    busLane0 = std::make_unique<npb::PixelBusT<npb::Rgb8Color>>(PixelCountPerLane, std::move(protocol0));
    busLane1 = std::make_unique<npb::PixelBusT<npb::Rgb8Color>>(PixelCountPerLane, std::move(protocol1));

    busLane0->begin();
    busLane1->begin();

    Serial.println("ESP32 I2S parallel strict-sync clock-data demo ready");
}

void loop()
{
    static uint8_t phase = 0;

    for (uint16_t i = 0; i < PixelCountPerLane; ++i)
    {
        const uint8_t lane0Value = static_cast<uint8_t>(phase + (i * 8));
        const uint8_t lane1Value = static_cast<uint8_t>((255 - phase) + (i * 8));

        busLane0->setPixelColor(i, npb::Rgb8Color(lane0Value, 0, static_cast<uint8_t>(255 - lane0Value)));
        busLane1->setPixelColor(i, npb::Rgb8Color(0, lane1Value, static_cast<uint8_t>(255 - lane1Value)));
    }

    // Strict sync policy: both lane buses must submit each frame.
    busLane0->show();
    busLane1->show();

    phase += 3;
    delay(20);
}

#else

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }
    Serial.println("This example requires ESP32 target with I2S parallel transport support");
}

void loop()
{
    delay(1000);
}

#endif
