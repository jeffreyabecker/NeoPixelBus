#include <Arduino.h>
#include <VirtualNeoPixelBus.h>

// ---------- strip configuration ----------
static constexpr uint16_t PixelCount = 8;

static npb::ColorOrderTransform rawTransform(
    npb::ColorOrderTransformConfig{
        .channelCount = 3,
        .channelOrder = {1, 0, 2, 0, 0}  // GRB
    });

// ---------- shaders ----------
// Gamma correction via CIE L* curve
static npb::GammaShader<npb::GammaCieLabMethod> gammaShader;

// Current limiter: 500 mA budget, 20 mA per channel at full brightness
static npb::CurrentLimiterShader limiter(500, 20);

// Shader chain — applied in order: gamma first, then current limiter
static npb::IShader* shaders[] = { &gammaShader, &limiter };

// ShadedTransform wraps the raw transform with the shader chain
static npb::ShadedTransform shadedTransform(rawTransform, shaders);

// ---------- emitter + bus ----------
static npb::PrintEmitter emitter(Serial);
static npb::PixelBus bus(PixelCount, shadedTransform, emitter);

void setup()
{
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    bus.begin();

    // Fill strip with a gradient so the current limiter has something to clamp
    for (uint16_t i = 0; i < PixelCount; ++i)
    {
        uint8_t v = static_cast<uint8_t>((i * 255) / (PixelCount - 1));
        bus.setPixelColor(i, npb::Color(v, v, v));
    }

    Serial.println("=== Shaded output (CIE L* gamma + 500 mA limiter) ===");
    bus.show();

    // Verify original colors are not mutated by the shader pipeline
    Serial.println("\n=== Original colors (should be unmodified) ===");
    for (uint16_t i = 0; i < PixelCount; ++i)
    {
        npb::Color c = bus.getPixelColor(i);
        Serial.print("pixel ");
        Serial.print(i);
        Serial.print(": R=");
        Serial.print(c[npb::Color::IdxR]);
        Serial.print(" G=");
        Serial.print(c[npb::Color::IdxG]);
        Serial.print(" B=");
        Serial.println(c[npb::Color::IdxB]);
    }
}

void loop()
{
    delay(5000);
}
