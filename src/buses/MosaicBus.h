#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "buses/MosaicBusSettings.h"
#include "buses/Topology.h"
#include "core/IPixelBus.h"

namespace npb
{

    template <typename TColor, typename... TBuses>
    static constexpr bool MosaicBusCompatibleBuses =
        (std::is_convertible<std::remove_reference_t<TBuses> *, IPixelBus<TColor> *>::value && ...);

    template <typename TColor>
    class MosaicBus : public I2dPixelBus<TColor>
    {
    public:
        MosaicBus(MosaicBusSettings config,
                  std::vector<IPixelBus<TColor> *> buses,
                  std::shared_ptr<std::vector<TColor>> ownedColors)
            : _config(std::move(config))
            , _topology(_config)
            , _buses(std::move(buses))
            , _colors(std::move(ownedColors))
        {
            _buses.erase(std::remove(_buses.begin(), _buses.end(), nullptr), _buses.end());

            if (_colors == nullptr)
            {
                _colors = std::make_shared<std::vector<TColor>>(_topology.pixelCount());
            }

            if (_colors->size() != _topology.pixelCount())
            {
                _colors->resize(_topology.pixelCount());
            }
        }

        void begin() override
        {
            for (auto* bus : _buses)
            {
                bus->begin();
            }
        }

        void show() override
        {
            for (auto* bus : _buses)
            {
                bus->show();
            }
        }

        bool canShow() const override
        {
            return std::all_of(_buses.begin(),
                               _buses.end(),
                               [](const IPixelBus<TColor>* bus)
                               {
                                   return bus != nullptr && bus->canShow();
                               });
        }

        span<TColor> pixelBuffer() override
        {
            return span<TColor>{_colors->data(), _colors->size()};
        }

        span<const TColor> pixelBuffer() const override
        {
            return span<const TColor>{_colors->data(), _colors->size()};
        }

        const Topology& topology() const override
        {
            return _topology;
        }

        size_t pixelCount() const
        {
            return _colors->size();
        }

        uint16_t width() const
        {
            return _topology.width();
        }

        uint16_t height() const
        {
            return _topology.height();
        }

    private:
        MosaicBusSettings _config;
        Topology _topology;
        std::vector<IPixelBus<TColor> *> _buses;
        std::shared_ptr<std::vector<TColor>> _colors;
    };

} // namespace npb
