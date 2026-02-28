#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "buses/MosaicBusSettings.h"
#include "buses/Topology.h"
#include "core/BufferHolder.h"
#include "core/IPixelBus.h"

namespace npb
{

    template <typename TColor, typename... TBuses>
    static constexpr bool MosaicBusCompatibleBuses =
        (std::is_convertible<std::remove_reference_t<TBuses> *, IAssignableBufferBus<TColor> *>::value && ...);

    template <typename TColor>
    class MosaicBus : public I2dPixelBus<TColor>
    {
    public:
        MosaicBus(MosaicBusSettings config,
                  std::vector<IAssignableBufferBus<TColor> *> buses,
                  BufferHolder<TColor> colors)
            : _config(std::move(config))
            , _topology(_config)
            , _buses(std::move(buses))
            , _colors(std::move(colors))
        {
            _buses.erase(std::remove(_buses.begin(), _buses.end(), nullptr), _buses.end());
            if (std::any_of(_buses.begin(), _buses.end(), [](const auto* bus) { return bus == nullptr; }))
            {
                throw std::invalid_argument("MosaicBus: null bus pointer provided");
            }

        }

        void begin() override
        {
            if (_colors.size == 0)
            {
                _colors = BufferHolder<TColor>{_topology.pixelCount(), nullptr, true};
            }            
            _colors.init();
            uint16_t panelPixelCount = _topology.panelPixelCount();
            size_t offset = 0;
            for (auto* bus : _buses)
            {
                bus->setBuffer(_colors.getSpan(offset, panelPixelCount));
                offset += panelPixelCount;
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
                               [](const IAssignableBufferBus<TColor>* bus)
                               {
                                   return bus != nullptr && bus->canShow();
                               });
        }

        span<TColor> pixelBuffer() override
        {
            return _colors.getSpan(0, _colors.size);
        }

        span<const TColor> pixelBuffer() const override
        {
            return _colors.getSpan(0, _colors.size);
        }

        const Topology& topology() const override
        {
            return _topology;
        }

        size_t pixelCount() const
        {
            return _colors.size;
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
        std::vector<IAssignableBufferBus<TColor> *> _buses;
        BufferHolder<TColor> _colors;
    };

} // namespace npb
