#include <Arduino.h>
#include <LumaWave.h>

/*
Target: Arduino platforms with enough RAM for 4096 RGB pixels (RP2040 recommended).
Requires: One-wire transport and one data pin.
Namespace mode: Explicit-safe (`lw::...`).
API assumptions: Demonstrates `Topology` map for 4x4 tiles where each tile is 16x16.
*/

namespace
{
using Protocol = lw::protocols::Ws2812x<>;
using StripType = lw::busses::PixelBus<Protocol>;

constexpr uint16_t PanelWidth = 16;
constexpr uint16_t PanelHeight = 16;
constexpr uint16_t TilesWide = 4;
constexpr uint16_t TilesHigh = 4;
constexpr int DataPin = 2;

constexpr uint16_t CanvasWidth = static_cast<uint16_t>(PanelWidth * TilesWide);
constexpr uint16_t CanvasHeight = static_cast<uint16_t>(PanelHeight * TilesHigh);
constexpr lw::PixelCount LedCount = static_cast<lw::PixelCount>(CanvasWidth * CanvasHeight);

StripType::TransportSettingsType makeTransportSettings()
{
    StripType::TransportSettingsType settings{};
    settings.dataPin = DataPin;
    settings.invert = false;
    return settings;
}

lw::TopologySettings makeTopologySettings()
{
    lw::TopologySettings settings{};
    settings.panelWidth = PanelWidth;
    settings.panelHeight = PanelHeight;
    settings.layout =
        lw::GridMapping::make(lw::GridMapping::AxisOrder::RowsFirst, lw::GridMapping::LinePattern::Serpentine,
                              lw::GridMapping::QuarterTurn::Deg0);
    settings.tilesWide = TilesWide;
    settings.tilesHigh = TilesHigh;
    settings.tileLayout =
        lw::GridMapping::make(lw::GridMapping::AxisOrder::RowsFirst, lw::GridMapping::LinePattern::Serpentine,
                              lw::GridMapping::QuarterTurn::Deg0);
    settings.mosaicRotation = false;
    return settings;
}

StripType strip(LedCount, makeTransportSettings());
lw::Topology topology(makeTopologySettings());
uint16_t phase = 0;
} // namespace

void setup()
{
    strip.begin();
}

void loop()
{
    auto& pixels = strip.pixels();

    for (uint16_t y = 0; y < CanvasHeight; ++y)
    {
        for (uint16_t x = 0; x < CanvasWidth; ++x)
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
