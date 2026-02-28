#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <tuple>
#include <type_traits>
#include <utility>
#include "core/IPixelBus.h"
#include "topologies/PanelLayout.h"

namespace npb
{

    template <typename TColor, typename... TBuses>
    static constexpr bool MosaicBusCompatibleBuses =
        (std::is_convertible<std::remove_reference_t<TBuses> *, IPixelBus<TColor> *>::value && ...);

    // -------------------------------------------------------------------
    // MosaicBusSettings ? per-panel bus + shared mosaic layout settings
    //
    // For MosaicBus, provide one entry per panel tile. Mixed panel sizes
    // are not supported.
    // -------------------------------------------------------------------
    template <typename TColor>
    struct MosaicBusSettings
    {
        uint16_t panelWidth;         // pixels wide on each panel
        uint16_t panelHeight;        // pixels tall on each panel
        PanelLayout layout;          // pixel layout within an individual panel
        uint16_t tilesWide;          // grid columns
        uint16_t tilesHigh;          // grid rows
        PanelLayout tileLayout;      // how panels are arranged in the grid
        bool mosaicRotation = false; // auto-rotate panels per tile-preference
    };

    // -------------------------------------------------------------------
    // MosaicBus ? 2D multi-bus mosaic implementing IPixelBus
    //
    // Manages multiple child buses arranged in a 2D grid.  Each child bus
    // corresponds to one panel/tile in the mosaic.  All panels are
    // assumed to share the same dimensions.
    //
    // For simple 1D strip concatenation (possibly uneven lengths) use
    // ConcatBus instead.
    //
    // Usage (borrowing):
    //   MosaicBusSettings<> config;
    //   std::vector<IPixelBus<*>*> buses;
    //   buses.emplace_back(&panel0);
    //   buses.emplace_back(&panel1);
    //   config.panelWidth = 8;
    //   config.panelHeight = 8;
    //   config.layout = PanelLayout::ColumnMajorAlternating;
    //   config.tilesWide = 2;
    //   config.tilesHigh = 1;
    //   config.tileLayout = PanelLayout::RowMajor;
    //   MosaicBus mosaic(std::move(config), std::move(buses));
    //   mosaic.setPixelColor(12, 5, Color(255, 0, 0));
    // -------------------------------------------------------------------
    template <typename TColor>
    class MosaicBus : public I2dPixelBus<TColor>
    {
    public:
        // Bring base class single-pixel overloads into scope alongside
        // the 2D overloads defined below.
        using IPixelBus<TColor>::setPixelColor;
        using IPixelBus<TColor>::getPixelColor;

        MosaicBus(MosaicBusSettings<TColor> config,
                std::vector<IPixelBus<TColor> *> buses)
            : _config(std::move(config)),
              _buses(std::move(buses))
        {
            _buses.erase(std::remove(_buses.begin(), _buses.end(), nullptr), _buses.end());
            _totalPixelCount = static_cast<size_t>(_config.panelWidth) *
                               _config.panelHeight *
                               _buses.size();
        }

        // --- IPixelBus lifecycle ----------------------------------------

        void begin() override
        {
            for (auto &bus : _buses)
            {
                bus->begin();
            }
        }

        void show() override
        {
            for (auto &bus : _buses)
            {
                bus->show();
            }
        }

        bool canShow() const override
        {
            return std::all_of(_buses.begin(), _buses.end(),
                               [](const IPixelBus<TColor> *bus)
                               { return bus != nullptr && bus->canShow(); });
        }

        size_t pixelCount() const override
        {
            return _totalPixelCount;
        }

        // --- 2D access (preferred interface) ----------------------------

        void setPixelColor(int16_t x, int16_t y, const TColor &color)
        {
            auto resolved = _resolve2D(x, y);
            if (resolved.isValid())
            {
                _buses[resolved.panelIndex]->setPixelColor(
                    resolved.localIndex, color);
            }
        }

        TColor getPixelColor(int16_t x, int16_t y) const
        {
            auto resolved = _resolve2D(x, y);
            if (resolved.isValid())
            {
                return _buses[resolved.panelIndex]->getPixelColor(
                    resolved.localIndex);
            }
            return TColor{};
        }

        uint16_t width() const
        {
            if (_buses.empty())
                return 0;
            return _config.panelWidth * _config.tilesWide;
        }

        uint16_t height() const
        {
            if (_buses.empty())
                return 0;
            return _config.panelHeight * _config.tilesHigh;
        }

        // --- Primary IPixelBus interface (iterator pair) ----------------
        // Linearizes the 2D mosaic into a flat sequence: panel 0 pixels
        // first, then panel 1, etc. (row-major tile order via tileLayout).
        // Each panel's pixels are linearized by its own panel layout.

