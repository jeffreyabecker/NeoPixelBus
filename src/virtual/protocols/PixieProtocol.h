#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>
#include <vector>

#include <Arduino.h>

#include "IProtocol.h"
#include "../transports/ITransport.h"
#include "../ResourceHandle.h"

namespace npb
{

    struct PixieProtocolSettings
    {
        ResourceHandle<ITransport> bus;
        const char *channelOrder = ChannelOrder::RGB;
    };

    class PixieProtocol : public IProtocol<Rgb8Color>
    {
    public:
        using SettingsType = PixieProtocolSettings;
        using TransportCategory = OneWireTransportTag;

        PixieProtocol(uint16_t pixelCount,
                      PixieProtocolSettings settings)
            : _settings{std::move(settings)}, _byteBuffer(pixelCount * BytesPerPixel)
        {
        }


        void initialize() override
        {
            _settings.bus->begin();
        }

        void update(std::span<const Rgb8Color> colors) override
        {
            while (!isReadyToUpdate())
            {
                yield();
            }

            size_t offset = 0;
            for (const auto &color : colors)
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
        std::vector<uint8_t> _byteBuffer;
        uint32_t _endTime{0};
    };

} // namespace npb
