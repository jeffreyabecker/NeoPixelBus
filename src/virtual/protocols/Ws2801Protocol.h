#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>
#include <memory>
#include <vector>

#include <Arduino.h>

#include "IProtocol.h"
#include "../transports/IClockDataTransport.h"
#include "../ResourceHandle.h"

namespace npb
{

struct Ws2801ProtocolSettings
{
    ResourceHandle<IClockDataTransport> bus;
    const char* channelOrder = ChannelOrder::RGB;
};

template<typename TClockDataTransport>
    requires std::derived_from<TClockDataTransport, IClockDataTransport>
struct Ws2801ProtocolSettingsOfT : Ws2801ProtocolSettings
{
    template<typename... BusArgs>
    explicit Ws2801ProtocolSettingsOfT(BusArgs&&... busArgs)
        : Ws2801ProtocolSettings{
            std::make_unique<TClockDataTransport>(std::forward<BusArgs>(busArgs)...)}
    {
    }
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
    Ws2801Protocol(uint16_t pixelCount,
                  Ws2801ProtocolSettings settings)
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

    Ws2801ProtocolSettings _settings;
    size_t _pixelCount;
    std::vector<uint8_t> _byteBuffer;
    uint32_t _endTime{0};
};

} // namespace npb
