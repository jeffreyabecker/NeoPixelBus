// Phase 4 Smoke Test — exercises each two-wire chip protocol via DebugClockDataTransport.
//
// Each protocol is constructed with a small pixel count, painted with a simple
// gradient, and then show() is called once.  The DebugClockDataTransport prints the
// raw bus traffic to Serial so the wire format can be inspected.

#include <Arduino.h>
#include <memory>
#include <VirtualNeoPixelBus.h>

static constexpr uint16_t PixelCount = 4;

// Shared debug bus — prints all clock/data traffic to Serial.
static npb::DebugClockDataTransport debugBus(Serial);

// ---------- helpers ----------

static void fillGradientRgb(npb::PixelBusT<npb::Rgb8Color>& bus)
{
    for (uint16_t i = 0; i < bus.pixelCount(); ++i)
    {
        uint8_t v = static_cast<uint8_t>((i * 255) / (bus.pixelCount() - 1));
        bus.setPixelColor(i, npb::Rgb8Color(v, 0, 255 - v));
    }
}

static void runProtocolRgb(const char* name, std::unique_ptr<npb::IProtocol<npb::Rgb8Color>> protocol)
{
    Serial.println();
    Serial.print("=== ");
    Serial.print(name);
    Serial.println(" ===");

    auto bus = std::make_unique<npb::PixelBusT<npb::Rgb8Color>>(PixelCount, std::move(protocol));
    bus->begin();
    fillGradientRgb(*bus);
    bus->show();
}

// ---------- sketch ----------

void setup()
{
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    Serial.println("Phase 4 — Two-wire chip protocol smoke test");

    // LPD8806 — 7-bit, MSB set, GRB default
    runProtocolRgb("LPD8806",
        std::make_unique<npb::Lpd8806Protocol>(
            PixelCount, npb::Lpd8806ProtocolSettings{debugBus}));

    // LPD6803 — 5-5-5 packed, 2 bytes per pixel
    runProtocolRgb("LPD6803",
        std::make_unique<npb::Lpd6803Protocol>(
            PixelCount, npb::Lpd6803ProtocolSettings{debugBus}));

    // P9813 — checksum header + BGR, 4 bytes per pixel
    runProtocolRgb("P9813",
        std::make_unique<npb::P9813Protocol>(
            PixelCount, npb::P9813ProtocolSettings{debugBus}));

    // WS2801 — raw 3 bytes, 500 µs latch
    runProtocolRgb("WS2801",
        std::make_unique<npb::Ws2801Protocol>(
            PixelCount, npb::Ws2801ProtocolSettings{debugBus}));

    // SM16716 — bit-level, 25 bits per pixel (pre-packed)
    runProtocolRgb("SM16716",
        std::make_unique<npb::Sm16716Protocol>(
            PixelCount, npb::Sm16716ProtocolSettings{debugBus}));

    Serial.println("\n=== All protocols exercised ===");
}

void loop()
{
    delay(5000);
}
