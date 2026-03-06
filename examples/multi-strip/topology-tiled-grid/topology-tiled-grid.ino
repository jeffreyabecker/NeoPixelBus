#include <Arduino.h>
#include <LumaWave.h>

/*
Target: Arduino platforms with enough RAM for 4096 RGB pixels (RP2040 recommended).
Requires: One-wire transport and one data pin.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Demonstrates `Topology` map for 4x4 tiles where each tile is 16x16.
*/

constexpr uint16_t panelWidth = 16;
constexpr uint16_t panelHeight = 16;
constexpr uint16_t tilesWide = 4;
constexpr uint16_t tilesHigh = 4;
constexpr int dataPin = 2;

constexpr uint16_t canvasWidth = static_cast<uint16_t>(panelWidth * tilesWide);
constexpr uint16_t canvasHeight = static_cast<uint16_t>(panelHeight * tilesHigh);
constexpr uint16_t ledCount = static_cast<uint16_t>(canvasWidth * canvasHeight);

Strip<Protocols::Ws2812<>> strip(ledCount, Transport::DefaultSettings{{.dataPin = dataPin}});

lw::TopologySettings topologySettings{
    .panelWidth = panelWidth,
    .panelHeight = panelHeight,
    .layout = lw::GridMapping::make(lw::GridMapping::AxisOrder::RowsFirst, lw::GridMapping::LinePattern::Serpentine,
                                    lw::GridMapping::QuarterTurn::Deg0),
    .tilesWide = tilesWide,
    .tilesHigh = tilesHigh,
    .tileLayout = lw::GridMapping::make(lw::GridMapping::AxisOrder::RowsFirst, lw::GridMapping::LinePattern::Serpentine,
                                        lw::GridMapping::QuarterTurn::Deg0),
    .mosaicRotation = false,
};
lw::Topology topology(topologySettings);
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

            auto color = pixels[index];
            color['R'] = static_cast<uint8_t>((x + phase) & 0x7F);
            color['G'] = static_cast<uint8_t>((y + phase) & 0x7F);
            color['B'] = static_cast<uint8_t>((x + y + phase) & 0x7F);
            pixels[index] = color;
        }
    }

    strip.show();
    ++phase;
    delay(30);
}
