#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <optional>
#include <algorithm>
#include "IPixelBus.h"
#include "ResourceHandle.h"
#include "topologies/PanelLayout.h"

namespace npb
{

    // -------------------------------------------------------------------
    // MosaicPanel — describes one panel in a multi-bus mosaic
    //
    // The bus is held via ResourceHandle: pass a unique_ptr to transfer
    // ownership, or pass a reference to borrow.
    // -------------------------------------------------------------------
    struct MosaicPanel
    {
        ResourceHandle<IPixelBus> bus;   // owning or borrowing
        uint16_t panelWidth;             // pixels wide on this panel
        uint16_t panelHeight;            // pixels tall on this panel
        PanelLayout layout;              // pixel layout within this panel
    };

    // -------------------------------------------------------------------
    // MosaicBusConfig — grid arrangement of panels
    // -------------------------------------------------------------------
    struct MosaicBusConfig
    {
        uint16_t tilesWide;          // grid columns
        uint16_t tilesHigh;          // grid rows
        PanelLayout tileLayout;      // how panels are arranged in the grid
        bool mosaicRotation = false; // auto-rotate panels per tile-preference
    };

    // -------------------------------------------------------------------
    // MosaicBus — 2D multi-bus mosaic implementing IPixelBus
    //
    // Manages multiple child buses arranged in a 2D grid.  Each child bus
    // corresponds to one panel/tile in the mosaic.  All panels are
    // assumed to share the same dimensions.
    //
    // For simple 1D strip concatenation (possibly uneven lengths) use
    // ConcatBus instead.
    //
    // Usage (borrowing):
    //   std::vector<MosaicPanel> panels;
    //   panels.push_back({panel0, 8, 8, PanelLayout::ColumnMajorAlternating});
    //   panels.push_back({panel1, 8, 8, PanelLayout::ColumnMajorAlternating});
    //   MosaicBus mosaic(std::move(panels), {.tilesWide=2, .tilesHigh=1,
    //                                        .tileLayout=PanelLayout::RowMajor});
    //   mosaic.setPixelColor(12, 5, Color(255, 0, 0));
    // -------------------------------------------------------------------
    class MosaicBus : public IPixelBus
    {
    public:
        // Bring base class single-pixel overloads into scope alongside
        // the 2D overloads defined below.
        using IPixelBus::setPixelColor;
        using IPixelBus::getPixelColor;

        MosaicBus(std::vector<MosaicPanel> panels,
                  MosaicBusConfig config)
            : _panels(std::move(panels)),
              _config(config)
        {
            _totalPixelCount = 0;
            for (const auto& p : _panels)
            {
                _totalPixelCount += static_cast<size_t>(p.panelWidth) * p.panelHeight;
            }
        }

        // --- IPixelBus lifecycle ----------------------------------------

        void begin() override
        {
            for (auto& panel : _panels)
            {
                panel.bus->begin();
            }
        }

        void show() override
        {
            for (auto& panel : _panels)
            {
                panel.bus->show();
            }
        }

        bool canShow() const override
        {
            return std::all_of(_panels.begin(), _panels.end(),
                [](const MosaicPanel& p) { return p.bus->canShow(); });
        }

        size_t pixelCount() const override
        {
            return _totalPixelCount;
        }

        // --- 2D access (preferred interface) ----------------------------

        void setPixelColor(int16_t x, int16_t y, const Color& color)
        {
            auto resolved = _resolve2D(x, y);
            if (resolved)
            {
                _panels[resolved->panelIndex].bus->setPixelColor(
                    resolved->localIndex, color);
            }
        }

        Color getPixelColor(int16_t x, int16_t y) const
        {
            auto resolved = _resolve2D(x, y);
            if (resolved)
            {
                return _panels[resolved->panelIndex].bus->getPixelColor(
                    resolved->localIndex);
            }
            return Color{};
        }

        uint16_t width() const
        {
            if (_panels.empty()) return 0;
            return _panels[0].panelWidth * _config.tilesWide;
        }

        uint16_t height() const
        {
            if (_panels.empty()) return 0;
            return _panels[0].panelHeight * _config.tilesHigh;
        }

        // --- Primary IPixelBus interface (iterator pair) ----------------
        // Linearizes the 2D mosaic into a flat sequence: panel 0 pixels
        // first, then panel 1, etc. (row-major tile order via tileLayout).
        // Each panel's pixels are linearized by its own panel layout.

