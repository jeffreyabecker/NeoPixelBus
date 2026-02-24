#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <optional>
#include <algorithm>
#include <tuple>
#include <type_traits>
#include <utility>
#include "../IPixelBus.h"
#include "../ResourceHandle.h"
#include "../topologies/PanelLayout.h"

namespace npb
{

    // -------------------------------------------------------------------
    // MosaicBusConfig — per-panel bus + shared mosaic layout settings
    //
    // For MosaicBus, provide one entry per panel tile. Mixed panel sizes
    // are not supported.
    // -------------------------------------------------------------------
    template <typename TColor = Color>
    struct MosaicBusConfig
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
    //   MosaicBusConfig<> config;
    //   std::vector<ResourceHandle<IPixelBus<>>> buses;
    //   buses.emplace_back(panel0);
    //   buses.emplace_back(panel1);
    //   config.panelWidth = 8;
    //   config.panelHeight = 8;
    //   config.layout = PanelLayout::ColumnMajorAlternating;
    //   config.tilesWide = 2;
    //   config.tilesHigh = 1;
    //   config.tileLayout = PanelLayout::RowMajor;
    //   MosaicBus mosaic(std::move(config), std::move(buses));
    //   mosaic.setPixelColor(12, 5, Color(255, 0, 0));
    // -------------------------------------------------------------------
    template <typename TColor = Color>
    class MosaicBus : public I2dPixelBus<TColor>
    {
    public:
        // Bring base class single-pixel overloads into scope alongside
        // the 2D overloads defined below.
        using IPixelBus<TColor>::setPixelColor;
        using IPixelBus<TColor>::getPixelColor;

        MosaicBus(MosaicBusConfig<TColor> config,
                  std::vector<ResourceHandle<IPixelBus<TColor>>> buses)
            : _config(std::move(config)),
              _buses(std::move(buses))
        {
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
                               [](const ResourceHandle<IPixelBus<TColor>> &bus)
                               { return bus->canShow(); });
        }

        size_t pixelCount() const override
        {
            return _totalPixelCount;
        }

        // --- 2D access (preferred interface) ----------------------------

        void setPixelColor(int16_t x, int16_t y, const TColor &color)
        {
            auto resolved = _resolve2D(x, y);
            if (resolved)
            {
                _buses[resolved->panelIndex]->setPixelColor(
                    resolved->localIndex, color);
            }
        }

        TColor getPixelColor(int16_t x, int16_t y) const
        {
            auto resolved = _resolve2D(x, y);
            if (resolved)
            {
                return _buses[resolved->panelIndex]->getPixelColor(
                    resolved->localIndex);
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
                if (resolved)
                {
                    first[static_cast<std::ptrdiff_t>(i)] =
                        _buses[resolved->panelIndex]->getPixelColor(
                            resolved->localIndex);
                }
            }
        }

