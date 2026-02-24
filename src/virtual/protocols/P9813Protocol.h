#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <memory>
#include <vector>

#include <Arduino.h>

#include "IProtocol.h"
#include "../transports/ITransport.h"
#include "../ResourceHandle.h"

namespace npb
{

struct P9813ProtocolSettings
{
    ResourceHandle<ITransport> bus;
};

template<typename TClockDataTransport>
    requires TaggedTransportLike<TClockDataTransport, ClockDataTransportTag>
struct P9813ProtocolSettingsT : P9813ProtocolSettings
{
    template<typename... BusArgs>
    explicit P9813ProtocolSettingsT(BusArgs&&... busArgs)
        : P9813ProtocolSettings{
            std::make_unique<TClockDataTransport>(std::forward<BusArgs>(busArgs)...)}
    {
    }
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
//   Start: 4 × 0x00
//   End:   4 × 0x00
//
class P9813Protocol : public IProtocol<Rgb8Color>
{
public:
    using SettingsType = P9813ProtocolSettings;

    P9813Protocol(uint16_t pixelCount,
                 P9813ProtocolSettings settings)
        : _settings{std::move(settings)}
        , _pixelCount{pixelCount}
        , _byteBuffer(pixelCount * BytesPerPixel)
    {
    }


    void initialize() override
    {
        _settings.bus->begin();
    }

    void update(std::span<const Rgb8Color> colors) override
    {
        // Serialize: checksum prefix + BGR
        size_t offset = 0;
        for (const auto& color : colors)
        {
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

        const uint8_t zeroByte = 0x00;
        const std::span<const uint8_t> zeroSpan{&zeroByte, 1};

        // Start frame: 4 × 0x00
        for (size_t i = 0; i < FrameSize; ++i)
        {
            _settings.bus->transmitBytes(zeroSpan);
        }

        // Pixel data
        _settings.bus->transmitBytes(_byteBuffer);

        // End frame: 4 × 0x00
        for (size_t i = 0; i < FrameSize; ++i)
        {
            _settings.bus->transmitBytes(zeroSpan);
        }

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

private:
    static constexpr size_t BytesPerPixel = 4;
    static constexpr size_t FrameSize = 4;

    P9813ProtocolSettings _settings;
    size_t _pixelCount;
    std::vector<uint8_t> _byteBuffer;
};

} // namespace npb
