#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>
#include <memory>
#include <vector>
#include <algorithm>

#include <Arduino.h>

#include "IEmitPixels.h"
#include "../shaders/IShader.h"
#include "../buses/ISelfClockingTransport.h"
#include "../ResourceHandle.h"

namespace npb
{

struct Dmx512EmitterSettings
{
    ResourceHandle<ISelfClockingTransport> bus;
    std::array<uint8_t, 3> channelOrder = {Color::IdxR, Color::IdxG, Color::IdxB};
    size_t channelsPerPixel = 3;
};

template<typename TSelfClockingTransport>
    requires std::derived_from<TSelfClockingTransport, ISelfClockingTransport>
struct Dmx512EmitterSettingsOfT : Dmx512EmitterSettings
{
    template<typename... BusArgs>
    explicit Dmx512EmitterSettingsOfT(BusArgs&&... busArgs)
        : Dmx512EmitterSettings{
            std::make_unique<TSelfClockingTransport>(std::forward<BusArgs>(busArgs)...)}
    {
    }
};

class Dmx512Emitter : public IEmitPixels
{
public:
    Dmx512Emitter(uint16_t pixelCount,
                  ResourceHandle<IShader> shader,
                  Dmx512EmitterSettings settings)
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

    Dmx512EmitterSettings _settings;
    ResourceHandle<IShader> _shader;
    std::vector<Color> _scratchColors;
    std::vector<uint8_t> _frameBuffer;
};

} // namespace npb
