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

struct Lpd8806ProtocolSettings
{
    ITransport *bus = nullptr;
    const char* channelOrder = ChannelOrder::GRB;
};



// LPD8806 protocol.
//
// Wire format: 7-bit color with MSB set ? (value >> 1) | 0x80 per channel.
// Framing:
//   Start: ceil(N / 32) bytes of 0x00
//   Pixel data: 3 bytes per pixel
//   End:   ceil(N / 32) bytes of 0xFF
//
class Lpd8806Protocol : public IProtocol<Rgb8Color>
{
public:
    using SettingsType = Lpd8806ProtocolSettings;
    using TransportCategory = TransportTag;

    Lpd8806Protocol(uint16_t pixelCount,
                   SettingsType settings)
        : IProtocol<Rgb8Color>(pixelCount)
        , _settings{std::move(settings)}
        , _byteBuffer(pixelCount * BytesPerPixel)
        , _frameSize{(pixelCount + 31u) / 32u}
    {
    }


    void initialize() override
    {
        _settings.bus->begin();
    }

    void update(span<const Rgb8Color> colors) override
    {
        // Serialize: 7-bit per channel with MSB set
        size_t offset = 0;
        const size_t pixelLimit = std::min(colors.size(), static_cast<size_t>(this->pixelCount()));
        for (size_t index = 0; index < pixelLimit; ++index)
        {
            const auto& color = colors[index];
            for (size_t channel = 0; channel < BytesPerPixel; ++channel)
            {
                _byteBuffer[offset++] = (color[_settings.channelOrder[channel]] >> 1) | 0x80;
            }
        }

        _settings.bus->beginTransaction();

        const uint8_t zeroByte = 0x00;
        const span<const uint8_t> zeroSpan{&zeroByte, 1};
        const uint8_t ffByte = 0xFF;
        const span<const uint8_t> ffSpan{&ffByte, 1};

        // Start frame: ceil(N/32) ? 0x00
        for (size_t i = 0; i < _frameSize; ++i)
        {
            _settings.bus->transmitBytes(zeroSpan);
        }

        // Pixel data
        _settings.bus->transmitBytes(span<const uint8_t>(_byteBuffer.data(), _byteBuffer.size()));

        // End frame: ceil(N/32) ? 0xFF
        for (size_t i = 0; i < _frameSize; ++i)
        {
            _settings.bus->transmitBytes(ffSpan);
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
    static constexpr size_t BytesPerPixel = ChannelOrder::LengthGRB;

    SettingsType _settings;
    std::vector<uint8_t> _byteBuffer;
    size_t _frameSize;
};

} // namespace npb


