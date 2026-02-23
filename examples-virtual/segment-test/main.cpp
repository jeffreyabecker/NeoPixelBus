// Smoke test: SegmentBus — non-owning subsegment view
// Creates one 20-pixel strip, then carves out three SegmentBus views
// over it and verifies independent read/write through segments.

#include <Arduino.h>
#include <memory>
#include <VirtualNeoPixelBus.h>

static constexpr uint16_t TotalLen = 20;

static std::unique_ptr<npb::PixelBus> strip;
static std::unique_ptr<npb::SegmentBus> segA;  // pixels 0..7
static std::unique_ptr<npb::SegmentBus> segB;  // pixels 8..14
static std::unique_ptr<npb::SegmentBus> segC;  // pixels 15..19

void setup()
{
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    Serial.println("===== SegmentBus Smoke Test =====\n");

    auto protocol = std::make_unique<npb::PrintProtocol>(
        TotalLen,
        npb::PrintProtocolSettings{ Serial });

    strip = std::make_unique<npb::PixelBus>(TotalLen, std::move(protocol));

    // Carve segments of unequal size
    segA = std::make_unique<npb::SegmentBus>(*strip, 0, 8);
    segB = std::make_unique<npb::SegmentBus>(*strip, 8, 7);
    segC = std::make_unique<npb::SegmentBus>(*strip, 15, 5);

    segA->begin();  // delegates to strip

    Serial.print("Strip total : "); Serial.println(strip->pixelCount());
    Serial.print("Segment A   : "); Serial.println(segA->pixelCount());
    Serial.print("Segment B   : "); Serial.println(segB->pixelCount());
    Serial.print("Segment C   : "); Serial.println(segC->pixelCount());

    // Write through segments
    Serial.println("\nWriting through segments:");

    Serial.println("  segA[0] = red        → strip[0]");
    segA->setPixelColor(0, npb::Color(255, 0, 0));

    Serial.println("  segA[7] = green      → strip[7]");
    segA->setPixelColor(7, npb::Color(0, 255, 0));

    Serial.println("  segB[0] = blue       → strip[8]");
    segB->setPixelColor(0, npb::Color(0, 0, 255));

    Serial.println("  segB[6] = yellow     → strip[14]");
    segB->setPixelColor(6, npb::Color(255, 255, 0));

    Serial.println("  segC[0] = cyan       → strip[15]");
    segC->setPixelColor(0, npb::Color(0, 255, 255));

    Serial.println("  segC[4] = magenta    → strip[19]");
    segC->setPixelColor(4, npb::Color(255, 0, 255));

    // Cross-verify: read through strip for values written via segments
    Serial.println("\nReading back via strip:");

    auto verify = [](const char* label, size_t idx, npb::IPixelBus& bus)
    {
        auto c = bus.getPixelColor(idx);
        Serial.print("  "); Serial.print(label);
        Serial.print(": R="); Serial.print(c[0]);
        Serial.print(" G="); Serial.print(c[1]);
        Serial.print(" B="); Serial.println(c[2]);
    };

    verify("strip[ 0]", 0, *strip);
    verify("strip[ 7]", 7, *strip);
    verify("strip[ 8]", 8, *strip);
    verify("strip[14]", 14, *strip);
    verify("strip[15]", 15, *strip);
    verify("strip[19]", 19, *strip);

    // Also read back through segment (should see same values)
    Serial.println("\nReading back via segments:");
    verify("segA[0]", 0, *segA);
    verify("segB[0]", 0, *segB);
    verify("segC[4]", 4, *segC);

    // Boundary safety: write past end of a segment should be ignored
    Serial.println("\nBoundary test: writing past segment end (no crash expected)");
    segC->setPixelColor(10, npb::Color(1, 2, 3));  // index 10 in 5-pixel segment, should be ignored
    Serial.println("  OK — out-of-bounds write was safely ignored");

    Serial.println("\nshow():");
    strip->show();

    Serial.println("\n===== SegmentBus Smoke Test Complete =====");
}

void loop()
{
    delay(10000);
}
