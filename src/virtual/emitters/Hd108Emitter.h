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
#include "../buses/IClockDataBus.h"
#include "../ResourceHandle.h"
#include "../colors/Color.h"

namespace npb
{

struct Hd108EmitterSettings
{
    ResourceHandle<IClockDataBus> bus;
    std::array<uint8_t, 3> channelOrder = {2, 1, 0};  // BGR default
};

template<typename TClockDataBus>
    requires std::derived_from<TClockDataBus, IClockDataBus>
struct Hd108EmitterSettingsOfT : Hd108EmitterSettings
{
    template<typename... BusArgs>
    explicit Hd108EmitterSettingsOfT(BusArgs&&... busArgs)
        : Hd108EmitterSettings{
            std::make_unique<TClockDataBus>(std::forward<BusArgs>(busArgs)...)}
    {
    }
};

// HD108 emitter.
//
// Wire format per pixel: 8 bytes
//   [2-byte prefix] [ch1 hi][ch1 lo] [ch2 hi][ch2 lo] [ch3 hi][ch3 lo]
//
// Prefix: 0xFFFF (all brightness bits max, upper bit always 1)
//   Layout: {1}{5-bit brightness ch1}{5-bit brightness ch2}{5-bit brightness ch3}
//   At max brightness → 0xFFFF.
//
// Channels are 16-bit big-endian, expanded from 8-bit via byte replication.
//
// Framing:
//   Start: 16 x 0x00
//   End:    4 x 0xFF
//
class Hd108Emitter : public IEmitPixels
{
public:
    Hd108Emitter(uint16_t pixelCount,
                 ResourceHandle<IShader> shader,
                 Hd108EmitterSettings settings)
        : _settings{std::move(settings)}
        , _shader{std::move(shader)}
        , _pixelCount{pixelCount}
        , _scratchColors(pixelCount)
        , _byteBuffer(pixelCount * BytesPerPixel)
    {
    }

    void initialize() override
    {
        _settings.bus->begin();
    }

    void update(std::span<const Color> colors) override
    {
        // Apply shader
        std::span<const Color> source = colors;
        if (nullptr != _shader)
        {
            std::copy(colors.begin(), colors.end(), _scratchColors.begin());
            _shader->apply(_scratchColors);
            source = _scratchColors;
        }

        // Serialize: 16-bit per channel, big-endian
        size_t offset = 0;
        for (const auto& color : source)
        {
            // Prefix: all brightness bits max
            _byteBuffer[offset++] = 0xFF;
            _byteBuffer[offset++] = 0xFF;

            // 3 channels, 8→16 via byte replication
            for (size_t ch = 0; ch < 3; ++ch)
            {
                uint8_t val = color[_settings.channelOrder[ch]];
                _byteBuffer[offset++] = val;   // high byte
                _byteBuffer[offset++] = val;   // low byte (replicate)
            }
        }

        _settings.bus->beginTransaction();

        // Start frame: 16 x 0x00
        for (size_t i = 0; i < StartFrameSize; ++i)
        {
            _settings.bus->transmitByte(0x00);
        }

        // Pixel data
        _settings.bus->transmitBytes(_byteBuffer);

        // End frame: 4 x 0xFF
        for (size_t i = 0; i < EndFrameSize; ++i)
        {
            _settings.bus->transmitByte(0xFF);
        }

        _settings.bus->endTransaction();
    }

    bool isReadyToUpdate() const override
    {
        return true;
    }

    bool alwaysUpdate() const override
    {
        return false;
    }

private:
    static constexpr size_t BytesPerPixel = 8;       // 2 prefix + 3 × 2 channel
    static constexpr size_t StartFrameSize = 16;
    static constexpr size_t EndFrameSize = 4;

    Hd108EmitterSettings _settings;
    ResourceHandle<IShader> _shader;
    size_t _pixelCount;
    std::vector<Color> _scratchColors;
    std::vector<uint8_t> _byteBuffer;
};

} // namespace npb
