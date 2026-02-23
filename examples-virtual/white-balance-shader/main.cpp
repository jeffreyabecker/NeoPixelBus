#include <Arduino.h>
#include <memory>
#include <VirtualNeoPixelBus.h>

static constexpr uint16_t PixelCount = 6;

static npb::WhiteBalanceShader singleWhiteBalance(3200);
static npb::IShader* singleShaders[] = { &singleWhiteBalance };

static npb::WhiteBalanceShader dualWhiteBalance(2700, 6500);
static npb::IShader* dualShaders[] = { &dualWhiteBalance };

template <size_t N>
void runDemo(const char* title, npb::IShader* const (&shaders)[N])
{
    auto protocol = std::make_unique<npb::PrintProtocol>(
        PixelCount,
        std::make_unique<npb::ShaderChain>(shaders),
        npb::PrintProtocolSettings{ Serial });

    npb::PixelBus bus(PixelCount, std::move(protocol));
    bus.begin();

    bus.setPixelColor(0, npb::Color(255, 255, 255, 0, 0));
    bus.setPixelColor(1, npb::Color(255, 200, 120, 32, 0));
    bus.setPixelColor(2, npb::Color(64, 128, 255, 128, 0));
    bus.setPixelColor(3, npb::Color(255, 128, 32, 255, 0));
    bus.setPixelColor(4, npb::Color(200, 220, 255, 32, 224));
    bus.setPixelColor(5, npb::Color(100, 180, 255, 180, 48));

    Serial.println();
    Serial.println(title);
    bus.show();
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    runDemo("=== White balance: single white channel @ 3200K ===", singleShaders);
    runDemo("=== White balance: dual white channels @ 2700K / 6500K ===", dualShaders);
}

void loop()
{
    delay(5000);
}
