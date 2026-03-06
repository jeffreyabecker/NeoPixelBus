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

constexpr uint16_t ledCount = 60;
#if defined(SCK)
constexpr int clockPin = SCK;
#else
constexpr int clockPin = 18;
#endif

#if defined(MOSI)
constexpr int dataPin = MOSI;
#else
constexpr int dataPin = 19;
#endif
Strip<Protocols::APA102> strip(ledCount, Transport::DefaultSettings{{false, 8000000UL, static_cast<uint8_t>(MSBFIRST),
                                                                     SPI_MODE0, clockPin, dataPin}});
uint16_t frame = 0;

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
