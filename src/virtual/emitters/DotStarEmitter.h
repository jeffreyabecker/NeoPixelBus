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
#include "../buses/IClockDataTransport.h"
#include "../ResourceHandle.h"
#include "../colors/Color.h"

namespace npb
{

    // DotStar / APA102 brightness modes.
    //
    //   FixedBrightness — 0xFF prefix byte, W channel ignored
    //   Luminance       — 0xE0 | WW prefix, uses WW channel as 5-bit luminance
    //
    enum class DotStarMode : uint8_t
    {
        FixedBrightness,
        Luminance,
    };

    struct DotStarEmitterSettings
    {
        ResourceHandle<IClockDataTransport> bus;
        std::array<uint8_t, 3> channelOrder = {2, 1, 0}; // BGR default
        DotStarMode mode = DotStarMode::FixedBrightness;
    };

    /// Convenience: constructs TClockDataTransport in-place from busArgs and
    /// passes an owning ResourceHandle to the base settings.
    /// Extra fields (channelOrder, mode) can be modified after construction.
    template <typename TClockDataTransport>
        requires std::derived_from<TClockDataTransport, IClockDataTransport>
    struct DotStarEmitterSettingsOfT : DotStarEmitterSettings
    {
        template <typename... BusArgs>
        explicit DotStarEmitterSettingsOfT(BusArgs &&...busArgs)
            : DotStarEmitterSettings{
                  std::make_unique<TClockDataTransport>(std::forward<BusArgs>(busArgs)...)}
        {
        }
    };

    // DotStar / APA102 emitter.
    //
    // Wire format per pixel: [prefix] [ch1] [ch2] [ch3]  (4 bytes)
    // Framing:
    //   Start: 4 x 0x00
    //   End:   4 x 0x00 + ceil(N/16) x 0x00
    //
    class DotStarEmitter : public IEmitPixels
    {
    public:
        DotStarEmitter(uint16_t pixelCount,
                       ResourceHandle<IShader> shader,
                       DotStarEmitterSettings settings)
            : _settings{std::move(settings)}, _shader{std::move(shader)}, _pixelCount{pixelCount}, _scratchColors(pixelCount), _byteBuffer(pixelCount * BytesPerPixel), _endFrameExtraBytes{(pixelCount + 15u) / 16u}
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

            // Serialize
            size_t offset = 0;
            if (_settings.mode == DotStarMode::FixedBrightness)
            {
                for (const auto &color : source)
                {
                    _byteBuffer[offset++] = 0xFF;
                    _byteBuffer[offset++] = color[_settings.channelOrder[0]];
                    _byteBuffer[offset++] = color[_settings.channelOrder[1]];
                    _byteBuffer[offset++] = color[_settings.channelOrder[2]];
                }
            }
            else // Luminance
            {
                for (const auto &color : source)
                {
                    uint8_t lum = color[Color::IdxWW] < 31 ? color[Color::IdxWW] : 31;
                    _byteBuffer[offset++] = 0xE0 | lum;
                    _byteBuffer[offset++] = color[_settings.channelOrder[0]];
                    _byteBuffer[offset++] = color[_settings.channelOrder[1]];
                    _byteBuffer[offset++] = color[_settings.channelOrder[2]];
                }
            }

            _settings.bus->beginTransaction();

            const uint8_t zeroByte = 0x00;
            const std::span<const uint8_t> zeroSpan{&zeroByte, 1};

            // Start frame: 4 x 0x00
            for (size_t i = 0; i < StartFrameSize; ++i)
            {
                _settings.bus->transmitBytes(zeroSpan);
            }

            // Pixel data
            _settings.bus->transmitBytes(_byteBuffer);

            // End frame: 4 x 0x00
            for (size_t i = 0; i < EndFrameFixedSize; ++i)
            {
                _settings.bus->transmitBytes(zeroSpan);
            }

            // Extra end-frame bytes: ceil(N/16) x 0x00
            for (size_t i = 0; i < _endFrameExtraBytes; ++i)
            {
                _settings.bus->transmitBytes(zeroSpan);
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
        static constexpr size_t BytesPerPixel = 4;
        static constexpr size_t StartFrameSize = 4;
        static constexpr size_t EndFrameFixedSize = 4;

        DotStarEmitterSettings _settings;
        ResourceHandle<IShader> _shader;
        size_t _pixelCount;
        std::vector<Color> _scratchColors;
        std::vector<uint8_t> _byteBuffer;
        size_t _endFrameExtraBytes;
    };

} // namespace npb
