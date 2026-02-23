#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <span>
#include <vector>
#include <algorithm>

#include <Arduino.h>
#include <Stream.h>

#include "IProtocol.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"

namespace npb
{

struct PixieStreamProtocolSettings
{
    Stream& output;
    std::array<uint8_t, 3> channelOrder = {Color::IdxR, Color::IdxG, Color::IdxB};
};

class PixieStreamProtocol : public IProtocol
{
public:
    PixieStreamProtocol(uint16_t pixelCount,
                       ResourceHandle<IShader> shader,
                       PixieStreamProtocolSettings settings)
        : _settings{std::move(settings)}
        , _shader{std::move(shader)}
        , _scratchColors(pixelCount)
        , _byteBuffer(pixelCount * BytesPerPixel)
    {
    }

    void initialize() override
    {
        // UART/stream configuration is owned by caller.
    }

    void update(std::span<const Color> colors) override
    {
        while (!isReadyToUpdate())
        {
        }

        std::span<const Color> source = colors;
        if (nullptr != _shader)
        {
            std::copy(colors.begin(), colors.end(), _scratchColors.begin());
            _shader->apply(_scratchColors);
            source = _scratchColors;
        }

        size_t offset = 0;
        for (const auto& color : source)
        {
            _byteBuffer[offset++] = color[_settings.channelOrder[0]];
            _byteBuffer[offset++] = color[_settings.channelOrder[1]];
            _byteBuffer[offset++] = color[_settings.channelOrder[2]];
        }

        _settings.output.write(_byteBuffer.data(), _byteBuffer.size());
        _endTime = micros();
    }

    bool isReadyToUpdate() const override
    {
        return (micros() - _endTime) >= LatchDelayUs;
    }

    bool alwaysUpdate() const override
    {
        return true;
    }

private:
    static constexpr size_t BytesPerPixel = 3;
    static constexpr uint32_t LatchDelayUs = 1000;

    PixieStreamProtocolSettings _settings;
    ResourceHandle<IShader> _shader;
    std::vector<Color> _scratchColors;
    std::vector<uint8_t> _byteBuffer;
    uint32_t _endTime{0};
};

} // namespace npb
