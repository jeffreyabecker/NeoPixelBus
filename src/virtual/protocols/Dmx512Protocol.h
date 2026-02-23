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
#include "../shaders/IShader.h"
#include "../transports/ISelfClockingTransport.h"
#include "../ResourceHandle.h"

namespace npb
{

struct Dmx512ProtocolSettings
{
    ResourceHandle<ISelfClockingTransport> bus;
    const char* channelOrder = ChannelOrder::RGB;
    size_t channelsPerPixel = ChannelOrder::LengthRGB;
};

template<typename TSelfClockingTransport>
    requires std::derived_from<TSelfClockingTransport, ISelfClockingTransport>
struct Dmx512ProtocolSettingsOfT : Dmx512ProtocolSettings
{
    template<typename... BusArgs>
    explicit Dmx512ProtocolSettingsOfT(BusArgs&&... busArgs)
        : Dmx512ProtocolSettings{
            std::make_unique<TSelfClockingTransport>(std::forward<BusArgs>(busArgs)...)}
    {
    }
};

class Dmx512Protocol : public IProtocol
{
public:
    Dmx512Protocol(uint16_t pixelCount,
                  ResourceHandle<IShader> shader,
                  Dmx512ProtocolSettings settings)
        : _settings{std::move(settings)}
        , _shader{std::move(shader)}
        , _scratchColors(pixelCount)
        , _frameBuffer(std::min(MaxFrameBytes,
            1u + static_cast<size_t>(pixelCount) * _settings.channelsPerPixel), 0)
    {
        if (_frameBuffer.empty())
        {
            _frameBuffer.resize(1, 0);
        }
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

        std::span<const Color> source = colors;
        if (nullptr != _shader)
        {
            std::copy(colors.begin(), colors.end(), _scratchColors.begin());
            _shader->apply(_scratchColors);
            source = _scratchColors;
        }

        _frameBuffer[0] = 0x00; // DMX start code slot

        size_t slot = 1;
        for (const auto& color : source)
        {
            if (slot >= _frameBuffer.size())
            {
                break;
            }

            _frameBuffer[slot++] = color[_settings.channelOrder[0]];
            if (_settings.channelsPerPixel > 1 && slot < _frameBuffer.size())
            {
                _frameBuffer[slot++] = color[_settings.channelOrder[1]];
            }
            if (_settings.channelsPerPixel > 2 && slot < _frameBuffer.size())
            {
                _frameBuffer[slot++] = color[_settings.channelOrder[2]];
            }
        }

        _settings.bus->transmitBytes(_frameBuffer);
    }

    bool isReadyToUpdate() const override
    {
        return _settings.bus->isReadyToUpdate();
    }

    bool alwaysUpdate() const override
    {
        return true;
    }

private:
    static constexpr size_t MaxFrameBytes = 513; // start code + 512 channel slots

    Dmx512ProtocolSettings _settings;
    ResourceHandle<IShader> _shader;
    std::vector<Color> _scratchColors;
    std::vector<uint8_t> _frameBuffer;
};

} // namespace npb
