#pragma once

#include <cstdint>
#include <optional>
#include <algorithm>

#include "PanelLayout.h"

namespace npb
{

    // -------------------------------------------------------------------
    // TiledTopologyConfig — configuration for a grid of identical panels
    // -------------------------------------------------------------------
    struct TiledTopologyConfig
    {
        uint16_t panelWidth;         // pixels per panel horizontally
        uint16_t panelHeight;        // pixels per panel vertically
        uint16_t tilesWide;          // number of panels horizontally
        uint16_t tilesHigh;          // number of panels vertically
        PanelLayout panelLayout;     // pixel layout within each panel
        PanelLayout tileLayout;      // layout of panels in the grid
        bool mosaicRotation = false; // if true, auto-rotate panels per
                                     // tile-preference (NeoMosaic behavior)
    };

    // -------------------------------------------------------------------
    // TiledTopology — 2D → 1D mapper for a grid of identical panels
    //
    // Replaces both NeoTiles and NeoMosaic from the original library.
    // Use Case A (single bus, tiled panels): a pure coordinate mapper
    // external to the bus.
    //
    //   TiledTopology mosaic({
    //       .panelWidth = 8, .panelHeight = 8,
    //       .tilesWide = 4, .tilesHigh = 2,
    //       .panelLayout = PanelLayout::ColumnMajorAlternating,
    //       .tileLayout  = PanelLayout::RowMajorAlternating,
    //       .mosaicRotation = true
    //   });
    //
    //   bus.setPixelColor(mosaic.map(x, y), color);
    // -------------------------------------------------------------------
    class TiledTopology
    {
    public:
        // Topology hint — indicates pixel position relative to its panel
        enum class TopologyHint
        {
            FirstOnPanel,
            InPanel,
            LastOnPanel,
            OutOfBounds
        };

        constexpr explicit TiledTopology(const TiledTopologyConfig& config)
            : _config(config)
        {
        }

        // ---------------------------------------------------------------
        // map — global (x, y) → linear strip index (clamped)
        // ---------------------------------------------------------------
        constexpr uint16_t map(int16_t x, int16_t y) const
        {
            uint16_t totalW = width();
            uint16_t totalH = height();

            if (totalW == 0 || totalH == 0 ||
                _config.panelWidth == 0 || _config.panelHeight == 0 ||
                _config.tilesWide == 0 || _config.tilesHigh == 0)
            {
                return 0;
            }

            auto cx = static_cast<uint16_t>(
                std::clamp<int16_t>(x, 0, static_cast<int16_t>(totalW - 1)));
            auto cy = static_cast<uint16_t>(
                std::clamp<int16_t>(y, 0, static_cast<int16_t>(totalH - 1)));

            uint16_t localIndex;
            uint16_t tileOffset;
            _calculate(cx, cy, localIndex, tileOffset);

            return localIndex + tileOffset;
        }

        // ---------------------------------------------------------------
        // mapProbe — bounds-checked mapping (nullopt if out of bounds)
        // ---------------------------------------------------------------
        constexpr std::optional<uint16_t> mapProbe(int16_t x, int16_t y) const
        {
            if (width() == 0 || height() == 0 ||
                _config.panelWidth == 0 || _config.panelHeight == 0 ||
                _config.tilesWide == 0 || _config.tilesHigh == 0)
            {
                return std::nullopt;
            }

            if (x < 0 || x >= static_cast<int16_t>(width()) ||
                y < 0 || y >= static_cast<int16_t>(height()))
            {
                return std::nullopt;
            }

            uint16_t localIndex;
            uint16_t tileOffset;
            _calculate(static_cast<uint16_t>(x), static_cast<uint16_t>(y),
                       localIndex, tileOffset);

            return localIndex + tileOffset;
        }

        // ---------------------------------------------------------------
        // topologyHint — panel boundary information
        // ---------------------------------------------------------------
        constexpr TopologyHint topologyHint(int16_t x, int16_t y) const
        {
            if (width() == 0 || height() == 0 ||
                _config.panelWidth == 0 || _config.panelHeight == 0 ||
                _config.tilesWide == 0 || _config.tilesHigh == 0)
            {
                return TopologyHint::OutOfBounds;
            }

            if (x < 0 || x >= static_cast<int16_t>(width()) ||
                y < 0 || y >= static_cast<int16_t>(height()))
            {
                return TopologyHint::OutOfBounds;
            }

            uint16_t localIndex;
            uint16_t tileOffset;
            _calculate(static_cast<uint16_t>(x), static_cast<uint16_t>(y),
                       localIndex, tileOffset);

            uint16_t panelPixelCount = _config.panelWidth * _config.panelHeight;

            if (localIndex == 0)
            {
                return TopologyHint::FirstOnPanel;
            }
            if (localIndex == panelPixelCount - 1)
            {
                return TopologyHint::LastOnPanel;
            }
            return TopologyHint::InPanel;
        }

        constexpr uint16_t width() const
        {
            return _config.panelWidth * _config.tilesWide;
        }

        constexpr uint16_t height() const
        {
            return _config.panelHeight * _config.tilesHigh;
        }

        constexpr uint16_t pixelCount() const
        {
            return width() * height();
        }

        constexpr const TiledTopologyConfig& config() const
        {
            return _config;
        }

    private:
        TiledTopologyConfig _config;

        // ---------------------------------------------------------------
        // Core coordinate resolution (matches original NeoMosaic logic)
        //
        // 1. Split global (x,y) into tile position + local position
        // 2. Compute tile offset via the tile layout mapper
        // 3. If mosaicRotation: pick the rotation that minimizes wiring
        // 4. Map local position through the (possibly rotated) panel layout
        // ---------------------------------------------------------------
        constexpr void _calculate(uint16_t x, uint16_t y,
                                  uint16_t& localIndex,
                                  uint16_t& tileOffset) const
        {
            if (_config.panelWidth == 0 || _config.panelHeight == 0 ||
                _config.tilesWide == 0 || _config.tilesHigh == 0)
            {
                localIndex = 0;
                tileOffset = 0;
                return;
            }

            uint16_t tileX  = x / _config.panelWidth;
            uint16_t localX = x % _config.panelWidth;
            uint16_t tileY  = y / _config.panelHeight;
            uint16_t localY = y % _config.panelHeight;

            uint16_t panelPixelCount = _config.panelWidth * _config.panelHeight;

            tileOffset = mapLayout(_config.tileLayout,
                                   _config.tilesWide, _config.tilesHigh,
                                   tileX, tileY) * panelPixelCount;

            PanelLayout effectiveLayout = _config.panelLayout;

            if (_config.mosaicRotation)
            {
                effectiveLayout = tilePreferredLayout(
                    _config.panelLayout,
                    (tileY & 1) != 0,
                    (tileX & 1) != 0);
            }

            localIndex = mapLayout(effectiveLayout,
                                   _config.panelWidth, _config.panelHeight,
                                   localX, localY);
        }
    };

} // namespace npb
