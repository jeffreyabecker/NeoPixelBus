#include <Arduino.h>
#include <LumaWave.h>

/*
Target: Arduino platforms with one-wire output support.
Requires: One data pin and uint8 color channels.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Applies deterministic `GammaShader` as a bus shader.
*/

constexpr pixel_count_t ledCount = 48;
constexpr int dataPin = 2;

Strip<Protocols::Ws2812, Transport::Default, Shader::Gamma<Rgb8Color>>
    strip(ledCount, Transport::DefaultSettings{{.dataPin = dataPin}},
          Shader::Gamma<Rgb8Color>(Shader::GammaSettings<Rgb8Color>{
              .gamma = 2.4f, .enableColorGamma = true, .enableBrightnessGamma = false}));
uint8_t phase = 0;

void setup()
{
    strip.begin();
}

void loop()
{
    auto& pixels = strip.pixels();
    const size_t count = pixels.size();

    while (true)
    {
        for (size_t i = 0; i < count; ++i)
        {
            const uint8_t base = static_cast<uint8_t>((phase + i * 4U) & 0xFF);
            pixels[i] = lw::Color(base, static_cast<uint8_t>(255U - base), static_cast<uint8_t>((base >> 1) + 16U));
        }

        strip.show();
        ++phase;
        delay(20);
    }
