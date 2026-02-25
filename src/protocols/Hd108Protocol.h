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
#include "transports/ITransport.h"
#include "core/ResourceHandle.h"
#include "colors/Color.h"

namespace npb
{

    struct Hd108ProtocolSettings
    {
        ResourceHandle<ITransport> bus;
        const char *channelOrder = ChannelOrder::BGR;
    };



    // I cant actually find any documentation on the HD108 protocol,
    // I've found the RGB, RGBW, and RGBW+C variants in the wild but not purchased to test
    // However the RGB+C variant uses separate chips for the RGB, W, and C channels, so it seems likely that the RGBW+C variant 
    // is actually an RGB variant with 

    // HD108 protocol.
    //
    // Wire format per pixel: 8 bytes
    //   [2-byte prefix] [ch1 hi][ch1 lo] [ch2 hi][ch2 lo] [ch3 hi][ch3 lo]
    //
    // Prefix: 0xFFFF (all brightness bits max, upper bit always 1)
    //   Layout: {1}{5-bit brightness ch1}{5-bit brightness ch2}{5-bit brightness ch3}
    //   At max brightness ? 0xFFFF.
    //
    // Channels are 16-bit big-endian, expanded from 8-bit via byte replication.
    //
    // Framing:
    //   Start: 16 x 0x00
    //   End:    4 x 0xFF
    //
    template <typename TColor>
        requires(std::same_as<typename TColor::ComponentType, uint16_t> &&
                 (TColor::ChannelCount >= 3))
    class Hd108Protocol : public IProtocol<TColor>
    {
    public:
        using SettingsType = Hd108ProtocolSettings;
        using TransportCategory = TransportTag;

        Hd108Protocol(uint16_t pixelCount,
                  SettingsType settings)
            : _settings{std::move(settings)}, _pixelCount{pixelCount}, _byteBuffer(pixelCount * BytesPerPixel)
        {
        }


        void initialize() override
        {
            _settings.bus->begin();
        }

        void update(std::span<const TColor> colors) override
        {
            // Serialize: 16-bit per channel, big-endian
            size_t offset = 0;
            const size_t pixelLimit = std::min(colors.size(), _pixelCount);
            for (size_t index = 0; index < pixelLimit; ++index)
            {
                const auto &color = colors[index];
                // Prefix: all brightness bits max
                _byteBuffer[offset++] = 0xFF;
                _byteBuffer[offset++] = 0xFF;

                // Channel data is 16-bit in protocol byte order
                for (size_t ch = 0; ch < ChannelCount; ++ch)
                {
                    uint16_t val = color[_settings.channelOrder[ch]];
                    _byteBuffer[offset++] = static_cast<uint8_t>(val >> 8);
                    _byteBuffer[offset++] = static_cast<uint8_t>(val & 0xFF);
                }
            }

            _settings.bus->beginTransaction();

            const uint8_t zeroByte = 0x00;
            const std::span<const uint8_t> zeroSpan{&zeroByte, 1};
            const uint8_t ffByte = 0xFF;
            const std::span<const uint8_t> ffSpan{&ffByte, 1};

            // Start frame: 16 x 0x00
            for (size_t i = 0; i < StartFrameSize; ++i)
            {
                _settings.bus->transmitBytes(zeroSpan);
            }

            // Pixel data
            _settings.bus->transmitBytes(_byteBuffer);

            // End frame: 4 x 0xFF
            for (size_t i = 0; i < EndFrameSize; ++i)
            {
                _settings.bus->transmitBytes(ffSpan);
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
        static constexpr size_t ChannelCount = TColor::ChannelCount;
        static constexpr size_t BytesPerPixel = 2 + (ChannelCount * 2);
        static constexpr size_t StartFrameSize = 16;
        static constexpr size_t EndFrameSize = 4;

        SettingsType _settings;
        size_t _pixelCount;
        std::vector<uint8_t> _byteBuffer;
    };

    using Hd108RgbProtocol = Hd108Protocol<Rgb16Color>;
    using Hd108RgbcwProtocol = Hd108Protocol<Rgbcw16Color>;

} // namespace npb


