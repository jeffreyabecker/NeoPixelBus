#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <vector>
#include <algorithm>

#include <Arduino.h>

#include "IProtocol.h"
#include "transports/ITransport.h"
#include "core/ResourceHandle.h"

namespace npb
{

enum class Tm1914Mode : uint8_t
{
    DinFdinAutoSwitch,
    DinOnly,
    FdinOnly
};

struct Tm1914ProtocolSettings
{
    ResourceHandle<ITransport> bus;
    const char* channelOrder = ChannelOrder::GRB;
    Tm1914Mode mode = Tm1914Mode::DinOnly;
};

class Tm1914Protocol : public IProtocol<Rgb8Color>
{
public:
    using SettingsType = Tm1914ProtocolSettings;
    using TransportCategory = OneWireTransportTag;

    Tm1914Protocol(uint16_t pixelCount,
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

    void update(span<const Rgb8Color> colors) override
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
    static constexpr size_t ChannelCount = 3;
    static constexpr size_t SettingsSize = 6;

    uint8_t encodedMode() const
    {
        switch (_settings.mode)
        {
        case Tm1914Mode::DinFdinAutoSwitch:
            return 0xff;

        case Tm1914Mode::FdinOnly:
            return 0xfa;

        case Tm1914Mode::DinOnly:
        default:
            return 0xf5;
        }
    }

    void encodeSettings()
    {
        _frameBuffer[0] = 0xff;
        _frameBuffer[1] = 0xff;
        _frameBuffer[2] = encodedMode();

        _frameBuffer[3] = static_cast<uint8_t>(~_frameBuffer[0]);
        _frameBuffer[4] = static_cast<uint8_t>(~_frameBuffer[1]);
        _frameBuffer[5] = static_cast<uint8_t>(~_frameBuffer[2]);
    }

    void serializePixels(span<const Rgb8Color> colors)
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


