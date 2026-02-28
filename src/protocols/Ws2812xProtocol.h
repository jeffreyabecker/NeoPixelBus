#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include <algorithm>

#include <Arduino.h>

#include "IProtocol.h"
#include "colors/Color.h"
#include "transports/ITransport.h"
#include "transports/OneWireTiming.h"

namespace npb
{

    struct Ws2812xProtocolSettings
    {
        ITransport *bus = nullptr;
        const char *channelOrder = ChannelOrder::GRB::value;
        OneWireTiming timing = timing::Ws2812x;
    };

    template <typename TColor>
    class Ws2812xProtocol : public IProtocol<TColor>
    {
    public:
        using SettingsType = Ws2812xProtocolSettings;
        using TransportCategory = OneWireTransportTag;

        static_assert((std::is_same<typename TColor::ComponentType, uint8_t>::value ||
                   std::is_same<typename TColor::ComponentType, uint16_t>::value),
                      "Ws2812xProtocol supports uint8_t or uint16_t color components.");
        static_assert(TColor::ChannelCount >= 3 && TColor::ChannelCount <= 5,
                      "Ws2812xProtocol expects 3 to 5 color channels.");

        Ws2812xProtocol(uint16_t pixelCount,
                SettingsType settings)
                        : IProtocol<TColor>(pixelCount),
                            _settings{std::move(settings)},
              _channelOrder{resolveChannelOrder(_settings.channelOrder)},
              _channelCount{resolveChannelCount(_channelOrder)},
              _sizeData{bytesNeeded(pixelCount, _channelCount)},
              _data(static_cast<uint8_t *>(malloc(_sizeData)))
        {
            if (_data)
            {
                std::memset(_data, 0, _sizeData);
            }
        }

        Ws2812xProtocol(uint16_t pixelCount,
                        const char *channelOrder,
                        ITransport *transport)
            : Ws2812xProtocol{pixelCount,
                              Ws2812xProtocolSettings{transport, channelOrder, timing::Ws2812x}}
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

        void update(span<const TColor> colors) override
        {
            while (!isReadyToUpdate())
            {
                yield();
            }

            serialize(span<uint8_t>{_data, _sizeData}, colors);
            _settings.bus->beginTransaction();
            _settings.bus->transmitBytes(span<uint8_t>{_data, _sizeData});
            _settings.bus->endTransaction();
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
        SettingsType _settings;
        static constexpr const char *resolveChannelOrder(const char *channelOrder)
        {
            return (nullptr != channelOrder) ? channelOrder : ChannelOrder::GRB::value;
        }

        static constexpr size_t resolveChannelCount(const char *channelOrder)
        {
            const size_t requestedCount = std::char_traits<char>::length(channelOrder);
            if (requestedCount == 0)
            {
                return ChannelOrder::GRB::length;
            }

            return std::min(requestedCount, TColor::ChannelCount);
        }

        static constexpr size_t bytesNeeded(size_t pixelCount, size_t channelCount)
        {
            return pixelCount * channelCount * sizeof(typename TColor::ComponentType);
        }

        static constexpr void appendWireComponent(span<uint8_t> pixels,
                                                  size_t &offset,
                                                  typename TColor::ComponentType value)
        {
            if constexpr (std::is_same<typename TColor::ComponentType, uint8_t>::value)
            {
                pixels[offset++] = value;
                return;
            }

            pixels[offset++] = static_cast<uint8_t>(value >> 8);
            pixels[offset++] = static_cast<uint8_t>(value & 0xFF);
        }

        void serialize(span<uint8_t> pixels,
                   span<const TColor> colors)
        {
            size_t offset = 0;
            const size_t pixelLimit = std::min(colors.size(), static_cast<size_t>(this->pixelCount()));

            for (size_t index = 0; index < pixelLimit; ++index)
            {
                const auto &color = colors[index];
                for (size_t channel = 0; channel < _channelCount; ++channel)
                {
                    appendWireComponent(pixels,
                                        offset,
                                        color[_channelOrder[channel]]);
                }
            }
        }

        const char *_channelOrder;
        size_t _channelCount;
        size_t _sizeData;

        uint8_t *_data{nullptr};
    };

} // namespace npb


