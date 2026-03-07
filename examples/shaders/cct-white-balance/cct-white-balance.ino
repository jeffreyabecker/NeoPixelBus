#include <Arduino.h>
#include <LumaWave.h>

/*
Target: Arduino platforms with one-wire output support.
Requires: Platform-default transport with a valid data pin.
Namespace mode: Public aliases from `LumaWave.h`.
API assumptions: Demonstrates RGBCW CCT white-balance shader using `Protocols::Ws2805`.
*/

using ColorType = Rgbcw8Color;
using Protocol = Protocols::Ws2805;
using TransportType = Transport::Default;
using ShaderType = Shader::CCTBalance<Protocols::Ws2805::ColorType>;
using BusType = Strip<Protocols::Ws2805, TransportType, ShaderType>;

constexpr pixel_count_t ledCount = 30;
constexpr int dataPin = 2;

BusType strip(ledCount, Transport::DefaultSettings{{.dataPin = dataPin}},
              ShaderType(ShaderType::SettingsType{
                  .lowKelvin = 2700,
                  .highKelvin = 6500,
                  .colorInterlock = Shader::CCTInterlock::MatchWhite,
              }));

uint8_t triangleWave(uint32_t t, uint32_t periodMs)
{
    const uint32_t wrapped = (periodMs > 0U) ? (t % periodMs) : 0U;
    const uint32_t half = periodMs / 2U;
    if (half == 0U)
    {
        return 0U;
    }

    const uint32_t rising = (wrapped < half) ? wrapped : (periodMs - wrapped);
    return static_cast<uint8_t>((rising * 255U) / half);
}

void setup()
{
    strip.begin();
}

void loop()
{
    constexpr uint32_t whitePeriodMs = 4000U;
    constexpr uint32_t cctPeriodMs = 7000U;
    constexpr uint32_t cctPhaseOffsetMs = 1000U;
    while (true)
    {

        const uint32_t now = millis();

        const uint8_t whiteBrightness = triangleWave(now, whitePeriodMs);
        const uint8_t cctBalance = triangleWave(now + cctPhaseOffsetMs, cctPeriodMs);
        auto color = Rgbcw8Color(0, 0, 0, cctBalance, whiteBrightness);

        fillPixels(strip.pixels(), color);

        strip.show();
        delay(16);
    }
