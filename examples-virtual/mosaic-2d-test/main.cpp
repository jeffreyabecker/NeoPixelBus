// Smoke test: MosaicBus 2D mosaic
// Three 4x4 panels arranged in a 3x1 grid.

#include <Arduino.h>
#include <memory>
#include <VirtualNeoPixelBus.h>

static constexpr uint16_t PanelW = 4;
static constexpr uint16_t PanelH = 4;
static constexpr uint16_t PanelPixels = PanelW * PanelH;

static std::unique_ptr<npb::PixelBus> panel0;
static std::unique_ptr<npb::PixelBus> panel1;
static std::unique_ptr<npb::PixelBus> panel2;
static std::unique_ptr<npb::MosaicBus> mosaic;

static npb::PrintProtocolSettings makeSettings()
{
    return npb::PrintProtocolSettings{ Serial };
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    Serial.println("===== MosaicBus 2D Smoke Test =====\n");

    // Create three 4x4 panels with PrintProtocol
    panel0 = std::make_unique<npb::PixelBus>(
        PanelPixels,
        std::make_unique<npb::PrintProtocol>(PanelPixels, makeSettings()));
    panel1 = std::make_unique<npb::PixelBus>(
        PanelPixels,
        std::make_unique<npb::PrintProtocol>(PanelPixels, makeSettings()));
    panel2 = std::make_unique<npb::PixelBus>(
        PanelPixels,
        std::make_unique<npb::PrintProtocol>(PanelPixels, makeSettings()));

    // Arrange as 3 wide, 1 high, using ColumnMajorAlternating per panel
    npb::MosaicBusConfig<> config{};
    config.panelWidth = PanelW;
    config.panelHeight = PanelH;
    config.layout = npb::PanelLayout::ColumnMajorAlternating;
    config.tilesWide = 3;
    config.tilesHigh = 1;
    config.tileLayout = npb::PanelLayout::RowMajor;

    std::vector<npb::ResourceHandle<npb::IPixelBus<>>> buses;
    buses.emplace_back(*panel0);
    buses.emplace_back(*panel1);
    buses.emplace_back(*panel2);

    mosaic = std::make_unique<npb::MosaicBus>(std::move(config), std::move(buses));

    mosaic->begin();

    Serial.print("Mosaic size: ");
    Serial.print(mosaic->width());
    Serial.print(" x ");
    Serial.println(mosaic->height());
    Serial.print("Total pixels: ");
    Serial.println(mosaic->pixelCount());

    // --- 2D pixel access ------------------------------------------

    Serial.println("\nSetting pixels via 2D coordinates:");

    // Panel 0 — top-left corner
    Serial.println("  (0, 0) = red");
    mosaic->setPixelColor(0, 0, npb::Color(255, 0, 0));

    // Panel 0 — bottom-right of first panel
    Serial.println("  (3, 3) = green");
    mosaic->setPixelColor(3, 3, npb::Color(0, 255, 0));

    // Panel 1 — first pixel of second panel
    Serial.println("  (4, 0) = blue");
    mosaic->setPixelColor(4, 0, npb::Color(0, 0, 255));

    // Panel 2 — middle of third panel
    Serial.println("  (10, 2) = yellow");
    mosaic->setPixelColor(10, 2, npb::Color(255, 255, 0));

    // --- Read back ---
    Serial.println("\nReading back via 2D:");
    auto c00 = mosaic->getPixelColor(0, 0);
    Serial.print("  (0,0): R="); Serial.print(c00[0]);
    Serial.print(" G="); Serial.print(c00[1]);
    Serial.print(" B="); Serial.println(c00[2]);

    auto c40 = mosaic->getPixelColor(4, 0);
    Serial.print("  (4,0): R="); Serial.print(c40[0]);
    Serial.print(" G="); Serial.print(c40[1]);
    Serial.print(" B="); Serial.println(c40[2]);

    // --- Show all ---
    Serial.println("\nshow() — all three panels:");
    mosaic->show();

    // --- Out-of-bounds 2D ---
    Serial.println("\nOut-of-bounds 2D (should be black):");
    auto oob = mosaic->getPixelColor(20, 0);
    Serial.print("  (20,0): R="); Serial.print(oob[0]);
    Serial.print(" G="); Serial.print(oob[1]);
    Serial.print(" B="); Serial.println(oob[2]);

    Serial.println("\n===== MosaicBus 2D Smoke Test Complete =====");
}

void loop()
{
    delay(10000);
}
