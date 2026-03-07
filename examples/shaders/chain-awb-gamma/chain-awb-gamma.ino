#include <Arduino.h>
#include <LumaWave.h>

/*
Target: Arduino platforms with one-wire output support.
Requires: RGBW-capable protocol output and one data pin.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Chains `AutoWhiteBalanceShader` + `GammaShader` through `CompositeShader`.
*/

constexpr pixel_count_t ledCount = 24;
constexpr int dataPin = 2;

using ChainShader = Shader::Composite<Rgbw8Color, Shader::AutoWhiteBalance<Rgbw8Color>, Shader::Gamma<Rgbw8Color>>;

Strip<Protocols::Ws2814, Transport::Default, ChainShader> strip(
    ledCount, Transport::DefaultSettings{{.dataPin = dataPin}},
    ChainShader(Shader::AutoWhiteBalance<Rgbw8Color>({.dualWhite = false, .whiteKelvin = 4200}),
                Shader::Gamma<Rgbw8Color>({.gamma = 2.2f, .enableColorGamma = true, .enableBrightnessGamma = false})));
uint16_t frame = 0;

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
            const uint8_t ramp = static_cast<uint8_t>((i * 11U + frame) & 0xFF);
            pixels[i] = Rgbw8Color(ramp, static_cast<uint8_t>(255U - ramp), static_cast<uint8_t>((ramp >> 1) + 32U),
                                   static_cast<uint8_t>(ramp >> 2));
        }

        strip.show();
        ++frame;
        delay(25);
    }
}
