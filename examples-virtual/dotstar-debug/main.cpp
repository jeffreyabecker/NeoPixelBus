#include <Arduino.h>
#include <VirtualNeoPixelBus.h>

// ---------- strip configuration ----------
static constexpr uint16_t PixelCount = 4;

// DotStar BGR channel order (matches APA102 default)
static npb::DotStarTransform dotStarTransform(
    npb::DotStarTransformConfig{
        .channelOrder = {2, 1, 0},    // BGR
        .mode = npb::DotStarMode::FixedBrightness,
    });

// Debug bus — prints all clock/data bus operations
static npb::DebugClockDataBus debugBus(Serial);

// ClockDataEmitter wraps bus + DotStar protocol framing
static npb::ClockDataEmitter emitter(debugBus, npb::protocol::DotStar, PixelCount);

// PixelBus
static npb::PixelBus bus(PixelCount, dotStarTransform, emitter);

void setup()
{
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    bus.begin();

    // --- Test 1: Fixed brightness mode (0xFF prefix) ---
    Serial.println("=== DotStar FixedBrightness (BGR) ===");
    bus.setPixelColor(0, npb::Color(255,   0,   0));       // Red
    bus.setPixelColor(1, npb::Color(  0, 255,   0));       // Green
    bus.setPixelColor(2, npb::Color(  0,   0, 255));       // Blue
    bus.setPixelColor(3, npb::Color(128,  64,  32));       // Mixed
    bus.show();

    // Expected pixel bytes (after start frame):
    //   pixel 0: FF 00 00 FF   (prefix=FF, B=0, G=0, R=255)
    //   pixel 1: FF 00 FF 00   (prefix=FF, B=0, G=255, R=0)
    //   pixel 2: FF FF 00 00   (prefix=FF, B=255, G=0, R=0)
    //   pixel 3: FF 20 40 80   (prefix=FF, B=32, G=64, R=128)

    Serial.println("\n=== Verify original colors unchanged ===");
    for (uint16_t i = 0; i < PixelCount; ++i)
    {
        npb::Color c = bus.getPixelColor(i);
        Serial.print("pixel ");
        Serial.print(i);
        Serial.print(": R=");
        Serial.print(c.R);
        Serial.print(" G=");
        Serial.print(c.G);
        Serial.print(" B=");
        Serial.println(c.B);
    }
}

void loop()
{
    delay(5000);
}
