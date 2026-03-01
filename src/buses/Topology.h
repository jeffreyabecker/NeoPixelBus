#pragma once

#include <cstddef>
#include <cstdint>

#include "buses/MosaicBusSettings.h"

namespace lw
{

    class Topology
    {
    public:
        static constexpr size_t InvalidIndex = static_cast<size_t>(-1);

        constexpr explicit Topology(MosaicBusSettings config)
            : _config(config)
        {
        }

        constexpr uint16_t width() const
        {
            return _config.panelWidth * _config.tilesWide;
        }

        constexpr uint16_t height() const
        {
            return _config.panelHeight * _config.tilesHigh;
        }

        constexpr size_t pixelCount() const
        {
            return static_cast<size_t>(width()) * height();
        }

        constexpr bool isInBounds(int16_t x, int16_t y) const
        {
            return x >= 0 &&
                   y >= 0 &&
                   x < static_cast<int16_t>(width()) &&
                   y < static_cast<int16_t>(height());
        }

        constexpr size_t getIndex(int16_t x, int16_t y) const
        {
            if (!isInBounds(x, y) || _config.panelWidth == 0 || _config.panelHeight == 0)
            {
                return InvalidIndex;
            }

            uint16_t px = static_cast<uint16_t>(x);
            uint16_t py = static_cast<uint16_t>(y);

            uint16_t tileX = px / _config.panelWidth;
            uint16_t localX = px % _config.panelWidth;
            uint16_t tileY = py / _config.panelHeight;
            uint16_t localY = py % _config.panelHeight;

            uint16_t tileIndex = mapLayout(_config.tileLayout,
                                           _config.tilesWide,
                                           _config.tilesHigh,
                                           tileX,
                                           tileY);

            PanelLayout effectiveLayout = _config.layout;
            if (_config.mosaicRotation)
            {
                effectiveLayout = tilePreferredLayout(_config.layout,
                                                      (tileY & 1) != 0,
                                                      (tileX & 1) != 0);
            }

            uint16_t localIndex = mapLayout(effectiveLayout,
                                            _config.panelWidth,
                                            _config.panelHeight,
                                            localX,
                                            localY);

            size_t panelPixels = static_cast<size_t>(_config.panelWidth) * _config.panelHeight;
            return static_cast<size_t>(tileIndex) * panelPixels + localIndex;
        }

        constexpr size_t panelPixelCount() const
        {
            return static_cast<size_t>(_config.panelWidth) * _config.panelHeight;
        }

        constexpr const MosaicBusSettings& settings() const
        {
            return _config;
        }

    private:
        MosaicBusSettings _config;
    };

} // namespace lw
