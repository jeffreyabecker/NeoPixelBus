#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <vector>

#include <Arduino.h>

#include "IProtocol.h"
#include "transports/ITransport.h"

namespace lw
{

struct P9813ProtocolSettings
{
    ITransport *bus = nullptr;
};



// P9813 protocol (Total Control Lighting).
//
// Wire format: 4 bytes per pixel.
//   Byte 0: 0xC0 | (~B >> 6 & 3) << 4 | (~G >> 6 & 3) << 2 | (~R >> 6 & 3)
//   Byte 1: Blue
//   Byte 2: Green
//   Byte 3: Red
//
// The header byte contains inverted top-2-bits of each channel as a checksum.
// Fixed channel order: BGR in data bytes.
//
// Framing:
//   Start: 4 ? 0x00
//   End:   4 ? 0x00
//
class P9813Protocol : public IProtocol<Rgb8Color>
{
public:
    using SettingsType = P9813ProtocolSettings;
    using TransportCategory = TransportTag;

    static size_t requiredBufferSize(uint16_t pixelCount,
                                     const SettingsType &)
    {
        return (FrameSize * 2u) + (static_cast<size_t>(pixelCount) * BytesPerPixel);
    }

    P9813Protocol(uint16_t pixelCount,
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

        std::fill(_byteBuffer.begin(), _byteBuffer.begin() + FrameSize, 0x00);
        std::fill(_byteBuffer.end() - FrameSize, _byteBuffer.end(), 0x00);
        _settings.bus->begin();
    }

    void update(span<const Rgb8Color> colors) override
    {
        if (_settings.bus == nullptr || _byteBuffer.size() != _requiredBufferSize)
        {
            return;
        }

        // Serialize: checksum prefix + BGR
        size_t offset = FrameSize;
        const size_t pixelLimit = std::min(colors.size(), static_cast<size_t>(this->pixelCount()));
        for (size_t index = 0; index < pixelLimit; ++index)
        {
            const auto& color = colors[index];
            uint8_t r = color['R'];
            uint8_t g = color['G'];
            uint8_t b = color['B'];

            // Header: 0xC0 | inverted top-2-bits of each channel
            uint8_t header = 0xC0
                | ((~b >> 6) & 0x03) << 4
                | ((~g >> 6) & 0x03) << 2
                | ((~r >> 6) & 0x03);

            _byteBuffer[offset++] = header;
            _byteBuffer[offset++] = b;
            _byteBuffer[offset++] = g;
            _byteBuffer[offset++] = r;
        }

        _settings.bus->beginTransaction();
        _settings.bus->transmitBytes(_byteBuffer);
        _settings.bus->endTransaction();
    }

    bool isReadyToUpdate() const override
    {
        if (_settings.bus == nullptr || _byteBuffer.size() != _requiredBufferSize)
        {
            return false;
        }

        return _settings.bus->isReadyToUpdate();
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
    static constexpr size_t BytesPerPixel = 4;
    static constexpr size_t FrameSize = 4;

    SettingsType _settings;
    size_t _requiredBufferSize{0};
    span<uint8_t> _byteBuffer{};
};

} // namespace lw


