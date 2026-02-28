#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <memory>
#include <vector>
#include <algorithm>

#include <Arduino.h>

#include "IProtocol.h"
#include "transports/ITransport.h"

namespace npb
{

struct Lpd6803ProtocolSettings
{
    ITransport *bus = nullptr;
    const char* channelOrder = ChannelOrder::RGB::value;
};



// LPD6803 protocol.
//
// Wire format: 5-5-5 packed RGB into 2 bytes per pixel (big-endian).
//   Bit 15: always 1
//   Bits 14..10: channel 1 (top 5 bits)
//   Bits  9.. 5: channel 2 (top 5 bits)
//   Bits  4.. 0: channel 3 (top 5 bits)
//
// Framing:
//   Start: 4 ? 0x00
//   Pixel data: 2 bytes per pixel
//   End:   ceil(N / 8) bytes of 0x00  (1 bit per pixel)
//
class Lpd6803Protocol : public IProtocol<Rgb8Color>
{
public:
    using SettingsType = Lpd6803ProtocolSettings;
    using TransportCategory = TransportTag;

    Lpd6803Protocol(uint16_t pixelCount,
                   SettingsType settings)
        : IProtocol<Rgb8Color>(pixelCount)
        , _settings{std::move(settings)}
        , _endFrameSize{(pixelCount + 7u) / 8u}
        , _byteBuffer(StartFrameSize + (pixelCount * BytesPerPixel) + ((pixelCount + 7u) / 8u), 0)
    {
    }


    void initialize() override
    {
        std::fill(_byteBuffer.begin(), _byteBuffer.begin() + StartFrameSize, 0x00);
        std::fill(_byteBuffer.end() - _endFrameSize, _byteBuffer.end(), 0x00);
        _settings.bus->begin();
    }

    void update(span<const Rgb8Color> colors) override
    {
        // Serialize: 5-5-5 packed into 2 bytes per pixel
        size_t offset = StartFrameSize;
        const size_t pixelLimit = std::min(colors.size(), static_cast<size_t>(this->pixelCount()));
        for (size_t index = 0; index < pixelLimit; ++index)
        {
            const auto& color = colors[index];
            uint8_t ch1 = color[_settings.channelOrder[0]] & 0xF8;
            uint8_t ch2 = color[_settings.channelOrder[1]] & 0xF8;
            uint8_t ch3 = color[_settings.channelOrder[2]] & 0xF8;

            // Pack: 1_ccccc_ccccc_ccccc (big-endian)
            uint16_t packed = 0x8000
                | (static_cast<uint16_t>(ch1) << 7)
                | (static_cast<uint16_t>(ch2) << 2)
                | (static_cast<uint16_t>(ch3) >> 3);

            _byteBuffer[offset++] = static_cast<uint8_t>(packed >> 8);
            _byteBuffer[offset++] = static_cast<uint8_t>(packed & 0xFF);
        }

        _settings.bus->beginTransaction();
        _settings.bus->transmitBytes(span<uint8_t>(_byteBuffer.data(), _byteBuffer.size()));
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
    static constexpr size_t BytesPerPixel = 2;
    static constexpr size_t StartFrameSize = 4;

    SettingsType _settings;
    std::vector<uint8_t> _byteBuffer;
    size_t _endFrameSize;
};

} // namespace npb


