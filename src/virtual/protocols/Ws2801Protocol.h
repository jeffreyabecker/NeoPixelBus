#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>
#include <memory>
#include <vector>

#include <Arduino.h>

#include "IProtocol.h"
#include "../transports/ITransport.h"
#include "../ResourceHandle.h"

namespace npb
{

struct Ws2801ProtocolSettings
{
    ResourceHandle<ITransport> bus;
    const char* channelOrder = ChannelOrder::RGB;
};


// WS2801 protocol.
//
// Wire format: raw 3 bytes per pixel, full 8-bit per channel.
// No start or end frame.
// Latch: 500 µs clock-low after last byte.
//
class Ws2801Protocol : public IProtocol<Rgb8Color>
{
public:
    using SettingsType = Ws2801ProtocolSettings;
    using TransportCategory = TransportTag;

    Ws2801Protocol(uint16_t pixelCount,
                  SettingsType settings)
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
        // Serialize: raw 3-byte channel data in configured order
        size_t offset = 0;
        for (const auto& color : colors)
        {
            for (size_t channel = 0; channel < BytesPerPixel; ++channel)
            {
                _byteBuffer[offset++] = color[_settings.channelOrder[channel]];
            }
        }

        _settings.bus->beginTransaction();

        // No start frame — pure data stream
        _settings.bus->transmitBytes(_byteBuffer);

        _settings.bus->endTransaction();

        _endTime = micros();

        // Latch delay: 500 µs
        delayMicroseconds(LatchDelayUs);
    }

    bool isReadyToUpdate() const override
    {
        return (micros() - _endTime) >= LatchDelayUs;
    }

    bool alwaysUpdate() const override
    {
        return false;
    }

private:
    static constexpr size_t BytesPerPixel = ChannelOrder::LengthRGB;
    static constexpr uint32_t LatchDelayUs = 500;

    SettingsType _settings;
    size_t _pixelCount;
    std::vector<uint8_t> _byteBuffer;
    uint32_t _endTime{0};
};

} // namespace npb
