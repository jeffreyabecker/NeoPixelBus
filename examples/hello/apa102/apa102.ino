#include <Arduino.h>
#include <LumaWave.h>

/*
Target: Arduino platforms with SPI support.
Requires: `LW_HAS_SPI_TRANSPORT` and valid SPI pins.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Uses `lw::transports::SpiTransport` settings with `SPI` instance.
*/

#if !defined(LW_HAS_SPI_TRANSPORT)
#error "This example requires SPI transport support (LW_HAS_SPI_TRANSPORT)."
#endif

namespace
{
using Protocol = lw::protocols::DotStar<>;
using Transport = lw::transports::SpiTransport;
using StripType = lw::busses::PixelBus<Protocol, Transport>;

constexpr lw::PixelCount LedCount = 60;
#if defined(SCK)
constexpr int ClockPin = SCK;
#else
constexpr int ClockPin = 18;
#endif

#if defined(MOSI)
constexpr int DataPin = MOSI;
#else
constexpr int DataPin = 19;
#endif

StripType::TransportSettingsType makeTransportSettings()
{
    StripType::TransportSettingsType settings{};
    settings.spi = &SPI;
    settings.clockPin = ClockPin;
    settings.dataPin = DataPin;
    settings.clockRateHz = 8000000UL;
    settings.invert = false;
    return settings;
}

StripType strip(LedCount, makeTransportSettings());
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
        auto color = pixels[i];
        color['R'] = static_cast<uint8_t>((i + frame) & 0x3F);
        color['G'] = static_cast<uint8_t>((2 * i + frame) & 0x3F);
        color['B'] = static_cast<uint8_t>((3 * i + frame) & 0x3F);
        pixels[i] = color;
    }

    strip.show();
    ++frame;
    delay(20);
}