    private:
        MosaicBusConfig<TColor> _config;
        std::vector<ResourceHandle<IPixelBus<TColor>>> _buses;
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
                           ColorIteratorT<TColor> first,
                           ColorIteratorT<TColor> last,
                           Fn &&fn)
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
            const size_t panelPixels =
                static_cast<size_t>(_config.panelWidth) * _config.panelHeight;
            if (panelPixels == 0)
            {
                return std::nullopt;
            }

            size_t panelIndex = globalIdx / panelPixels;
            if (panelIndex >= _buses.size())
            {
                return std::nullopt;
            }

            return ResolvedPixel{
                panelIndex,
                static_cast<uint16_t>(globalIdx % panelPixels)};
        }

        // ---------------------------------------------------------------
        // _resolve2D — map global (x, y) → panel + local pixel index
        // ---------------------------------------------------------------
        std::optional<ResolvedPixel> _resolve2D(int16_t x, int16_t y) const
        {
            if (_buses.empty())
                return std::nullopt;

            uint16_t totalW = width();
            uint16_t totalH = height();

            if (x < 0 || x >= static_cast<int16_t>(totalW) ||
                y < 0 || y >= static_cast<int16_t>(totalH))
            {
                return std::nullopt;
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
                return std::nullopt;
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

    template <typename TColor, typename... TBuses>
        requires(std::convertible_to<std::remove_reference_t<TBuses> *, IPixelBus<TColor> *> && ...)
    class OwningMosaicBus : public I2dPixelBus<TColor>
    {
    public:
        static constexpr size_t BusCount = sizeof...(TBuses);

        struct ResolvedPixel
        {
            size_t panelIndex;
            uint16_t localIndex;
        };

        using IPixelBus<TColor>::setPixelColor;
        using IPixelBus<TColor>::getPixelColor;

        OwningMosaicBus(MosaicBusConfig<TColor> config,
                        TBuses &&...buses)
            : _config(std::move(config)),
              _buses(std::forward<TBuses>(buses)...)
        {
            _totalPixelCount = static_cast<size_t>(_config.panelWidth) *
                               _config.panelHeight *
                               BusCount;
        }

        template <int16_t X,
                  int16_t Y,
                  uint16_t PanelWidth,
                  uint16_t PanelHeight,
                  PanelLayout Layout,
                  uint16_t TilesWide,
                  uint16_t TilesHigh,
                  PanelLayout TileLayout,
                  bool MosaicRotation = false>
        static consteval std::optional<ResolvedPixel> resolve2DStatic()
        {
            constexpr uint16_t totalW = static_cast<uint16_t>(PanelWidth * TilesWide);
            constexpr uint16_t totalH = static_cast<uint16_t>(PanelHeight * TilesHigh);

            if constexpr (X < 0 || Y < 0 || X >= static_cast<int16_t>(totalW) || Y >= static_cast<int16_t>(totalH))
            {
                return std::nullopt;
            }

            constexpr uint16_t tileX = static_cast<uint16_t>(X) / PanelWidth;
            constexpr uint16_t localX = static_cast<uint16_t>(X) % PanelWidth;
            constexpr uint16_t tileY = static_cast<uint16_t>(Y) / PanelHeight;
            constexpr uint16_t localY = static_cast<uint16_t>(Y) % PanelHeight;

            constexpr uint16_t tileIndex = mapLayout(TileLayout,
                                                     TilesWide,
                                                     TilesHigh,
                                                     tileX, tileY);

            PanelLayout effectiveLayout = Layout;
            if constexpr (MosaicRotation)
            {
                effectiveLayout = tilePreferredLayout(
                    Layout,
                    (tileY & 1) != 0,
                    (tileX & 1) != 0);
            }

            constexpr uint16_t localIndex = mapLayout(effectiveLayout,
                                                      PanelWidth,
                                                      PanelHeight,
                                                      localX, localY);

            return ResolvedPixel{
                static_cast<size_t>(tileIndex),
                localIndex};
        }

        void begin() override
        {
            std::apply(
                [](auto &...bus)
                {
                    (bus.begin(), ...);
                },
                _buses);
        }

        void show() override
        {
            std::apply(
                [](auto &...bus)
                {
                    (bus.show(), ...);
                },
                _buses);
        }

        bool canShow() const override
        {
            return std::apply(
                [](const auto &...bus)
                {
                    return (bus.canShow() && ...);
                },
                _buses);
        }

        size_t pixelCount() const override
        {
            return _totalPixelCount;
        }

        void setPixelColor(int16_t x, int16_t y, const TColor &color) override
        {
            auto resolved = _resolve2D(x, y);
            if (resolved)
            {
                _forBusAtIndex(
                    resolved->panelIndex,
                    [&](auto &bus)
                    {
                        bus.setPixelColor(resolved->localIndex, color);
                    });
            }
        }

        TColor getPixelColor(int16_t x, int16_t y) const override
        {
            auto resolved = _resolve2D(x, y);
            if (resolved)
            {
                TColor result{};
                _forBusAtIndex(
                    resolved->panelIndex,
                    [&](const auto &bus)
                    {
                        result = bus.getPixelColor(resolved->localIndex);
                    });
                return result;
            }

            return TColor{};
        }

        uint16_t width() const override
        {
            if constexpr (BusCount == 0)
            {
                return 0;
            }

            return static_cast<uint16_t>(_config.panelWidth * _config.tilesWide);
        }

        uint16_t height() const override
        {
            if constexpr (BusCount == 0)
            {
                return 0;
            }

            return static_cast<uint16_t>(_config.panelHeight * _config.tilesHigh);
        }

        void setPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) override
        {
            auto count = static_cast<size_t>(last - first);

            for (size_t i = 0; i < count; ++i)
            {
                auto resolved = _resolveLinear(offset + i);
                if (resolved)
                {
                    _forBusAtIndex(
                        resolved->panelIndex,
                        [&](auto &bus)
                        {
                            bus.setPixelColor(
                                resolved->localIndex,
                                first[static_cast<std::ptrdiff_t>(i)]);
                        });
                }
            }
        }

        void getPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) const override
        {
            auto count = static_cast<size_t>(last - first);

            for (size_t i = 0; i < count; ++i)
            {
                auto resolved = _resolveLinear(offset + i);
                if (resolved)
                {
                    _forBusAtIndex(
                        resolved->panelIndex,
                        [&](const auto &bus)
                        {
                            first[static_cast<std::ptrdiff_t>(i)] =
                                bus.getPixelColor(resolved->localIndex);
                        });
                }
            }
        }

    private:
        std::optional<ResolvedPixel> _resolveLinear(size_t globalIdx) const
        {
            const size_t panelPixels =
                static_cast<size_t>(_config.panelWidth) * _config.panelHeight;
            if (panelPixels == 0)
            {
                return std::nullopt;
            }

            size_t panelIndex = globalIdx / panelPixels;
            if (panelIndex >= BusCount)
            {
                return std::nullopt;
            }

            return ResolvedPixel{
                panelIndex,
                static_cast<uint16_t>(globalIdx % panelPixels)};
        }

        std::optional<ResolvedPixel> _resolve2D(int16_t x, int16_t y) const
        {
            if constexpr (BusCount == 0)
            {
                return std::nullopt;
            }

            uint16_t totalW = width();
            uint16_t totalH = height();

            if (x < 0 || x >= static_cast<int16_t>(totalW) ||
                y < 0 || y >= static_cast<int16_t>(totalH))
            {
                return std::nullopt;
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

            if (tileIndex >= BusCount)
            {
                return std::nullopt;
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

        template <size_t TIndex = 0, typename TFn>
        void _forBusAtIndex(size_t busIndex,
                            TFn &&fn)
        {
            if constexpr (TIndex < BusCount)
            {
                if (busIndex == TIndex)
                {
                    fn(std::get<TIndex>(_buses));
                }
                else
                {
                    _forBusAtIndex<TIndex + 1>(busIndex, std::forward<TFn>(fn));
                }
            }
        }

        template <size_t TIndex = 0, typename TFn>
        void _forBusAtIndex(size_t busIndex,
                            TFn &&fn) const
        {
            if constexpr (TIndex < BusCount)
            {
                if (busIndex == TIndex)
                {
                    fn(std::get<TIndex>(_buses));
                }
                else
                {
                    _forBusAtIndex<TIndex + 1>(busIndex, std::forward<TFn>(fn));
                }
            }
        }

        MosaicBusConfig<TColor> _config;
        std::tuple<std::remove_reference_t<TBuses>...> _buses;
        size_t _totalPixelCount{0};
    };

    template <typename TColor, typename... TBuses>
        requires(std::convertible_to<std::remove_reference_t<TBuses> *, IPixelBus<TColor> *> && ...)
    auto makeOwningMosaicBus(MosaicBusConfig<TColor> config,
                             TBuses &&...buses)
    {
        return OwningMosaicBus<TColor, TBuses...>(
            std::move(config),
            std::forward<TBuses>(buses)...);
    }

} // namespace npb
