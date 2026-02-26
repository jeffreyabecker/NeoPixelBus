#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <memory>
#include <vector>
#include <algorithm>

#include <Arduino.h>

#include "IProtocol.h"
#include "transports/ITransport.h"
#include "colors/Color.h"

namespace npb
{

    // DotStar / APA102 brightness modes.
    //
    //   FixedBrightness ? 0xFF prefix byte, W channel ignored
    //   Luminance       ? 0xE0 | luminance prefix (Rgb8Color has no W channel)
    //
    enum class DotStarMode : uint8_t
    {
        FixedBrightness,
        Luminance,
    };

    struct DotStarProtocolSettings
    {
        ITransport *bus = nullptr;
        const char* channelOrder = ChannelOrder::BGR;
        DotStarMode mode = DotStarMode::FixedBrightness;
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
        using SettingsType = DotStarProtocolSettings;
        using TransportCategory = TransportTag;

        DotStarProtocol(uint16_t pixelCount,
                   SettingsType settings)
            : IProtocol<Rgb8Color>(pixelCount), _settings{std::move(settings)}, _byteBuffer(pixelCount * BytesPerPixel), _endFrameExtraBytes{(pixelCount + 15u) / 16u}
        {
        }


        void initialize() override
        {
            _settings.bus->begin();
        }

        void update(span<const Rgb8Color> colors) override
        {
            // Serialize
            size_t offset = 0;
            const size_t pixelLimit = std::min(colors.size(), static_cast<size_t>(this->pixelCount()));
            if (_settings.mode == DotStarMode::FixedBrightness)
            {
                for (size_t index = 0; index < pixelLimit; ++index)
                {
                    const auto &color = colors[index];
                    _byteBuffer[offset++] = 0xFF;
                    for (size_t channel = 0; channel < ChannelCount; ++channel)
                    {
                        _byteBuffer[offset++] = color[_settings.channelOrder[channel]];
                    }
                }
            }
            else // Luminance
            {
                for (size_t index = 0; index < pixelLimit; ++index)
                {
                    const auto &color = colors[index];
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
            const span<const uint8_t> zeroSpan{&zeroByte, 1};

            // Start frame: 4 x 0x00
            for (size_t i = 0; i < StartFrameSize; ++i)
            {
                _settings.bus->transmitBytes(zeroSpan);
            }

            // Pixel data
            _settings.bus->transmitBytes(span<const uint8_t>(_byteBuffer.data(), _byteBuffer.size()));

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

        SettingsType _settings;
        std::vector<uint8_t> _byteBuffer;
        size_t _endFrameExtraBytes;
    };

} // namespace npb


