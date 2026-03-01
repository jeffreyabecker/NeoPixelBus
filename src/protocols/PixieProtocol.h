#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <vector>

#include <Arduino.h>

#include "IProtocol.h"
#include "transports/ITransport.h"

namespace lw
{

    struct PixieProtocolSettings
    {
        ITransport *bus = nullptr;
        const char *channelOrder = ChannelOrder::RGB::value;
    };

    class PixieProtocol : public IProtocol<Rgb8Color>
    {
    public:
        using SettingsType = PixieProtocolSettings;
        using TransportCategory = OneWireTransportTag;

        PixieProtocol(uint16_t pixelCount,
                  SettingsType settings)
            : IProtocol<Rgb8Color>(pixelCount), _settings{std::move(settings)}, _byteBuffer(pixelCount * BytesPerPixel)
        {
        }


        void initialize() override
        {
            _settings.bus->begin();
        }

        void update(span<const Rgb8Color> colors) override
        {
            while (!isReadyToUpdate())
            {
                yield();
            }

            size_t offset = 0;
            const size_t pixelLimit = std::min(colors.size(), static_cast<size_t>(this->pixelCount()));
            for (size_t index = 0; index < pixelLimit; ++index)
            {
                const auto &color = colors[index];
                for (size_t channel = 0; channel < BytesPerPixel; ++channel)
                {
                    _byteBuffer[offset++] = color[_settings.channelOrder[channel]];
                }
            }

            _settings.bus->beginTransaction();
            _settings.bus->transmitBytes(span<uint8_t>(_byteBuffer.data(), _byteBuffer.size()));
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
        static constexpr size_t BytesPerPixel = ChannelOrder::RGB::length;
        static constexpr uint32_t LatchDelayUs = 1000;

        SettingsType _settings;
        std::vector<uint8_t> _byteBuffer;
        uint32_t _endTime{0};
    };

} // namespace lw


