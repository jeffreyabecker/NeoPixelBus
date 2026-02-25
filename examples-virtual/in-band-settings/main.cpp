// In-band settings smoke test:
// - TLC59711 brightness header
// - TLC5947 latch-backed payload
// - TM1814 current settings preamble
// - TM1914 mode preamble
// - SM168x gain settings trailer (3/4/5-channel variants)

#include <Arduino.h>
#include <memory>
#include <VirtualNeoPixelBus.h>

static constexpr uint16_t PixelCount = 4;
static constexpr uint8_t DataPin = 16;

// Shared debug bus — prints all clock/data traffic to Serial.
static npb::DebugTransport debugBus(Serial);

// ---------- helpers ----------

static void fillGradientRgb8(npb::PixelBusT<npb::Rgb8Color>& bus)
{
    for (uint16_t i = 0; i < bus.pixelCount(); ++i)
    {
        uint8_t v = static_cast<uint8_t>((i * 255) / (bus.pixelCount() - 1));
        bus.setPixelColor(i, npb::Rgb8Color(v, 0, 255 - v));
    }
}

static void runProtocolRgb8(const char* name, std::unique_ptr<npb::IProtocol<npb::Rgb8Color>> protocol)
{
    Serial.println();
    Serial.print("=== ");
    Serial.print(name);
    Serial.println(" ===");

    auto bus = std::make_unique<npb::PixelBusT<npb::Rgb8Color>>(PixelCount, std::move(protocol));
    bus->begin();
    fillGradientRgb8(*bus);
    bus->show();
}

static void fillGradientRgb16(npb::PixelBusT<npb::Rgb16Color>& bus)
{
    for (uint16_t i = 0; i < bus.pixelCount(); ++i)
    {
        uint16_t v = static_cast<uint16_t>((static_cast<uint32_t>(i) * 65535u) / (bus.pixelCount() - 1));
        bus.setPixelColor(i, npb::Rgb16Color(v, 0, static_cast<uint16_t>(65535u - v)));
    }
}

static void runProtocolRgb16(const char* name, std::unique_ptr<npb::IProtocol<npb::Rgb16Color>> protocol)
{
    Serial.println();
    Serial.print("=== ");
    Serial.print(name);
    Serial.println(" ===");

    auto bus = std::make_unique<npb::PixelBusT<npb::Rgb16Color>>(PixelCount, std::move(protocol));
    bus->begin();
    fillGradientRgb16(*bus);
    bus->show();
}

static void fillGradientRgbw8(npb::PixelBusT<npb::Rgbw8Color>& bus)
{
    for (uint16_t i = 0; i < bus.pixelCount(); ++i)
    {
        uint8_t v = static_cast<uint8_t>((i * 255) / (bus.pixelCount() - 1));
        bus.setPixelColor(i, npb::Rgbw8Color(v, 255 - v, v / 2, 255 - (v / 2)));
    }
}

static void runProtocolRgbw8(const char* name, std::unique_ptr<npb::IProtocol<npb::Rgbw8Color>> protocol)
{
    Serial.println();
    Serial.print("=== ");
    Serial.print(name);
    Serial.println(" ===");

    auto bus = std::make_unique<npb::PixelBusT<npb::Rgbw8Color>>(PixelCount, std::move(protocol));
    bus->begin();
    fillGradientRgbw8(*bus);
    bus->show();
}

static void fillGradientRgbcw8(npb::PixelBusT<npb::Rgbcw8Color>& bus)
{
    for (uint16_t i = 0; i < bus.pixelCount(); ++i)
    {
        uint8_t v = static_cast<uint8_t>((i * 255) / (bus.pixelCount() - 1));
        bus.setPixelColor(i, npb::Rgbcw8Color(v, 255 - v, v / 2, 255 - (v / 2), v / 3));
    }
}

