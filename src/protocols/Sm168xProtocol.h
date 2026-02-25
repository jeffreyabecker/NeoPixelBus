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

namespace npb
{

enum class Sm168xVariant : uint8_t
{
    ThreeChannel,
    FourChannel,
    FiveChannel
};

struct Sm168xProtocolSettings
{
    ResourceHandle<ITransport> bus;
    const char* channelOrder = ChannelOrder::RGB;
    Sm168xVariant variant = Sm168xVariant::ThreeChannel;
    std::array<uint8_t, 5> gains = {15, 15, 15, 15, 15};
};

template<typename TColor>
class Sm168xProtocol : public IProtocol<TColor>
{
public:
    using SettingsType = Sm168xProtocolSettings;
    using TransportCategory = TransportTag;

    static_assert(std::same_as<typename TColor::ComponentType, uint8_t>,
        "Sm168xProtocol requires 8-bit color components.");
    static_assert(TColor::ChannelCount >= 3 && TColor::ChannelCount <= 5,
        "Sm168xProtocol requires 3, 4, or 5 channels.");

    Sm168xProtocol(uint16_t pixelCount,
                  SettingsType settings)
        : _settings{std::move(settings)}
        , _channelCount{resolveChannelCount(_settings.variant)}
        , _settingsSize{resolveSettingsSize(_settings.variant)}
        , _frameBuffer(static_cast<size_t>(pixelCount) * _channelCount + _settingsSize, 0)
    {
    }

    void initialize() override
    {
        _settings.bus->begin();
    }

    void update(std::span<const TColor> colors) override
    {
        serializePixels(colors);
        encodeSettings();

        _settings.bus->beginTransaction();
        _settings.bus->transmitBytes(_frameBuffer);
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
    static constexpr size_t resolveChannelCount(Sm168xVariant variant)
    {
        switch (variant)
        {
        case Sm168xVariant::ThreeChannel:
            return 3;

        case Sm168xVariant::FourChannel:
            return 4;

        case Sm168xVariant::FiveChannel:
            return 5;
        }

        return 3;
    }

    static constexpr size_t resolveSettingsSize(Sm168xVariant variant)
    {
        switch (variant)
        {
        case Sm168xVariant::ThreeChannel:
            return 2;

        case Sm168xVariant::FourChannel:
            return 2;

        case Sm168xVariant::FiveChannel:
            return 4;
        }

        return 2;
    }

    uint8_t gainFromChannel(char channel) const
    {
        size_t idx = TColor::indexFromChannel(channel);
        idx = std::min(idx, _settings.gains.size() - 1);

        uint8_t gain = _settings.gains[idx];
        if (_settings.variant == Sm168xVariant::FiveChannel)
        {
            return static_cast<uint8_t>(gain & 0x1f);
        }

        return static_cast<uint8_t>(gain & 0x0f);
    }

    void serializePixels(std::span<const TColor> colors)
    {
        size_t offset = 0;
        const size_t payloadSize = _frameBuffer.size() - _settingsSize;
        std::fill(_frameBuffer.begin(), _frameBuffer.begin() + payloadSize, 0);

        const size_t maxPixels = (_channelCount == 0) ? 0 : (payloadSize / _channelCount);
        const size_t pixelLimit = std::min(colors.size(), maxPixels);
        for (size_t index = 0; index < pixelLimit; ++index)
        {
            const auto& color = colors[index];
            for (size_t channel = 0; channel < _channelCount; ++channel)
            {
                _frameBuffer[offset++] = color[_settings.channelOrder[channel]];
            }
        }
    }

    void encodeSettings()
    {
        uint8_t ic[5] = {0, 0, 0, 0, 0};
        for (size_t channel = 0; channel < _channelCount; ++channel)
        {
            ic[channel] = gainFromChannel(_settings.channelOrder[channel]);
        }

        uint8_t* encoded = _frameBuffer.data() + (_frameBuffer.size() - _settingsSize);

        switch (_settings.variant)
        {
        case Sm168xVariant::ThreeChannel:
            encoded[0] = ic[0];
            encoded[1] = static_cast<uint8_t>((ic[1] << 4) | ic[2]);
            break;

        case Sm168xVariant::FourChannel:
            encoded[0] = static_cast<uint8_t>((ic[0] << 4) | ic[1]);
            encoded[1] = static_cast<uint8_t>((ic[2] << 4) | ic[3]);
            break;

        case Sm168xVariant::FiveChannel:
            encoded[0] = static_cast<uint8_t>((ic[0] << 3) | (ic[1] >> 2));
            encoded[1] = static_cast<uint8_t>((ic[1] << 6) | (ic[2] << 1) | (ic[3] >> 4));
            encoded[2] = static_cast<uint8_t>((ic[3] << 4) | (ic[4] >> 1));
            encoded[3] = static_cast<uint8_t>((ic[4] << 7) | 0b00011111);
            break;
        }
    }

    SettingsType _settings;
    size_t _channelCount;
    size_t _settingsSize;
    std::vector<uint8_t> _frameBuffer;
};

using Sm168xRgbcwProtocol = Sm168xProtocol<Rgbcw8Color>;

} // namespace npb


