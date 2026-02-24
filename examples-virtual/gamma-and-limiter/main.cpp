#include <Arduino.h>
#include <memory>
#include <VirtualNeoPixelBus.h>

using ColorType = npb::Color;
using BusType = npb::PixelBusT<ColorType>;

static constexpr uint16_t PixelCount = 8;

static npb::GammaShader<ColorType> gammaShader(2.6f, true, false);
static npb::CurrentLimiterShader<ColorType> limiterShader(500, {20, 20, 20, 0, 0});
static npb::IShader<ColorType>* shaders[] = { &gammaShader, &limiterShader };

static std::unique_ptr<BusType> bus;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }

    auto protocol = std::make_unique<npb::WithShader<ColorType, npb::PrintProtocol>>(
        PixelCount,
        std::make_unique<npb::ShaderChain<ColorType>>(shaders),
        npb::PrintProtocolSettings{Serial});

    bus = std::make_unique<BusType>(PixelCount, std::move(protocol));
    bus->begin();

    for (uint16_t i = 0; i < PixelCount; ++i)
    {
        uint8_t value = static_cast<uint8_t>((i * 255) / (PixelCount - 1));
        bus->setPixelColor(i, ColorType(value, value, value));
    }

    Serial.println("=== Shaded output (WLED gamma 2.6 + 500 mA limiter) ===");
    bus->show();

    Serial.println("\n=== Original colors (should be unmodified) ===");
    for (uint16_t i = 0; i < PixelCount; ++i)
    {
        ColorType color = bus->getPixelColor(i);
        Serial.print("pixel ");
        Serial.print(i);
        Serial.print(": R=");
        Serial.print(color['R']);
        Serial.print(" G=");
        Serial.print(color['G']);
        Serial.print(" B=");
        Serial.println(color['B']);
    }
}

void loop()
{
    delay(5000);
}
