#include <Arduino.h>
#include <LumaWave.h>

/*
Target: Arduino platforms with enough RAM for 1024 RGB pixels (RP2040 recommended).
Requires: One-wire transport and four data pins.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Demonstrates `Topology` map for a 2x2 tile canvas using `CompositeBus` over 4 strips of 256 pixels.
*/

constexpr uint16_t panelWidth = 16;
constexpr uint16_t panelHeight = 16;
constexpr uint16_t tilesWide = 2;
constexpr uint16_t tilesHigh = 2;
constexpr uint16_t stripLedCount = static_cast<uint16_t>(panelWidth * panelHeight);

constexpr int dataPin0 = 2;
constexpr int dataPin1 = 3;
constexpr int dataPin2 = 4;
constexpr int dataPin3 = 5;

constexpr uint16_t canvasWidth = static_cast<uint16_t>(panelWidth * tilesWide);
constexpr uint16_t canvasHeight = static_cast<uint16_t>(panelHeight * tilesHigh);
constexpr uint16_t ledCount = static_cast<uint16_t>(canvasWidth * canvasHeight);

auto strip = CompositeStrip<Strip<Protocols::Ws2812>, Strip<Protocols::Ws2812>, Strip<Protocols::Ws2812>,
                            Strip<Protocols::Ws2812>>(
    Strip<Protocols::Ws2812>(stripLedCount, Transport::DefaultSettings{{.dataPin = dataPin0}}),
    Strip<Protocols::Ws2812>(stripLedCount, Transport::DefaultSettings{{.dataPin = dataPin1}}),
    Strip<Protocols::Ws2812>(stripLedCount, Transport::DefaultSettings{{.dataPin = dataPin2}}),
    Strip<Protocols::Ws2812>(stripLedCount, Transport::DefaultSettings{{.dataPin = dataPin3}}));

lw::Topology topology({
    .panelWidth = panelWidth,
    .panelHeight = panelHeight,
    .layout = GridMapping::RowsFirstSerpentine,
    .tilesWide = tilesWide,
    .tilesHigh = tilesHigh,
    .tileLayout = GridMapping::RowsFirstProgressive,
    .mosaicRotation = false,
});
uint16_t phase = 0;

void setup()
{
    strip.begin();
}

void loop()
{
    auto& pixels = strip.pixels();

    for (uint16_t y = 0; y < canvasHeight; ++y)
    {
        for (uint16_t x = 0; x < canvasWidth; ++x)
        {
            const size_t index = topology.map(static_cast<int16_t>(x), static_cast<int16_t>(y));
            if (index == lw::Topology::InvalidIndex)
            {
                continue;
            }

            pixels[index] = Color(static_cast<uint8_t>((x + phase) & 0x7F), static_cast<uint8_t>((y + phase) & 0x7F),
                                  static_cast<uint8_t>((x + y + phase) & 0x7F));
        }
    }

    strip.show();
    ++phase;
    delay(30);
}
