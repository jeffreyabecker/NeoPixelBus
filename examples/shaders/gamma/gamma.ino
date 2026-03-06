#include <Arduino.h>
#include <LumaWave.h>

/*
Target: Arduino platforms with one-wire output support.
Requires: One data pin and uint8 color channels.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Applies deterministic `GammaShader` as a bus shader.
*/

namespace
{
using ColorType = lw::Rgb8Color;
using Protocol = lw::protocols::Ws2812x<ColorType>;
using ShaderType = lw::shaders::GammaShader<ColorType>;
using StripType = lw::busses::PixelBus<Protocol, lw::busses::PlatformDefaultTransport, ShaderType>;

constexpr lw::PixelCount LedCount = 48;
constexpr int DataPin = 2;

StripType::TransportSettingsType makeTransportSettings()
{
    StripType::TransportSettingsType settings{};
    settings.dataPin = DataPin;
    settings.invert = false;
    return settings;
}

ShaderType::SettingsType makeShaderSettings()
{
    ShaderType::SettingsType settings{};
    settings.gamma = 2.4f;
    settings.enableColorGamma = true;
    settings.enableBrightnessGamma = false;
    return settings;
}

StripType strip(LedCount, makeTransportSettings(), ShaderType(makeShaderSettings()));
uint8_t phase = 0;
} // namespace

void setup()
{
    strip.begin();
}

void loop()
{
    auto& pixels = strip.pixels();
    const size_t count = pixels.size();

    for (size_t i = 0; i < count; ++i)
    {
        const uint8_t base = static_cast<uint8_t>((phase + i * 4U) & 0xFF);
        auto color = pixels[i];
        color['R'] = base;
        color['G'] = static_cast<uint8_t>(255U - base);
        color['B'] = static_cast<uint8_t>((base >> 1) + 16U);
        pixels[i] = color;
    }

    strip.show();
    ++phase;
    delay(20);
}
