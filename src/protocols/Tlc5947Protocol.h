#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <memory>
#include <vector>
#include <algorithm>

#include <Arduino.h>

#include "IProtocol.h"
#include "transports/ITransport.h"

namespace npb
{

    static constexpr int8_t PinNotUsed = -1;

    enum class Tlc5947PixelStrategy : uint8_t
    {
        UseColorChannelCount,
        ForceRgb,
        ForceRgbw,
        ForceRgbcw
    };

    enum class Tlc5947TailFillStrategy : uint8_t
    {
        Zero,
        RepeatFirstPixel,
        RepeatLastPixel
    };

    struct Tlc5947ProtocolSettings
    {
        ITransport *bus = nullptr;
        int8_t latchPin;
        int8_t oePin = PinNotUsed;
        const char *channelOrder = ChannelOrder::RGB;
        Tlc5947PixelStrategy pixelStrategy = Tlc5947PixelStrategy::UseColorChannelCount;
        Tlc5947TailFillStrategy tailFillStrategy = Tlc5947TailFillStrategy::Zero;
    };

    // TLC5947 protocol.
    //
    // SPI-like two-wire (clock + data), transport-managed control signaling.
    // 24 PWM channels per module (= 8 RGB pixels per module).
    // 12-bit per channel on the wire.
    //
    // No in-band settings ? pure channel data.
    //
    // Transmission order:
    //   - Within each module, channels are sent in REVERSE order
    //   - 16-bit input is narrowed to 12-bit (value >> 4)
    //   - Two 12-bit channels are packed into 3 bytes
    //
    // Control signaling (latch/OE or equivalent) is transport-specific.
    // The transport implementation is responsible for any required timing and
    // bit-level twiddling across begin()/beginTransaction()/endTransaction().
    // TODO: introduce a dedicated TLC5947 transport contract (or transport config
    // capability flag set) for deterministic latch/OE sequencing under DMA-backed
    // SPI controllers, so protocol behavior remains portable without protocol-side
    // GPIO manipulation.
    //
    template <typename TColor>
    class Tlc5947Protocol : public IProtocol<TColor>
    {
    public:
        using SettingsType = Tlc5947ProtocolSettings;
        using TransportCategory = TransportTag;

        static_assert(std::is_same<typename TColor::ComponentType, uint16_t>::value,
                      "Tlc5947Protocol requires 16-bit color components.");
        static_assert(TColor::ChannelCount >= 3 && TColor::ChannelCount <= 5,
                      "Tlc5947Protocol expects 3, 4, or 5 channels from the input color type.");

        Tlc5947Protocol(uint16_t pixelCount,
                SettingsType settings)
            : IProtocol<TColor>(pixelCount), _settings{std::move(settings)}, _pixelStrategy{_settings.pixelStrategy}, _tailFillStrategy{_settings.tailFillStrategy}, _activeChannelCount{resolveActiveChannelCount()}, _pixelsPerModule{ChannelsPerModule / _activeChannelCount}, _moduleCount{(pixelCount + _pixelsPerModule - 1) / _pixelsPerModule}, _byteBuffer(_moduleCount * BytesPerModule)
        {
        }

        void initialize() override
        {
            _settings.bus->begin();
        }

