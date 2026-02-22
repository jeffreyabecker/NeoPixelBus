// Phase 4 Smoke Test — exercises each two-wire chip emitter via DebugClockDataBus.
//
// Each emitter is constructed with a small pixel count, painted with a simple
// gradient, and then show() is called once.  The DebugClockDataBus prints the
// raw bus traffic to Serial so the wire format can be inspected.

#include <Arduino.h>
#include <memory>
#include <VirtualNeoPixelBus.h>

static constexpr uint16_t PixelCount = 4;

// Shared debug bus — prints all clock/data traffic to Serial.
static npb::DebugClockDataBus debugBus(Serial);

// ---------- helpers ----------

static void fillGradient(npb::PixelBus& bus)
{
    for (uint16_t i = 0; i < bus.pixelCount(); ++i)
    {
        uint8_t v = static_cast<uint8_t>((i * 255) / (bus.pixelCount() - 1));
        bus.setPixelColor(i, npb::Color(v, 0, 255 - v));
    }
}

static void runEmitter(const char* name, std::unique_ptr<npb::IEmitPixels> emitter)
{
    Serial.println();
    Serial.print("=== ");
    Serial.print(name);
    Serial.println(" ===");

    auto bus = std::make_unique<npb::PixelBus>(PixelCount, std::move(emitter));
    bus->begin();
    fillGradient(*bus);
    bus->show();
}

// ---------- sketch ----------

void setup()
{
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    Serial.println("Phase 4 — Two-wire chip emitter smoke test");

    // LPD8806 — 7-bit, MSB set, GRB default
    runEmitter("LPD8806",
        std::make_unique<npb::Lpd8806Emitter>(
            debugBus, nullptr, PixelCount));

    // LPD6803 — 5-5-5 packed, 2 bytes per pixel
    runEmitter("LPD6803",
        std::make_unique<npb::Lpd6803Emitter>(
            debugBus, nullptr, PixelCount));

    // P9813 — checksum header + BGR, 4 bytes per pixel
    runEmitter("P9813",
        std::make_unique<npb::P9813Emitter>(
            debugBus, nullptr, PixelCount));

    // WS2801 — raw 3 bytes, 500 µs latch
    runEmitter("WS2801",
        std::make_unique<npb::Ws2801Emitter>(
            debugBus, nullptr, PixelCount));

    // SM16716 — bit-level, 25 bits per pixel (pre-packed)
    runEmitter("SM16716",
        std::make_unique<npb::Sm16716Emitter>(
            debugBus, nullptr, PixelCount));

    Serial.println("\n=== All emitters exercised ===");
}

void loop()
{
    delay(5000);
}
