#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>

#include "buses/MosaicBusSettings.h"
#include "buses/Topology.h"

namespace lw
{

    struct TiledTopologySettings
    {
        uint16_t panelWidth;
        uint16_t panelHeight;
        uint16_t tilesWide;
        uint16_t tilesHigh;
        PanelLayout panelLayout;
        PanelLayout tileLayout;
        bool mosaicRotation = false;
    };

    class TiledTopology
    {
    public:
        enum class TopologyHint
        {
            FirstOnPanel,
            InPanel,
            LastOnPanel,
            OutOfBounds
        };

        constexpr explicit TiledTopology(const TiledTopologySettings& config)
            : _config(config)
            , _settings{config.panelWidth,
                        config.panelHeight,
                        config.panelLayout,
                        config.tilesWide,
                        config.tilesHigh,
                        config.tileLayout,
                        config.mosaicRotation}
            , _topology(_settings)
        {
        }

        constexpr uint16_t map(int16_t x, int16_t y) const
        {
            if (width() == 0 || height() == 0)
            {
                return 0;
            }

            auto cx = static_cast<int16_t>(std::clamp<int32_t>(x, 0, static_cast<int32_t>(width()) - 1));
            auto cy = static_cast<int16_t>(std::clamp<int32_t>(y, 0, static_cast<int32_t>(height()) - 1));
            return static_cast<uint16_t>(_topology.getIndex(cx, cy));
        }

        constexpr std::optional<uint16_t> mapProbe(int16_t x, int16_t y) const
        {
            size_t index = _topology.getIndex(x, y);
            if (index == Topology::InvalidIndex)
            {
                return std::nullopt;
            }
            return static_cast<uint16_t>(index);
        }

        constexpr TopologyHint topologyHint(int16_t x, int16_t y) const
        {
            size_t index = _topology.getIndex(x, y);
            if (index == Topology::InvalidIndex)
            {
                return TopologyHint::OutOfBounds;
            }

            size_t localIndex = index % _topology.panelPixelCount();
            if (localIndex == 0)
            {
                return TopologyHint::FirstOnPanel;
            }

            if (localIndex + 1 == _topology.panelPixelCount())
            {
                return TopologyHint::LastOnPanel;
            }

            return TopologyHint::InPanel;
        }

        constexpr uint16_t width() const
        {
            return _topology.width();
        }

        constexpr uint16_t height() const
        {
            return _topology.height();
        }

        constexpr uint16_t pixelCount() const
        {
            return static_cast<uint16_t>(_topology.pixelCount());
        }

        constexpr const TiledTopologySettings& config() const
        {
            return _config;
        }

    private:
        TiledTopologySettings _config;
        MosaicBusSettings _settings;
        Topology _topology;
    };

} // namespace lw
