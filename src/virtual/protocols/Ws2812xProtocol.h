#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>
#include <span>
#include <vector>
#include <algorithm>

#include <Arduino.h>

#include "IProtocol.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"
#include "../colors/Color.h"
#include "../buses/ISelfClockingTransport.h"

namespace npb
{

    class Ws2812xProtocol : public IProtocol
    {
    public:
        Ws2812xProtocol(uint16_t pixelCount,
                        ResourceHandle<IShader> shader,
                        const char* channelOrder,
                        ResourceHandle<ISelfClockingTransport> transport)
            : _shader{std::move(shader)}
            , _channelOrder{resolveChannelOrder(channelOrder)}
            , _channelCount{resolveChannelCount(_channelOrder)}
            , _pixelCount{pixelCount}
            , _sizeData{bytesNeeded(pixelCount, _channelCount)}
            , _scratchColors(pixelCount)
            , _data(static_cast<uint8_t *>(malloc(_sizeData)))
            , _transport{std::move(transport)}
        {
            if (_data)
            {
                std::memset(_data, 0, _sizeData);
            }
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
            _transport->begin();
        }

        void update(std::span<const Color> colors) override
        {
            while (!isReadyToUpdate())
            {
                yield();
            }

            std::span<const Color> source = colors;
            if (nullptr != _shader)
            {
                std::copy(colors.begin(), colors.end(), _scratchColors.begin());
                _shader->apply(_scratchColors);
                source = _scratchColors;
            }

            serialize(std::span<uint8_t>{_data, _sizeData}, source);
            _transport->transmitBytes(std::span<const uint8_t>{_data, _sizeData});
        }

        bool isReadyToUpdate() const override
        {
            return _transport->isReadyToUpdate();
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
        static constexpr const char* resolveChannelOrder(const char* channelOrder)
        {
            return (nullptr != channelOrder) ? channelOrder : ChannelOrder::GRB;
        }

        static constexpr size_t resolveChannelCount(const char* channelOrder)
        {
            const size_t requestedCount = std::char_traits<char>::length(channelOrder);
            if (requestedCount == 0)
            {
                return ChannelOrder::LengthGRB;
            }

            return std::min(requestedCount, Color::ChannelCount);
        }

        static constexpr size_t bytesNeeded(size_t pixelCount, size_t channelCount)
        {
            return pixelCount * channelCount;
        }

        void serialize(std::span<uint8_t> pixels,
                       std::span<const Color> colors)
        {
            size_t offset = 0;

            for (const auto& color : colors)
            {
                for (size_t channel = 0; channel < _channelCount; ++channel)
                {
                    pixels[offset++] = color[_channelOrder[channel]];
                }
            }
        }

        ResourceHandle<IShader> _shader;
        const char* _channelOrder;
        size_t _channelCount;
        uint16_t _pixelCount;
        size_t _sizeData;

        std::vector<Color> _scratchColors;
        uint8_t *_data{nullptr};
        ResourceHandle<ISelfClockingTransport> _transport;
    };

} // namespace npb