static void runProtocolRgbcw8(const char* name, std::unique_ptr<npb::IProtocol<npb::Rgbcw8Color>> protocol)
{
    Serial.println();
    Serial.print("=== ");
    Serial.print(name);
    Serial.println(" ===");

    auto bus = std::make_unique<npb::PixelBusT<npb::Rgbcw8Color>>(PixelCount, std::move(protocol));
    bus->begin();
    fillGradientRgbcw8(*bus);
    bus->show();
}

// ---------- sketch ----------

void setup()
{
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    Serial.println("In-band settings protocol smoke test");

    // TLC59711 — 4 RGB pixels per chip, per-chip brightness header
    // Custom config: half brightness, default control flags
    npb::Tlc59711Config tlcConfig{};
    tlcConfig.bcRed   = 64;
    tlcConfig.bcGreen = 64;
    tlcConfig.bcBlue  = 64;

    runProtocolRgb8("TLC59711 (bc=64)",
        std::make_unique<npb::Tlc59711Protocol>(
            PixelCount,
            npb::Tlc59711ProtocolSettings{debugBus, tlcConfig}));

    // TLC5947 — 8 RGB pixels per module, 12-bit channels, GPIO latch
    // Using PinNotUsed for latch/OE since we're on DebugTransport
    runProtocolRgb16("TLC5947",
        std::make_unique<npb::Tlc5947RgbProtocol>(
            PixelCount,
            npb::Tlc5947ProtocolSettings{debugBus, npb::PinNotUsed}));

    npb::Tm1814CurrentSettings tm1814Current{};
    tm1814Current.redMilliAmps = 140;
    tm1814Current.greenMilliAmps = 180;
    tm1814Current.blueMilliAmps = 220;
    tm1814Current.whiteMilliAmps = 260;

    {
        npb::RpPioOneWireTransportConfig config{};
        config.pin = DataPin;
        config.pioIndex = 1;
        config.frameBytes = PixelCount * 4;
        config.invert = false;
        config.timing = npb::timing::Ws2812x;
        npb::RpPioOneWireTransport oneWireBus(config);

        runProtocolRgbw8("TM1814 (current preamble)",
            std::make_unique<npb::Tm1814Protocol>(
                PixelCount,
                npb::Tm1814ProtocolSettings{oneWireBus, "WRGB", tm1814Current}));
    }

    {
        npb::RpPioOneWireTransportConfig config{};
        config.pin = DataPin;
        config.pioIndex = 1;
        config.frameBytes = PixelCount * 3;
        config.invert = false;
        config.timing = npb::timing::Ws2812x;
        npb::RpPioOneWireTransport oneWireBus(config);

        runProtocolRgb8("TM1914 (mode preamble)",
            std::make_unique<npb::Tm1914Protocol>(
                PixelCount,
                npb::Tm1914ProtocolSettings{oneWireBus, npb::ChannelOrder::GRB, npb::Tm1914Mode::FdinOnly}));
    }

    runProtocolRgbcw8("SM168x (3ch gain trailer)",
        std::make_unique<npb::Sm168xProtocol<npb::Rgbcw8Color>>(
            PixelCount,
            npb::Sm168xProtocolSettings{
                debugBus,
                npb::ChannelOrder::RGB,
                npb::Sm168xVariant::ThreeChannel,
                {3, 7, 11, 0, 0}}));

    runProtocolRgbcw8("SM168x (4ch gain trailer)",
        std::make_unique<npb::Sm168xProtocol<npb::Rgbcw8Color>>(
            PixelCount,
            npb::Sm168xProtocolSettings{
                debugBus,
                npb::ChannelOrder::RGBW,
                npb::Sm168xVariant::FourChannel,
                {2, 6, 10, 14, 0}}));

    runProtocolRgbcw8("SM168x (5ch gain trailer)",
        std::make_unique<npb::Sm168xProtocol<npb::Rgbcw8Color>>(
            PixelCount,
            npb::Sm168xProtocolSettings{
                debugBus,
                npb::ChannelOrder::RGBCW,
                npb::Sm168xVariant::FiveChannel,
                {1, 5, 9, 13, 17}}));

    Serial.println("\n=== All protocols exercised ===");
}

void loop()
{
    delay(5000);
}
