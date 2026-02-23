// Phase 5.1 Smoke Test — exercises TLC59711 and TLC5947 protocols via DebugClockDataTransport.
//
// TLC59711: per-chip header + reversed 16-bit BGR data
// TLC5947:  12-bit packed channels + GPIO latch pin
//
// One-wire transforms (TM1814, TM1914, SM168x) are deferred to Phase 6.

#include <Arduino.h>
#include <memory>
#include <VirtualNeoPixelBus.h>

static constexpr uint16_t PixelCount = 4;

// Shared debug bus — prints all clock/data traffic to Serial.
static npb::DebugClockDataTransport debugBus(Serial);

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

// ---------- sketch ----------

void setup()
{
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    Serial.println("Phase 5.1 — In-band settings protocol smoke test");

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

    Serial.println("\n=== All protocols exercised ===");
}

void loop()
{
    delay(5000);
}
