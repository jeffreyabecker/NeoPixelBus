#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>
#include <vector>
#include <algorithm>

#include <Arduino.h>

#include "IProtocol.h"
#include "../shaders/IShader.h"
#include "../buses/IClockDataTransport.h"
#include "../ResourceHandle.h"

namespace npb
{

    struct PixieProtocolSettings
    {
        ResourceHandle<IClockDataTransport> bus;
        const char* channelOrder = ChannelOrder::RGB;
    };

    class PixieProtocol : public IProtocol
    {
    public:
        PixieProtocol(uint16_t pixelCount,
                      ResourceHandle<IShader> shader,
                      PixieProtocolSettings settings)
            : _settings{std::move(settings)}
            , _shader{std::move(shader)}
            , _scratchColors(pixelCount)
            , _byteBuffer(pixelCount * BytesPerPixel)
        {
        }

        void initialize() override
        {
            _settings.bus->begin();
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

            size_t offset = 0;
            for (const auto &color : source)
            {
                for (size_t channel = 0; channel < BytesPerPixel; ++channel)
                {
                    _byteBuffer[offset++] = color[_settings.channelOrder[channel]];
                }
            }

            _settings.bus->beginTransaction();
            _settings.bus->transmitBytes(_byteBuffer);
            _settings.bus->endTransaction();

            _endTime = micros();
        }

        bool isReadyToUpdate() const override
        {
            return _settings.bus->isReadyToUpdate() && ((micros() - _endTime) >= LatchDelayUs);
        }

        bool alwaysUpdate() const override
        {
            return true;
        }

    private:
        static constexpr size_t BytesPerPixel = ChannelOrder::LengthRGB;
        static constexpr uint32_t LatchDelayUs = 1000;

        PixieProtocolSettings _settings;
        ResourceHandle<IShader> _shader;
        std::vector<Color> _scratchColors;
        std::vector<uint8_t> _byteBuffer;
        uint32_t _endTime{0};
    };

} // namespace npb
