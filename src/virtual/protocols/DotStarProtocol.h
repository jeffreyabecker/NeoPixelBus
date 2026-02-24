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
#include "../transports/ITransport.h"
#include "../ResourceHandle.h"
#include "../colors/Color.h"

namespace npb
{

    // DotStar / APA102 brightness modes.
    //
    //   FixedBrightness — 0xFF prefix byte, W channel ignored
    //   Luminance       — 0xE0 | luminance prefix (Rgb8Color has no W channel)
    //
    enum class DotStarMode : uint8_t
    {
        FixedBrightness,
        Luminance,
    };

    struct DotStarProtocolSettings
    {
        ResourceHandle<ITransport> bus;
        const char* channelOrder = ChannelOrder::BGR;
        DotStarMode mode = DotStarMode::FixedBrightness;
    };

    /// Convenience: constructs TClockDataTransport in-place from busArgs and
    /// passes an owning ResourceHandle to the base settings.
    /// Extra fields (channelOrder, mode) can be modified after construction.
    template <typename TClockDataTransport>
        requires TaggedTransportLike<TClockDataTransport, ClockDataTransportTag>
    struct DotStarProtocolSettingsT : DotStarProtocolSettings
    {
        template <typename... BusArgs>
        explicit DotStarProtocolSettingsT(BusArgs &&...busArgs)
            : DotStarProtocolSettings{
                  std::make_unique<TClockDataTransport>(std::forward<BusArgs>(busArgs)...)}
        {
        }
    };

    // DotStar / APA102 protocol.
    //
    // Wire format per pixel: [prefix] [ch1] [ch2] [ch3]  (4 bytes)
    // Framing:
    //   Start: 4 x 0x00
    //   End:   4 x 0x00 + ceil(N/16) x 0x00
    //
    class DotStarProtocol : public IProtocol<Rgb8Color>
    {
    public:
        DotStarProtocol(uint16_t pixelCount,
                       DotStarProtocolSettings settings)
            : _settings{std::move(settings)}, _pixelCount{pixelCount}, _byteBuffer(pixelCount * BytesPerPixel), _endFrameExtraBytes{(pixelCount + 15u) / 16u}
        {
        }

        void initialize() override
        {
            _settings.bus->begin();
        }

        void update(std::span<const Rgb8Color> colors) override
        {
            // Serialize
            size_t offset = 0;
            if (_settings.mode == DotStarMode::FixedBrightness)
            {
                for (const auto &color : colors)
                {
                    _byteBuffer[offset++] = 0xFF;
                    for (size_t channel = 0; channel < ChannelCount; ++channel)
                    {
                        _byteBuffer[offset++] = color[_settings.channelOrder[channel]];
                    }
                }
            }
            else // Luminance
            {
                for (const auto &color : colors)
                {
                    (void)color;
                    uint8_t lum = 31;
                    _byteBuffer[offset++] = 0xE0 | lum;
                    for (size_t channel = 0; channel < ChannelCount; ++channel)
                    {
                        _byteBuffer[offset++] = color[_settings.channelOrder[channel]];
                    }
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
        static constexpr size_t ChannelCount = ChannelOrder::LengthBGR;
        static constexpr size_t BytesPerPixel = 4;
        static constexpr size_t StartFrameSize = 4;
        static constexpr size_t EndFrameFixedSize = 4;

        DotStarProtocolSettings _settings;
        size_t _pixelCount;
        std::vector<uint8_t> _byteBuffer;
        size_t _endFrameExtraBytes;
    };

} // namespace npb
