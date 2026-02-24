#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>
#include <span>
#include <memory>
#include <utility>
#include <vector>
#include <algorithm>

#include <Arduino.h>

#include "IProtocol.h"
#include "../ResourceHandle.h"
#include "../colors/Color.h"
#include "../transports/ITransport.h"

namespace npb
{

    struct Ws2812xProtocolSettings
    {
        ResourceHandle<ITransport> bus;
        const char *channelOrder = ChannelOrder::GRB;
    };

    template <typename TSelfClockingTransport>
        requires TaggedTransportLike<TSelfClockingTransport, SelfClockingTransportTag>
    struct Ws2812xProtocolSettingsT : Ws2812xProtocolSettings
    {
        template <typename... BusArgs>
        explicit Ws2812xProtocolSettingsT(BusArgs &&...busArgs)
            : Ws2812xProtocolSettings{
                  std::make_unique<TSelfClockingTransport>(std::forward<BusArgs>(busArgs)...)}
        {
        }
    };

    template <typename TColor>
    class Ws2812xProtocol : public IProtocol<TColor>
    {
    public:
        using SettingsType = Ws2812xProtocolSettings;

        static_assert((std::same_as<typename TColor::ComponentType, uint8_t> ||
                       std::same_as<typename TColor::ComponentType, uint16_t>),
                      "Ws2812xProtocol supports uint8_t or uint16_t color components.");
        static_assert(TColor::ChannelCount >= 3 && TColor::ChannelCount <= 5,
                      "Ws2812xProtocol expects 3 to 5 color channels.");

        Ws2812xProtocol(uint16_t pixelCount,
                        Ws2812xProtocolSettings settings)
            : _settings{std::move(settings)},
              _channelOrder{resolveChannelOrder(_settings.channelOrder)},
              _channelCount{resolveChannelCount(_channelOrder)},
              _pixelCount{pixelCount},
              _sizeData{bytesNeeded(pixelCount, _channelCount)},
              _data(static_cast<uint8_t *>(malloc(_sizeData)))}
        {
            if (_data)
            {
                std::memset(_data, 0, _sizeData);
            }
        }

        Ws2812xProtocol(uint16_t pixelCount,
                        const char *channelOrder,
                        ResourceHandle<ITransport> transport)
            : Ws2812xProtocol{pixelCount,
                              Ws2812xProtocolSettings{std::move(transport), channelOrder}}
        {
        }

        ~Ws2812xProtocol() override
        {
            free(_data);
        }

        Ws2812xProtocol(const Ws2812xProtocol &) = delete;
        Ws2812xProtocol &operator=(const Ws2812xProtocol &) = delete;
        Ws2812xProtocol(Ws2812xProtocol &&) = delete;
        Ws2812xProtocol &operator=(Ws2812xProtocol &&) = delete;

        void initialize() override
        {
            _settings.bus->begin();
        }

        void update(std::span<const TColor> colors) override
        {
            while (!isReadyToUpdate())
            {
                yield();
            }

            serialize(std::span<uint8_t>{_data, _sizeData}, colors);
            _settings.bus->transmitBytes(std::span<const uint8_t>{_data, _sizeData});
        }

        bool isReadyToUpdate() const override
        {
            return _settings.bus->isReadyToUpdate();
        }

        bool alwaysUpdate() const override
        {
            return false;
        }

    protected:
        size_t frameSize() const
        {
            return _sizeData;
        }

    private:
        static constexpr const char *resolveChannelOrder(const char *channelOrder)
        {
            return (nullptr != channelOrder) ? channelOrder : ChannelOrder::GRB;
        }

        static constexpr size_t resolveChannelCount(const char *channelOrder)
        {
            const size_t requestedCount = std::char_traits<char>::length(channelOrder);
            if (requestedCount == 0)
            {
                return ChannelOrder::LengthGRB;
            }

            return std::min(requestedCount, TColor::ChannelCount);
        }

        static constexpr size_t bytesNeeded(size_t pixelCount, size_t channelCount)
        {
            return pixelCount * channelCount;
        }

        static constexpr uint8_t toWireComponent(typename TColor::ComponentType value)
        {
            if constexpr (std::same_as<typename TColor::ComponentType, uint8_t>)
            {
                return value;
            }

            return static_cast<uint8_t>(value >> 8);
        }

        void serialize(std::span<uint8_t> pixels,
                       std::span<const TColor> colors)
        {
            size_t offset = 0;

            for (const auto &color : colors)
            {
                for (size_t channel = 0; channel < _channelCount; ++channel)
                {
                    pixels[offset++] = toWireComponent(color[_channelOrder[channel]]);
                }
            }
        }

        const char *_channelOrder;
        size_t _channelCount;
        uint16_t _pixelCount;
        size_t _sizeData;

        uint8_t *_data{nullptr};
        Ws2812xProtocolSettings _settings;
    };

} // namespace npb
