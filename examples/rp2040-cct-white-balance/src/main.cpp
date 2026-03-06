#include <Arduino.h>
#include <LumaWave.h>

#if !defined(ARDUINO_ARCH_RP2040)
#error "This example requires an RP2040 target."
#endif

#if !defined(LW_HAS_SPI_TRANSPORT)
#error "This example requires SPI transport support."
#endif

using namespace lw;

namespace
{
    using ColorType = Rgbcw8Color;
    using Protocol = protocols::Ws2805<ColorType>;
    using Transport = SpiTransport;
    using Shader = CCTWhiteBalanceShader<ColorType>;
    using BusType = busses::PixelBus<Protocol, Transport, Shader>;

    constexpr PixelCount LedCount = 30;
    constexpr int ClockPin = 2;
    constexpr int DataPin = 3;

    typename Protocol::SettingsType makeProtocolSettings()
    {
        auto settings = Protocol::defaultSettings();
        settings.channelOrder = ChannelOrder::RGBCW::value;
        return settings;
    }

    Transport::TransportSettingsType makeTransportSettings()
    {
        Transport::TransportSettingsType settings{};
        settings.spi = &SPI;
        settings.clockPin = ClockPin;
        settings.dataPin = DataPin;
        settings.clockRateHz = 2400000UL;
        settings.invert = false;
        return settings;
    }

    Shader::SettingsType makeShaderSettings()
    {
        Shader::SettingsType settings{};
        settings.lowKelvin = 2700;
        settings.highKelvin = 6500;
        settings.colorInterlock = CCTColorInterlock::MatchWhite;
        return settings;
    }

    BusType strip(LedCount,
                  makeProtocolSettings(),
                  makeTransportSettings(),
                  Shader(makeShaderSettings()));

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
}

void setup()
{
    strip.begin();
}

void loop()
{
    const uint32_t now = millis();

    // C is brightness control, W is warm/cool balance control.
    const uint8_t whiteBrightness = triangleWave(now, 4000U);
    const uint8_t cctBalance = triangleWave(now + 1000U, 7000U);

    auto &pixels = strip.pixels();
    for (uint32_t index = 0; index < pixels.size(); ++index)
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
