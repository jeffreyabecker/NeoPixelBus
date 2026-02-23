#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>
#include <memory>
#include <vector>
#include <algorithm>

#include <Arduino.h>

#include "IProtocol.h"
#include "../transports/IClockDataTransport.h"
#include "../ResourceHandle.h"

namespace npb
{

struct Lpd8806ProtocolSettings
{
    ResourceHandle<IClockDataTransport> bus;
    const char* channelOrder = ChannelOrder::GRB;
};

template<typename TClockDataTransport>
    requires std::derived_from<TClockDataTransport, IClockDataTransport>
struct Lpd8806ProtocolSettingsOfT : Lpd8806ProtocolSettings
{
    template<typename... BusArgs>
    explicit Lpd8806ProtocolSettingsOfT(BusArgs&&... busArgs)
        : Lpd8806ProtocolSettings{
            std::make_unique<TClockDataTransport>(std::forward<BusArgs>(busArgs)...)}
    {
    }
};

// LPD8806 protocol.
//
// Wire format: 7-bit color with MSB set — (value >> 1) | 0x80 per channel.
// Framing:
//   Start: ceil(N / 32) bytes of 0x00
//   Pixel data: 3 bytes per pixel
//   End:   ceil(N / 32) bytes of 0xFF
//
class Lpd8806Protocol : public IProtocol
{
public:
    Lpd8806Protocol(uint16_t pixelCount,
                   Lpd8806ProtocolSettings settings)
        : _settings{std::move(settings)}
        , _pixelCount{pixelCount}
        , _byteBuffer(pixelCount * BytesPerPixel)
        , _frameSize{(pixelCount + 31u) / 32u}
    {
    }

    void initialize() override
    {
        _settings.bus->begin();
    }

    void update(std::span<const Color> colors) override
    {
        // Serialize: 7-bit per channel with MSB set
        size_t offset = 0;
        for (const auto& color : colors)
        {
            for (size_t channel = 0; channel < BytesPerPixel; ++channel)
            {
                _byteBuffer[offset++] = (color[_settings.channelOrder[channel]] >> 1) | 0x80;
            }
        }

        _settings.bus->beginTransaction();

        const uint8_t zeroByte = 0x00;
        const std::span<const uint8_t> zeroSpan{&zeroByte, 1};
        const uint8_t ffByte = 0xFF;
        const std::span<const uint8_t> ffSpan{&ffByte, 1};

        // Start frame: ceil(N/32) × 0x00
        for (size_t i = 0; i < _frameSize; ++i)
        {
            _settings.bus->transmitBytes(zeroSpan);
        }

        // Pixel data
        _settings.bus->transmitBytes(_byteBuffer);

        // End frame: ceil(N/32) × 0xFF
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

    Lpd8806ProtocolSettings _settings;
    size_t _pixelCount;
    std::vector<uint8_t> _byteBuffer;
    size_t _frameSize;
};

} // namespace npb
