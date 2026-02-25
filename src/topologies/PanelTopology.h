#pragma once

#include <cstdint>
#include <optional>
#include <algorithm>

#include "PanelLayout.h"

namespace npb
{

    // -------------------------------------------------------------------
    // PanelTopology ? runtime 2D ? 1D mapper for a single panel
    //
    // Replaces the original templated NeoTopology<T_LAYOUT>.
    // The layout is selected at runtime via a PanelLayout enum value.
    //
    //   PanelTopology topo(8, 8, PanelLayout::RowMajorAlternating);
    //   uint16_t idx = topo.map(x, y);       // clamped
    //   auto opt     = topo.mapProbe(x, y);  // bounds-checked
    // -------------------------------------------------------------------
    class PanelTopology
    {
    public:
        constexpr PanelTopology(uint16_t width, uint16_t height,
                                PanelLayout layout)
            : _width(width), _height(height), _layout(layout)
        {
        }

        // ---------------------------------------------------------------
        // map ? coordinate mapping with clamping
        //
        // Out-of-bounds coordinates are clamped to the nearest edge
        // (matching original NeoTopology::Map behavior).
        // ---------------------------------------------------------------
        constexpr uint16_t map(int16_t x, int16_t y) const
        {
            auto cx = static_cast<uint16_t>(
                std::clamp<int16_t>(x, 0, static_cast<int16_t>(_width - 1)));
            auto cy = static_cast<uint16_t>(
                std::clamp<int16_t>(y, 0, static_cast<int16_t>(_height - 1)));
            return mapLayout(_layout, _width, _height, cx, cy);
        }

        // ---------------------------------------------------------------
        // mapProbe ? bounds-checked mapping
        //
        // Returns std::nullopt if (x, y) is outside the panel.
        // Replaces original MapProbe which returned width*height as
        // an out-of-bounds sentinel.
        // ---------------------------------------------------------------
        constexpr std::optional<uint16_t> mapProbe(int16_t x, int16_t y) const
        {
            if (x < 0 || x >= static_cast<int16_t>(_width) ||
                y < 0 || y >= static_cast<int16_t>(_height))
            {
                return std::nullopt;
            }
            return mapLayout(_layout, _width, _height,
                             static_cast<uint16_t>(x),
                             static_cast<uint16_t>(y));
        }

        constexpr uint16_t width() const { return _width; }
        constexpr uint16_t height() const { return _height; }
        constexpr uint16_t pixelCount() const { return _width * _height; }
        constexpr PanelLayout layout() const { return _layout; }

    private:
        uint16_t _width;
        uint16_t _height;
        PanelLayout _layout;
    };

} // namespace npb

