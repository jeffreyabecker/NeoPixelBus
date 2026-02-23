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

// Shared debug bus — prints all clock/data traffic to Serial.
static npb::DebugClockDataTransport debugBus(Serial);
static npb::DebugSelfClockingTransport debugSelfBus(Serial);

// ---------- helpers ----------

static void fillGradient(npb::PixelBus& bus)
{
    for (uint16_t i = 0; i < bus.pixelCount(); ++i)
    {
        uint8_t v = static_cast<uint8_t>((i * 255) / (bus.pixelCount() - 1));
        bus.setPixelColor(i, npb::Color(v, 0, 255 - v));
    }
}

static void runProtocol(const char* name, std::unique_ptr<npb::IProtocol> protocol)
{
    Serial.println();
    Serial.print("=== ");
    Serial.print(name);
    Serial.println(" ===");

    auto bus = std::make_unique<npb::PixelBus>(PixelCount, std::move(protocol));
    bus->begin();
    fillGradient(*bus);
    bus->show();
}

static void fillGradientRgbw(npb::PixelBus& bus)
{
    for (uint16_t i = 0; i < bus.pixelCount(); ++i)
    {
        uint8_t v = static_cast<uint8_t>((i * 255) / (bus.pixelCount() - 1));
        bus.setPixelColor(i, npb::Color(v, 255 - v, v / 2, 255 - (v / 2), v / 3));
    }
}

static void runProtocolRgbw(const char* name, std::unique_ptr<npb::IProtocol> protocol)
{
    Serial.println();
    Serial.print("=== ");
    Serial.print(name);
    Serial.println(" ===");

    auto bus = std::make_unique<npb::PixelBus>(PixelCount, std::move(protocol));
    bus->begin();
    fillGradientRgbw(*bus);
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

    runProtocol("TLC59711 (bc=64)",
        std::make_unique<npb::Tlc59711Protocol>(
            PixelCount, nullptr,
            npb::Tlc59711ProtocolSettings{debugBus, tlcConfig}));

    // TLC5947 — 8 RGB pixels per module, 12-bit channels, GPIO latch
    // Using PinNotUsed for latch/OE since we're on DebugClockDataTransport
    runProtocol("TLC5947",
        std::make_unique<npb::Tlc5947Protocol>(
            PixelCount, nullptr,
            npb::Tlc5947ProtocolSettings{debugBus, npb::PinNotUsed}));

    npb::Tm1814CurrentSettings tm1814Current{};
    tm1814Current.redMilliAmps = 140;
    tm1814Current.greenMilliAmps = 180;
    tm1814Current.blueMilliAmps = 220;
    tm1814Current.whiteMilliAmps = 260;

    runProtocolRgbw("TM1814 (current preamble)",
        std::make_unique<npb::Tm1814Protocol>(
            PixelCount, nullptr,
            npb::Tm1814ProtocolSettings{debugSelfBus, "WRGB", tm1814Current}));

    runProtocol("TM1914 (mode preamble)",
        std::make_unique<npb::Tm1914Protocol>(
            PixelCount, nullptr,
            npb::Tm1914ProtocolSettings{debugSelfBus, npb::ChannelOrder::GRB, npb::Tm1914Mode::FdinOnly}));

    runProtocol("SM168x (3ch gain trailer)",
        std::make_unique<npb::Sm168xProtocol>(
            PixelCount, nullptr,
            npb::Sm168xProtocolSettings{
                debugBus,
                npb::ChannelOrder::RGB,
                npb::Sm168xVariant::ThreeChannel,
                {3, 7, 11, 0, 0}}));

    runProtocolRgbw("SM168x (4ch gain trailer)",
        std::make_unique<npb::Sm168xProtocol>(
            PixelCount, nullptr,
            npb::Sm168xProtocolSettings{
                debugBus,
                npb::ChannelOrder::RGBW,
                npb::Sm168xVariant::FourChannel,
                {2, 6, 10, 14, 0}}));

    runProtocolRgbw("SM168x (5ch gain trailer)",
        std::make_unique<npb::Sm168xProtocol>(
            PixelCount, nullptr,
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
