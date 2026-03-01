#pragma once

#include <cstdint>

#include "buses/PanelLayout.h"

namespace lw
{

    struct MosaicBusSettings
    {
        uint16_t panelWidth;
        uint16_t panelHeight;
        PanelLayout layout;
        uint16_t tilesWide;
        uint16_t tilesHigh;
        PanelLayout tileLayout;
        bool mosaicRotation = false;
    };

} // namespace lw
