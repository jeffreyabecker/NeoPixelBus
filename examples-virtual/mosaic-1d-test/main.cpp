// Smoke test: ConcatBus 1D concatenation
// Three strips of UNEVEN lengths on separate "buses" combined into one
// logical strip.  Verifies that uneven-length concatenation works.

#include <Arduino.h>
#include <memory>
#include <VirtualNeoPixelBus.h>

static constexpr uint16_t Strip0Len = 8;
static constexpr uint16_t Strip1Len = 6;
static constexpr uint16_t Strip2Len = 10;   // third strip, different length

static std::unique_ptr<npb::PixelBus> strip0;
static std::unique_ptr<npb::PixelBus> strip1;
static std::unique_ptr<npb::PixelBus> strip2;
static std::unique_ptr<npb::ConcatBus> combined;

void setup()
{
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    Serial.println("===== ConcatBus 1D Smoke Test =====\n");

    // Create three PrintProtocol-backed buses of differing lengths
    auto emitter0 = std::make_unique<npb::PrintProtocol>(
        Strip0Len, nullptr,
        npb::PrintProtocolSettings{ Serial });
    auto emitter1 = std::make_unique<npb::PrintProtocol>(
        Strip1Len, nullptr,
        npb::PrintProtocolSettings{ Serial });
    auto emitter2 = std::make_unique<npb::PrintProtocol>(
        Strip2Len, nullptr,
        npb::PrintProtocolSettings{ Serial });

    strip0 = std::make_unique<npb::PixelBus>(Strip0Len, std::move(emitter0));
    strip1 = std::make_unique<npb::PixelBus>(Strip1Len, std::move(emitter1));
    strip2 = std::make_unique<npb::PixelBus>(Strip2Len, std::move(emitter2));

    // Concat: uneven lengths 8 + 6 + 10 = 24 total (borrowing)
    std::vector<npb::ResourceHandle<npb::IPixelBus>> buses;
    buses.emplace_back(*strip0);  // borrow
    buses.emplace_back(*strip1);
    buses.emplace_back(*strip2);

    combined = std::make_unique<npb::ConcatBus>(std::move(buses));

    combined->begin();

    Serial.print("Total pixels: ");
    Serial.println(combined->pixelCount());   // expect 24

    // Set pixels across boundaries
    Serial.println("\nSetting pixel  0 (strip0[0])  = red");
    combined->setPixelColor(0, npb::Color(255, 0, 0));

    Serial.println("Setting pixel  7 (strip0[7])  = green");
    combined->setPixelColor(7, npb::Color(0, 255, 0));

    Serial.println("Setting pixel  8 (strip1[0])  = blue");
    combined->setPixelColor(8, npb::Color(0, 0, 255));

    Serial.println("Setting pixel 13 (strip1[5])  = yellow");
    combined->setPixelColor(13, npb::Color(255, 255, 0));

    Serial.println("Setting pixel 14 (strip2[0])  = cyan");
    combined->setPixelColor(14, npb::Color(0, 255, 255));

    Serial.println("Setting pixel 23 (strip2[9])  = white");
    combined->setPixelColor(23, npb::Color(255, 255, 255));

    // Verify reads
    Serial.println("\nReading back:");
    auto c0  = combined->getPixelColor(0);
    Serial.print("  pixel  0: R="); Serial.print(c0[0]);
    Serial.print(" G="); Serial.print(c0[1]);
    Serial.print(" B="); Serial.println(c0[2]);

    auto c8  = combined->getPixelColor(8);
    Serial.print("  pixel  8: R="); Serial.print(c8[0]);
    Serial.print(" G="); Serial.print(c8[1]);
    Serial.print(" B="); Serial.println(c8[2]);

    auto c14 = combined->getPixelColor(14);
    Serial.print("  pixel 14: R="); Serial.print(c14[0]);
    Serial.print(" G="); Serial.print(c14[1]);
    Serial.print(" B="); Serial.println(c14[2]);

    auto c23 = combined->getPixelColor(23);
    Serial.print("  pixel 23: R="); Serial.print(c23[0]);
    Serial.print(" G="); Serial.print(c23[1]);
    Serial.print(" B="); Serial.println(c23[2]);

    // Show (triggers all three underlying buses)
    Serial.println("\nshow() — all three strips:");
    combined->show();

    Serial.println("\n===== ConcatBus 1D Smoke Test Complete =====");
}

void loop()
{
    delay(10000);
}
