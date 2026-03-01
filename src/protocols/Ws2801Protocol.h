#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <memory>
#include <vector>

#include <Arduino.h>

#include "IProtocol.h"
#include "transports/ITransport.h"

namespace lw
{

struct Ws2801ProtocolSettings
{
    ITransport *bus = nullptr;
    const char* channelOrder = ChannelOrder::RGB::value;
};


// WS2801 protocol.
//
// Wire format: raw 3 bytes per pixel, full 8-bit per channel.
// No start or end frame.
// Latch: 500 ?s clock-low after last byte.
//
class Ws2801Protocol : public IProtocol<Rgb8Color>
{
public:
    using SettingsType = Ws2801ProtocolSettings;
    using TransportCategory = TransportTag;

    static size_t requiredBufferSize(uint16_t pixelCount,
                                     const SettingsType &)
    {
        return static_cast<size_t>(pixelCount) * BytesPerPixel;
    }

    Ws2801Protocol(uint16_t pixelCount,
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

        // Serialize: raw 3-byte channel data in configured order
        size_t offset = 0;
        const size_t pixelLimit = std::min(colors.size(), static_cast<size_t>(this->pixelCount()));
        for (size_t index = 0; index < pixelLimit; ++index)
        {
            const auto& color = colors[index];
            for (size_t channel = 0; channel < BytesPerPixel; ++channel)
            {
                _byteBuffer[offset++] = color[_settings.channelOrder[channel]];
            }
        }

        _settings.bus->beginTransaction();

        // No start frame ? pure data stream
        _settings.bus->transmitBytes(_byteBuffer);

        _settings.bus->endTransaction();

        _endTime = micros();

        // Latch delay: 500 ?s
        delayMicroseconds(LatchDelayUs);
    }

    bool isReadyToUpdate() const override
    {
        if (_byteBuffer.size() != _requiredBufferSize)
        {
            return false;
        }

        return (micros() - _endTime) >= LatchDelayUs;
    }

    bool alwaysUpdate() const override
    {
        return false;
    }

    size_t requiredBufferSizeBytes() const override
    {
        return _requiredBufferSize;
    }

private:
    static constexpr size_t BytesPerPixel = ChannelOrder::RGB::length;
    static constexpr uint32_t LatchDelayUs = 500;

    SettingsType _settings;
    size_t _requiredBufferSize{0};
    span<uint8_t> _byteBuffer{};
    uint32_t _endTime{0};
};

} // namespace lw


