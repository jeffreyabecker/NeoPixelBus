#include <Arduino.h>
#include <LumaWave.h>

/*
Target: Arduino platforms with SPI transport support.
Requires: `LW_HAS_SPI_TRANSPORT`.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Uses direct `PixelBus<Protocol, Transport, Shader>` construction for RGBCW + CCT balancing.
*/

#if !defined(LW_HAS_SPI_TRANSPORT)
#error "This example requires SPI transport support."
#endif

namespace
{
using ColorType = lw::Rgbcw8Color;
using Protocol = lw::protocols::Ws2812xProtocol<ColorType, lw::Rgbcw8Color>;
using BusTransport = lw::transports::SpiTransport;
using ShaderType = lw::shaders::CCTWhiteBalanceShader<ColorType>;
using BusType = lw::busses::PixelBus<Protocol, BusTransport, ShaderType>;

constexpr lw::PixelCount LedCount = 30;
#if defined(SCK)
constexpr int ClockPin = SCK;
#else
constexpr int ClockPin = 2;
#endif

#if defined(MOSI)
constexpr int DataPin = MOSI;
#else
constexpr int DataPin = 3;
#endif

typename Protocol::SettingsType makeProtocolSettings()
{
    typename Protocol::SettingsType settings{};
    settings.channelOrder = lw::ChannelOrder::RGBCW::value;
    settings.timing = lw::transports::OneWireTiming::Ws2805;
    settings.idleHigh = false;
    return Protocol::SettingsType::template normalizeForColor<ColorType>(std::move(settings),
                                                                         lw::ChannelOrder::RGBCW::value);
}

BusTransport::TransportSettingsType makeTransportSettings()
{
    BusTransport::TransportSettingsType settings{};
    settings.spi = &SPI;
    settings.clockPin = ClockPin;
    settings.dataPin = DataPin;
    settings.clockRateHz = 2400000UL;
    settings.invert = false;
    return settings;
}

ShaderType::SettingsType makeShaderSettings()
{
    ShaderType::SettingsType settings{};
    settings.lowKelvin = 2700;
    settings.highKelvin = 6500;
    settings.colorInterlock = lw::shaders::CCTColorInterlock::MatchWhite;
    return settings;
}

BusType strip(LedCount, makeProtocolSettings(), makeTransportSettings(), ShaderType(makeShaderSettings()));

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
} // namespace

void setup()
{
    strip.begin();
}

void loop()
{
    const uint32_t now = millis();

    const uint8_t whiteBrightness = triangleWave(now, 4000U);
    const uint8_t cctBalance = triangleWave(now + 1000U, 7000U);

    auto& pixels = strip.pixels();
    for (size_t index = 0; index < pixels.size(); ++index)
    {
        auto color = pixels[index];

        color['R'] = 0;
        color['G'] = 0;
        color['B'] = 0;
        color['W'] = cctBalance;
        color['C'] = whiteBrightness;

        pixels[index] = color;
    }

    strip.show();
    delay(16);
}
