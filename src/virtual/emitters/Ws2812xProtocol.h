#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>
#include <vector>
#include <algorithm>

#include <Arduino.h>

#include "IProtocol.h"
#include "ColorOrderTransform.h"
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
                        ColorOrderTransformConfig colorConfig,
                        ResourceHandle<ISelfClockingTransport> transport)
            : _shader{std::move(shader)}
            , _transform{colorConfig}
            , _pixelCount{pixelCount}
            , _sizeData{_transform.bytesNeeded(pixelCount)}
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

            _transform.apply(std::span<uint8_t>{_data, _sizeData}, source);
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
        ResourceHandle<IShader> _shader;
        ColorOrderTransform _transform;
        uint16_t _pixelCount;
        size_t _sizeData;

        std::vector<Color> _scratchColors;
        uint8_t *_data{nullptr};
        ResourceHandle<ISelfClockingTransport> _transport;
    };

} // namespace npb
