#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>

#include "buses/MosaicBusSettings.h"
#include "buses/Topology.h"

namespace lw
{

    class PanelTopology
    {
    public:
        constexpr PanelTopology(uint16_t width, uint16_t height,
                                PanelLayout layout)
            : _settings{width, height, layout, 1, 1, PanelLayout::RowMajor, false}
            , _topology(_settings)
        {
        }

        constexpr uint16_t map(int16_t x, int16_t y) const
        {
            if (_settings.panelWidth == 0 || _settings.panelHeight == 0)
            {
                return 0;
            }

            auto cx = static_cast<int16_t>(std::clamp<int32_t>(x, 0, static_cast<int32_t>(_settings.panelWidth) - 1));
            auto cy = static_cast<int16_t>(std::clamp<int32_t>(y, 0, static_cast<int32_t>(_settings.panelHeight) - 1));
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

        constexpr uint16_t width() const { return _topology.width(); }
        constexpr uint16_t height() const { return _topology.height(); }
        constexpr uint16_t pixelCount() const { return static_cast<uint16_t>(_topology.pixelCount()); }
        constexpr PanelLayout layout() const { return _settings.layout; }

    private:
        MosaicBusSettings _settings;
        Topology _topology;
    };

} // namespace lw
