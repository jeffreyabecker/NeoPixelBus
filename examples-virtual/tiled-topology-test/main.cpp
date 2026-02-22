// Smoke test: PanelTopology and TiledTopology
// Verifies coordinate mapping against known layout patterns.

#include <Arduino.h>
#include <VirtualNeoPixelBus.h>

// ---------------------------------------------------------------------------
// Helper: print a panel's mapping grid
// ---------------------------------------------------------------------------
static void printGrid(const char* label,
                      npb::PanelLayout layout,
                      uint16_t w, uint16_t h)
{
    npb::PanelTopology topo(w, h, layout);

    Serial.print("--- ");
    Serial.print(label);
    Serial.print(" (");
    Serial.print(w);
    Serial.print("x");
    Serial.print(h);
    Serial.println(") ---");

    for (uint16_t y = 0; y < h; ++y)
    {
        for (uint16_t x = 0; x < w; ++x)
        {
            uint16_t idx = topo.map(x, y);
            if (idx < 10) Serial.print(' ');
            Serial.print(idx);
            Serial.print(' ');
        }
        Serial.println();
    }
    Serial.println();
}

// ---------------------------------------------------------------------------
// Verify PanelTopology clamping and bounds checking
// ---------------------------------------------------------------------------
static void testPanelBounds()
{
    npb::PanelTopology topo(4, 4, npb::PanelLayout::RowMajor);

    Serial.println("=== PanelTopology bounds ===");

    // Clamp: (-1, -1) → (0, 0) → index 0
    Serial.print("map(-1,-1) clamped = ");
    Serial.println(topo.map(-1, -1));

    // Clamp: (10, 10) → (3, 3) → index 15
    Serial.print("map(10,10) clamped = ");
    Serial.println(topo.map(10, 10));

    // Probe: valid
    auto ok = topo.mapProbe(2, 1);
    Serial.print("mapProbe(2,1) = ");
    if (ok) Serial.println(*ok);
    else    Serial.println("nullopt");

    // Probe: out of bounds
    auto bad = topo.mapProbe(-1, 0);
    Serial.print("mapProbe(-1,0) = ");
    if (bad)  Serial.println(*bad);
    else      Serial.println("nullopt (expected)");

    Serial.println();
}

// ---------------------------------------------------------------------------
// Verify TiledTopology (replaces NeoTiles)
// ---------------------------------------------------------------------------
static void testTiledTopology()
{
    Serial.println("=== TiledTopology (NeoTiles equivalent) ===");

    // 2x2 grid of 4x4 panels, RowMajor panels, RowMajor tile layout
    npb::TiledTopology tiled({
        .panelWidth = 4, .panelHeight = 4,
        .tilesWide = 2, .tilesHigh = 2,
        .panelLayout = npb::PanelLayout::RowMajor,
        .tileLayout  = npb::PanelLayout::RowMajor,
        .mosaicRotation = false
    });

    Serial.print("Total size: ");
    Serial.print(tiled.width());
    Serial.print("x");
    Serial.print(tiled.height());
    Serial.print(" = ");
    Serial.print(tiled.pixelCount());
    Serial.println(" pixels");

    // Print the full grid mapping
    for (uint16_t y = 0; y < tiled.height(); ++y)
    {
        for (uint16_t x = 0; x < tiled.width(); ++x)
        {
            uint16_t idx = tiled.map(x, y);
            if (idx < 10) Serial.print(' ');
            Serial.print(idx);
            Serial.print(' ');
        }
        Serial.println();
    }
    Serial.println();
}

// ---------------------------------------------------------------------------
// Verify TiledTopology with mosaic rotation (replaces NeoMosaic)
// ---------------------------------------------------------------------------
static void testMosaicRotation()
{
    Serial.println("=== TiledTopology with mosaicRotation (NeoMosaic equivalent) ===");

    npb::TiledTopology mosaic({
        .panelWidth = 4, .panelHeight = 4,
        .tilesWide = 2, .tilesHigh = 2,
        .panelLayout = npb::PanelLayout::RowMajor,
        .tileLayout  = npb::PanelLayout::RowMajorAlternating,
        .mosaicRotation = true
    });

    for (uint16_t y = 0; y < mosaic.height(); ++y)
    {
        for (uint16_t x = 0; x < mosaic.width(); ++x)
        {
            uint16_t idx = mosaic.map(x, y);
            if (idx < 10) Serial.print(' ');
            Serial.print(idx);
            Serial.print(' ');
        }
        Serial.println();
    }
    Serial.println();

    // Test topology hints
    auto h0 = mosaic.topologyHint(0, 0);
    auto h1 = mosaic.topologyHint(1, 1);
    auto h2 = mosaic.topologyHint(3, 3);
    auto h3 = mosaic.topologyHint(-1, 0);

    Serial.print("Hint(0,0) = ");
    Serial.println(static_cast<int>(h0));
    Serial.print("Hint(1,1) = ");
    Serial.println(static_cast<int>(h1));
    Serial.print("Hint(3,3) = ");
    Serial.println(static_cast<int>(h2));
    Serial.print("Hint(-1,0) = ");
    Serial.println(static_cast<int>(h3));
    Serial.println();
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    Serial.println("===== Topology Smoke Test =====\n");

    // Print grids for all 4 base layouts (4x4)
    printGrid("RowMajor",               npb::PanelLayout::RowMajor, 4, 4);
    printGrid("RowMajorAlternating",     npb::PanelLayout::RowMajorAlternating, 4, 4);
    printGrid("ColumnMajor",             npb::PanelLayout::ColumnMajor, 4, 4);
    printGrid("ColumnMajorAlternating",  npb::PanelLayout::ColumnMajorAlternating, 4, 4);

    // Rotated variant
    printGrid("RowMajor90", npb::PanelLayout::RowMajor90, 4, 4);

    testPanelBounds();
    testTiledTopology();
    testMosaicRotation();

    Serial.println("===== Topology Smoke Test Complete =====");
}

void loop()
{
    delay(10000);
}
