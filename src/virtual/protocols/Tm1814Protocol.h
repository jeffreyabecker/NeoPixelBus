#pragma once

#include <cstdint>
#include <cstddef>
#include <span>
#include <memory>
#include <vector>
#include <algorithm>

#include <Arduino.h>

#include "IProtocol.h"
#include "../transports/ISelfClockingTransport.h"
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
    ResourceHandle<ISelfClockingTransport> bus;
    const char* channelOrder = "WRGB";
    Tm1814CurrentSettings current{};
};

template<typename TSelfClockingTransport>
    requires std::derived_from<TSelfClockingTransport, ISelfClockingTransport>
struct Tm1814ProtocolSettingsOfT : Tm1814ProtocolSettings
{
    template<typename... BusArgs>
    explicit Tm1814ProtocolSettingsOfT(BusArgs&&... busArgs)
        : Tm1814ProtocolSettings{
            std::make_unique<TSelfClockingTransport>(std::forward<BusArgs>(busArgs)...)}
    {
    }
};

class Tm1814Protocol : public IProtocol
{
public:
    Tm1814Protocol(uint16_t pixelCount,
                   Tm1814ProtocolSettings settings)
        : _settings{std::move(settings)}
        , _pixelCount{pixelCount}
        , _frameBuffer(SettingsSize + static_cast<size_t>(pixelCount) * ChannelCount, 0)
    {
    }

    void initialize() override
    {
        _settings.bus->begin();
    }

    void update(std::span<const Color> colors) override
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

    void serializePixels(std::span<const Color> colors)
    {
        size_t offset = SettingsSize;
        for (const auto& color : colors)
        {
            for (size_t channel = 0; channel < ChannelCount; ++channel)
            {
                _frameBuffer[offset++] = color[_settings.channelOrder[channel]];
            }
        }
    }

    Tm1814ProtocolSettings _settings;
    uint16_t _pixelCount;
    std::vector<uint8_t> _frameBuffer;
};

} // namespace npb
