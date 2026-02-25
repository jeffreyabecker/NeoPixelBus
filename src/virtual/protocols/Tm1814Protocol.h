#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <memory>
#include <vector>
#include <algorithm>

#include <Arduino.h>

#include "IProtocol.h"
#include "../transports/ITransport.h"
#include "../ResourceHandle.h"

namespace npb
{

struct Tm1814CurrentSettings
{
    uint16_t redMilliAmps = 190;
    uint16_t greenMilliAmps = 190;
    uint16_t blueMilliAmps = 190;
    uint16_t whiteMilliAmps = 190;
};

struct Tm1814ProtocolSettings
{
    ResourceHandle<ITransport> bus;
    const char* channelOrder = "WRGB";
    Tm1814CurrentSettings current{};
};


class Tm1814Protocol : public IProtocol<Rgbw8Color>
{
public:
    using SettingsType = Tm1814ProtocolSettings;
    using TransportCategory = OneWireTransportTag;

    Tm1814Protocol(uint16_t pixelCount,
                   SettingsType settings)
        : _settings{std::move(settings)}
        , _pixelCount{pixelCount}
        , _frameBuffer(SettingsSize + static_cast<size_t>(pixelCount) * ChannelCount, 0)
    {
    }

    void initialize() override
    {
        _settings.bus->begin();
    }

    void update(std::span<const Rgbw8Color> colors) override
    {
        while (!_settings.bus->isReadyToUpdate())
        {
            yield();
        }

        encodeSettings();
        serializePixels(colors);

        _settings.bus->transmitBytes(_frameBuffer);
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
    static constexpr size_t ChannelCount = 4;
    static constexpr size_t SettingsSize = 8;
    static constexpr uint16_t MinCurrent = 65;
    static constexpr uint16_t MaxCurrent = 380;
    static constexpr uint16_t EncodeDivisor = 5;

    static uint8_t encodeCurrent(uint16_t milliAmps)
    {
        const uint16_t limited = std::clamp(milliAmps, MinCurrent, MaxCurrent);
        return static_cast<uint8_t>((limited - MinCurrent) / EncodeDivisor);
    }

    uint8_t currentForChannel(char channel) const
    {
        switch (channel)
        {
        case 'R':
        case 'r':
            return encodeCurrent(_settings.current.redMilliAmps);

        case 'G':
        case 'g':
            return encodeCurrent(_settings.current.greenMilliAmps);

        case 'B':
        case 'b':
            return encodeCurrent(_settings.current.blueMilliAmps);

        case 'W':
        case 'w':
        default:
            return encodeCurrent(_settings.current.whiteMilliAmps);
        }
    }

    void encodeSettings()
    {
        for (size_t i = 0; i < ChannelCount; ++i)
        {
            _frameBuffer[i] = currentForChannel(_settings.channelOrder[i]);
        }

        for (size_t i = 0; i < ChannelCount; ++i)
        {
            _frameBuffer[ChannelCount + i] = static_cast<uint8_t>(~_frameBuffer[i]);
        }
    }

    void serializePixels(std::span<const Rgbw8Color> colors)
    {
        size_t offset = SettingsSize;
        const size_t pixelLimit = std::min(colors.size(), static_cast<size_t>(_pixelCount));
        for (size_t index = 0; index < pixelLimit; ++index)
        {
            const auto& color = colors[index];
            for (size_t channel = 0; channel < ChannelCount; ++channel)
            {
                _frameBuffer[offset++] = color[_settings.channelOrder[channel]];
            }
        }
    }

    SettingsType _settings;
    uint16_t _pixelCount;
    std::vector<uint8_t> _frameBuffer;
};

} // namespace npb