        void setPixelColors(size_t offset,
                            ColorIterator first,
                            ColorIterator last) override
        {
            _forEachPixel(offset, first, last,
                [this](size_t panelIdx, uint16_t localIdx,
                       ColorIterator& it, std::ptrdiff_t i)
                {
                    _panels[panelIdx].bus->setPixelColor(localIdx, it[i]);
                });
        }

        void getPixelColors(size_t offset,
                            ColorIterator first,
                            ColorIterator last) const override
        {
            auto count = static_cast<size_t>(last - first);
            for (size_t i = 0; i < count; ++i)
            {
                size_t globalIdx = offset + i;
                if (globalIdx >= _totalPixelCount) break;

                auto resolved = _resolveLinear(globalIdx);
                if (resolved)
                {
                    first[static_cast<std::ptrdiff_t>(i)] =
                        _panels[resolved->panelIndex].bus->getPixelColor(
                            resolved->localIndex);
                }
            }
        }

    private:
        std::vector<MosaicPanel> _panels;
        MosaicBusConfig _config;
        size_t _totalPixelCount{0};

        struct ResolvedPixel
        {
            size_t panelIndex;
            uint16_t localIndex;
        };

        // ---------------------------------------------------------------
        // _forEachPixel — iterate a linear range, resolving each global
        // index to a panel + local index.
        // ---------------------------------------------------------------
        template <typename Fn>
        void _forEachPixel(size_t offset,
                           ColorIterator first, ColorIterator last,
                           Fn&& fn)
        {
            auto count = static_cast<size_t>(last - first);
            for (size_t i = 0; i < count; ++i)
            {
                auto resolved = _resolveLinear(offset + i);
                if (resolved)
                {
                    fn(resolved->panelIndex, resolved->localIndex,
                       first, static_cast<std::ptrdiff_t>(i));
                }
            }
        }

        // ---------------------------------------------------------------
        // _resolveLinear — map flat linear index → panel + local pixel
        //
        // Walk panels sequentially (panels are few; O(N) is fine).
        // ---------------------------------------------------------------
        std::optional<ResolvedPixel> _resolveLinear(size_t globalIdx) const
        {
            size_t running = 0;
            for (size_t i = 0; i < _panels.size(); ++i)
            {
                size_t panelPixels = static_cast<size_t>(
                    _panels[i].panelWidth) * _panels[i].panelHeight;
                if (globalIdx < running + panelPixels)
                {
                    return ResolvedPixel{i,
                        static_cast<uint16_t>(globalIdx - running)};
                }
                running += panelPixels;
            }
            return std::nullopt;
        }

        // ---------------------------------------------------------------
        // _resolve2D — map global (x, y) → panel + local pixel index
        // ---------------------------------------------------------------
        std::optional<ResolvedPixel> _resolve2D(int16_t x, int16_t y) const
        {
            if (_panels.empty()) return std::nullopt;

            uint16_t totalW = width();
            uint16_t totalH = height();

            if (x < 0 || x >= static_cast<int16_t>(totalW) ||
                y < 0 || y >= static_cast<int16_t>(totalH))
            {
                return std::nullopt;
            }

            uint16_t pw = _panels[0].panelWidth;
            uint16_t ph = _panels[0].panelHeight;

            uint16_t tileX  = static_cast<uint16_t>(x) / pw;
            uint16_t localX = static_cast<uint16_t>(x) % pw;
            uint16_t tileY  = static_cast<uint16_t>(y) / ph;
            uint16_t localY = static_cast<uint16_t>(y) % ph;

            uint16_t tileIndex = mapLayout(_config.tileLayout,
                                           _config.tilesWide,
                                           _config.tilesHigh,
                                           tileX, tileY);

            if (tileIndex >= _panels.size())
            {
                return std::nullopt;
            }

            const auto& panel = _panels[tileIndex];

            PanelLayout effectiveLayout = panel.layout;
            if (_config.mosaicRotation)
            {
                effectiveLayout = tilePreferredLayout(
                    panel.layout,
                    (tileY & 1) != 0,
                    (tileX & 1) != 0);
            }

            uint16_t localIndex = mapLayout(effectiveLayout,
                                            panel.panelWidth,
                                            panel.panelHeight,
                                            localX, localY);

            return ResolvedPixel{tileIndex, localIndex};
        }
    };

} // namespace npb
