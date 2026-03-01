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

        static size_t requiredBufferSize(uint16_t pixelCount,
                                         const SettingsType &)
        {
            return static_cast<size_t>(pixelCount) * BytesPerPixel;
        }

        PixieProtocol(uint16_t pixelCount,
                  SettingsType settings)
            : IProtocol<Rgb8Color>(pixelCount)
            , _settings{std::move(settings)}
            , _requiredBufferSize(requiredBufferSize(pixelCount, _settings))
        {
        }

        void setBuffer(span<uint8_t> buffer) override
        {
            if (buffer.size() < _requiredBufferSize)
            {
                _byteBuffer = span<uint8_t>{};
                return;
            }

            _byteBuffer = span<uint8_t>{buffer.data(), _requiredBufferSize};
        }

        void bindTransport(ITransport *transport) override
        {
            _settings.bus = transport;
        }


        void initialize() override
        {
            if (_settings.bus == nullptr || _byteBuffer.size() != _requiredBufferSize)
            {
                return;
            }

            _settings.bus->begin();
        }

        void update(span<const Rgb8Color> colors) override
        {
            if (_settings.bus == nullptr || _byteBuffer.size() != _requiredBufferSize)
            {
                return;
            }

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
            _settings.bus->transmitBytes(_byteBuffer);
            _settings.bus->endTransaction();

            _endTime = micros();
        }

        bool isReadyToUpdate() const override
        {
            if (_settings.bus == nullptr || _byteBuffer.size() != _requiredBufferSize)
            {
                return false;
            }

            return _settings.bus->isReadyToUpdate() && ((micros() - _endTime) >= LatchDelayUs);
        }

        bool alwaysUpdate() const override
        {
            return true;
        }

        size_t requiredBufferSizeBytes() const override
        {
            return _requiredBufferSize;
        }

    private:
        static constexpr size_t BytesPerPixel = ChannelOrder::RGB::length;
        static constexpr uint32_t LatchDelayUs = 1000;

        SettingsType _settings;
        size_t _requiredBufferSize{0};
        span<uint8_t> _byteBuffer{};
        uint32_t _endTime{0};
    };

} // namespace lw