        void setPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) override
        {
            _forEachPixel(offset, first, last,
                          [this](size_t panelIdx, uint16_t localIdx,
                                 ColorIteratorT<TColor> &it, std::ptrdiff_t i)
                          {
                              _buses[panelIdx]->setPixelColor(localIdx, it[i]);
                          });
        }

        void getPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) const override
        {
            auto count = static_cast<size_t>(last - first);
            for (size_t i = 0; i < count; ++i)
            {
                size_t globalIdx = offset + i;
                if (globalIdx >= _totalPixelCount)
                    break;

                auto resolved = _resolveLinear(globalIdx);
                if (resolved.isValid())
                {
                    first[static_cast<std::ptrdiff_t>(i)] =
                        _buses[resolved.panelIndex]->getPixelColor(
                            resolved.localIndex);
                }
            }
        }

    private:
        MosaicBusSettings<TColor> _config;
        std::vector<IPixelBus<TColor> *> _buses;
        size_t _totalPixelCount{0};

        struct ResolvedPixel
        {
            size_t panelIndex;
            uint16_t localIndex;

            static constexpr size_t InvalidPanelIndex = static_cast<size_t>(-1);
            static constexpr uint16_t InvalidLocalIndex = static_cast<uint16_t>(-1);

            bool isValid() const
            {
                return panelIndex != InvalidPanelIndex;
            }
        };

        // ---------------------------------------------------------------
        // _forEachPixel ? iterate a linear range, resolving each global
        // index to a panel + local index.
        // ---------------------------------------------------------------
        template <typename Fn>
        void _forEachPixel(size_t offset,
                           ColorIteratorT<TColor> first,
                           ColorIteratorT<TColor> last,
                           Fn &&fn)
        {
            auto count = static_cast<size_t>(last - first);
            for (size_t i = 0; i < count; ++i)
            {
                auto resolved = _resolveLinear(offset + i);
                if (resolved.isValid())
                {
                    fn(resolved.panelIndex, resolved.localIndex,
                       first, static_cast<std::ptrdiff_t>(i));
                }
            }
        }

        // ---------------------------------------------------------------
        // _resolveLinear ? map flat linear index ? panel + local pixel
        //
        // Walk panels sequentially (panels are few; O(N) is fine).
        // ---------------------------------------------------------------
        ResolvedPixel _resolveLinear(size_t globalIdx) const
        {
            const size_t panelPixels =
                static_cast<size_t>(_config.panelWidth) * _config.panelHeight;
            if (panelPixels == 0)
            {
                return ResolvedPixel{ResolvedPixel::InvalidPanelIndex, ResolvedPixel::InvalidLocalIndex};
            }

            size_t panelIndex = globalIdx / panelPixels;
            if (panelIndex >= _buses.size())
            {
                return ResolvedPixel{ResolvedPixel::InvalidPanelIndex, ResolvedPixel::InvalidLocalIndex};
            }

            return ResolvedPixel{
                panelIndex,
                static_cast<uint16_t>(globalIdx % panelPixels)};
        }

        // ---------------------------------------------------------------
        // _resolve2D ? map global (x, y) ? panel + local pixel index
        // ---------------------------------------------------------------
        ResolvedPixel _resolve2D(int16_t x, int16_t y) const
        {
            if (_buses.empty())
            return ResolvedPixel{ResolvedPixel::InvalidPanelIndex, ResolvedPixel::InvalidLocalIndex};

            uint16_t totalW = width();
            uint16_t totalH = height();

            if (x < 0 || x >= static_cast<int16_t>(totalW) ||
                y < 0 || y >= static_cast<int16_t>(totalH))
            {
                return ResolvedPixel{ResolvedPixel::InvalidPanelIndex, ResolvedPixel::InvalidLocalIndex};
            }

            uint16_t pw = _config.panelWidth;
            uint16_t ph = _config.panelHeight;

            uint16_t tileX = static_cast<uint16_t>(x) / pw;
            uint16_t localX = static_cast<uint16_t>(x) % pw;
            uint16_t tileY = static_cast<uint16_t>(y) / ph;
            uint16_t localY = static_cast<uint16_t>(y) % ph;

            uint16_t tileIndex = mapLayout(_config.tileLayout,
                                           _config.tilesWide,
                                           _config.tilesHigh,
                                           tileX, tileY);

            if (tileIndex >= _buses.size())
            {
                return ResolvedPixel{ResolvedPixel::InvalidPanelIndex, ResolvedPixel::InvalidLocalIndex};
            }

            PanelLayout effectiveLayout = _config.layout;
            if (_config.mosaicRotation)
            {
                effectiveLayout = tilePreferredLayout(
                    _config.layout,
                    (tileY & 1) != 0,
                    (tileX & 1) != 0);
            }

            uint16_t localIndex = mapLayout(effectiveLayout,
                                            _config.panelWidth,
                                            _config.panelHeight,
                                            localX, localY);

            return ResolvedPixel{tileIndex, localIndex};
        }
    };

} // namespace npb


