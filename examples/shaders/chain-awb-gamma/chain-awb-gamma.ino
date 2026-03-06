#include <Arduino.h>
#include <LumaWave.h>

/*
Target: Arduino platforms with one-wire output support.
Requires: RGBW-capable protocol output and one data pin.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Chains `AutoWhiteBalanceShader` + `GammaShader` through `AggregateShader`.
*/

namespace
{
using ColorType = lw::Rgbw8Color;
using Protocol =
    lw::protocols::Ws2812x<ColorType, lw::ChannelOrder::RGBW, &lw::transports::timing::Ws2814, lw::Rgbw8Color, false>;
using AwbShaderType = lw::shaders::AutoWhiteBalanceShader<ColorType>;
using GammaShaderType = lw::shaders::GammaShader<ColorType>;
using ChainShaderType = lw::shaders::AggregateShader<ColorType>;
using StripType = lw::busses::PixelBus<Protocol, lw::busses::PlatformDefaultTransport, ChainShaderType>;

constexpr lw::PixelCount LedCount = 24;
constexpr int DataPin = 2;

StripType::TransportSettingsType makeTransportSettings()
{
    StripType::TransportSettingsType settings{};
    settings.dataPin = DataPin;
    settings.invert = false;
    return settings;
}

AwbShaderType::SettingsType makeAwbSettings()
{
    AwbShaderType::SettingsType settings{};
    settings.dualWhite = false;
    settings.whiteKelvin = 4200;
    return settings;
}

GammaShaderType::SettingsType makeGammaSettings()
{
    GammaShaderType::SettingsType settings{};
    settings.gamma = 2.2f;
    settings.enableColorGamma = true;
    settings.enableBrightnessGamma = false;
    return settings;
}

AwbShaderType awb(makeAwbSettings());
GammaShaderType gamma(makeGammaSettings());

ChainShaderType::SettingsType makeChainSettings()
{
    ChainShaderType::SettingsType settings{};
    settings.shaders.push_back(&awb);
    settings.shaders.push_back(&gamma);
    return settings;
}

StripType strip(LedCount, makeTransportSettings(), ChainShaderType(makeChainSettings()));
uint16_t frame = 0;
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
        const uint8_t ramp = static_cast<uint8_t>((i * 11U + frame) & 0xFF);
        auto color = pixels[i];
        color['R'] = ramp;
        color['G'] = static_cast<uint8_t>(255U - ramp);
        color['B'] = static_cast<uint8_t>((ramp >> 1) + 32U);
        color['W'] = static_cast<uint8_t>(ramp >> 2);
        pixels[i] = color;
    }

    strip.show();
    ++frame;
    delay(25);
}