        void update(span<const TColor> colors) override
        {
            // Serialize: 12-bit channels, reversed order within each module
            serialize(colors);

            _settings.bus->beginTransaction();
            _settings.bus->transmitBytes(span<uint8_t>(_byteBuffer.data(), _byteBuffer.size()));
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
        static constexpr size_t ChannelsPerModule = 24;
        static constexpr size_t BytesPerModule = 36; // 24 ? 12 bits / 8

        SettingsType _settings;
        Tlc5947PixelStrategy _pixelStrategy;
        Tlc5947TailFillStrategy _tailFillStrategy;
        size_t _activeChannelCount;
        size_t _pixelsPerModule;
        size_t _moduleCount;
        std::vector<uint8_t> _byteBuffer;

        static constexpr char defaultChannelForIndex(size_t channel)
        {
            switch (channel)
            {
            case 0:
                return 'R';

            case 1:
                return 'G';

            case 2:
                return 'B';

            case 3:
                return 'W';

            case 4:
                return 'C';
            }

            return 'R';
        }

        size_t resolveActiveChannelCount() const
        {
            switch (_pixelStrategy)
            {
            case Tlc5947PixelStrategy::UseColorChannelCount:
                return TColor::ChannelCount;

            case Tlc5947PixelStrategy::ForceRgb:
                return 3;

            case Tlc5947PixelStrategy::ForceRgbw:
                return std::min<size_t>(4, TColor::ChannelCount);

            case Tlc5947PixelStrategy::ForceRgbcw:
                return std::min<size_t>(5, TColor::ChannelCount);
            }

            return TColor::ChannelCount;
        }

        char channelAt(size_t channel) const
        {
            if (_settings.channelOrder != nullptr && _settings.channelOrder[channel] != '\0')
            {
                return _settings.channelOrder[channel];
            }

            return defaultChannelForIndex(channel);
        }

        static constexpr uint16_t to12Bit(uint16_t value)
        {
            return static_cast<uint16_t>((value >> 4) & 0x0FFF);
        }

        void writePixelChannels(const TColor &color,
                                uint16_t *channels,
                                size_t channelOffset) const
        {
            for (size_t channel = 0; channel < _activeChannelCount; ++channel)
            {
                const char mappedChannel = channelAt(channel);
                channels[channelOffset + channel] = to12Bit(color[mappedChannel]);
            }
        }

        void fillTailChannels(uint16_t *channels,
                              size_t usedChannels,
                              size_t modStartPixel,
                              span<const TColor> colors) const
        {
            if (usedChannels >= ChannelsPerModule || _tailFillStrategy == Tlc5947TailFillStrategy::Zero)
            {
                return;
            }

            size_t sourcePixelIndex = modStartPixel;
            if (_tailFillStrategy == Tlc5947TailFillStrategy::RepeatLastPixel)
            {
                const size_t lastPixelInModule = modStartPixel + _pixelsPerModule - 1;
                sourcePixelIndex = std::min(lastPixelInModule, colors.size() - 1);
            }

            if (sourcePixelIndex >= colors.size())
            {
                return;
            }

            const TColor &sourcePixel = colors[sourcePixelIndex];
            size_t tailOffset = usedChannels;
            while (tailOffset < ChannelsPerModule)
            {
                const size_t channelsToWrite = std::min(_activeChannelCount, ChannelsPerModule - tailOffset);
                for (size_t channel = 0; channel < channelsToWrite; ++channel)
                {
                    const char mappedChannel = channelAt(channel);
                    channels[tailOffset + channel] = to12Bit(sourcePixel[mappedChannel]);
                }
                tailOffset += channelsToWrite;
            }
        }

        void serialize(span<const TColor> colors)
        {
            size_t bufOffset = 0;

            for (size_t mod = 0; mod < _moduleCount; ++mod)
            {
                size_t modStartPixel = mod * _pixelsPerModule;

                // Collect 24 channels in forward order, then reverse
                uint16_t channels[ChannelsPerModule]{};
                size_t usedChannels = 0;

                for (size_t px = 0; px < _pixelsPerModule; ++px)
                {
                    size_t pixelIdx = modStartPixel + px;
                    size_t chBase = px * _activeChannelCount;

                    if (chBase + _activeChannelCount > ChannelsPerModule)
                    {
                        break;
                    }

                    if (pixelIdx < colors.size())
                    {
                        writePixelChannels(colors[pixelIdx], channels, chBase);
                        usedChannels = chBase + _activeChannelCount;
                    }
                }

                fillTailChannels(channels, usedChannels, modStartPixel, colors);

                // Pack 12-bit channels in REVERSE order, 2 channels per 3 bytes
                for (size_t i = ChannelsPerModule; i >= 2; i -= 2)
                {
                    uint16_t ch1 = channels[i - 2]; // earlier channel
                    uint16_t ch2 = channels[i - 1]; // later channel

                    _byteBuffer[bufOffset++] = static_cast<uint8_t>(ch2 >> 4);
                    _byteBuffer[bufOffset++] = static_cast<uint8_t>(
                        ((ch2 & 0x0F) << 4) | (ch1 >> 8));
                    _byteBuffer[bufOffset++] = static_cast<uint8_t>(ch1 & 0xFF);
                }
            }
        }
    };

} // namespace npb


