#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <vector>
#include <algorithm>
#include <type_traits>

#include <Arduino.h>

#include "IProtocol.h"
#include "transports/ITransport.h"
#include "transports/OneWireTiming.h"

namespace lw
{

struct Tm1814CurrentSettings
{
    uint16_t redMilliAmps = 190;
    uint16_t greenMilliAmps = 190;
    uint16_t blueMilliAmps = 190;
    uint16_t whiteMilliAmps = 190;
};

struct Tm1814ProtocolSettings
{
    ITransport *bus = nullptr;
    const char* channelOrder = "WRGB";
    OneWireTiming timing = timing::Tm1814;
    Tm1814CurrentSettings current{};
};


template <typename TInterfaceColor = Rgbw8Color>
class Tm1814ProtocolT : public IProtocol<TInterfaceColor>
{
public:
    using InterfaceColorType = TInterfaceColor;
    using StripColorType = Rgbw8Color;
    using SettingsType = Tm1814ProtocolSettings;
    using TransportCategory = OneWireTransportTag;

    static_assert((std::is_same<typename InterfaceColorType::ComponentType, uint8_t>::value ||
                   std::is_same<typename InterfaceColorType::ComponentType, uint16_t>::value),
                  "Tm1814Protocol requires uint8_t or uint16_t interface components.");
    static_assert(InterfaceColorType::ChannelCount >= 4,
                  "Tm1814Protocol requires at least 4 interface channels.");

    static size_t requiredBufferSize(uint16_t pixelCount,
                                     const SettingsType &)
    {
        return SettingsSize + (static_cast<size_t>(pixelCount) * ChannelCount);
    }

    Tm1814ProtocolT(uint16_t pixelCount,
                    SettingsType settings)
        : IProtocol<InterfaceColorType>(pixelCount)
        , _settings{std::move(settings)}
        , _requiredBufferSize(requiredBufferSize(pixelCount, _settings))
    {
    }

    void setBuffer(span<uint8_t> buffer) override
    {
        if (buffer.size() < _requiredBufferSize)
        {
            _frameBuffer = span<uint8_t>{};
            return;
        }

        _frameBuffer = span<uint8_t>{buffer.data(), _requiredBufferSize};
    }

    void bindTransport(ITransport *transport) override
    {
        _settings.bus = transport;
    }

    void initialize() override
    {
        if (_settings.bus == nullptr || _frameBuffer.size() != _requiredBufferSize)
        {
            return;
        }

        _settings.bus->begin();
    }

    void update(span<const InterfaceColorType> colors) override
    {
        if (_settings.bus == nullptr || _frameBuffer.size() != _requiredBufferSize)
        {
            return;
        }

        while (!_settings.bus->isReadyToUpdate())
        {
            yield();
        }

        encodeSettings();
        serializePixels(colors);

        _settings.bus->beginTransaction();
        _settings.bus->transmitBytes(_frameBuffer);
        _settings.bus->endTransaction();
    }

    bool isReadyToUpdate() const override
    {
        if (_settings.bus == nullptr || _frameBuffer.size() != _requiredBufferSize)
        {
            return false;
        }

        return _settings.bus->isReadyToUpdate();
    }

    bool alwaysUpdate() const override
    {
        return false;
    }

    size_t requiredBufferSizeBytes() const override
    {
        return _requiredBufferSize;
    }

private:
    static constexpr size_t ChannelCount = 4;
    static constexpr size_t SettingsSize = 8;
    static constexpr uint16_t MinCurrent = 65;
    static constexpr uint16_t MaxCurrent = 380;
    static constexpr uint16_t EncodeDivisor = 5;

    static uint8_t encodeCurrent(uint16_t milliAmps)
    {
        const uint16_t limited = std::clamp(milliAmps, MinCurrent, MaxCurrent);
        return static_cast<uint8_t>((limited - MinCurrent) / EncodeDivisor);
    }

    uint8_t currentForChannel(char channel) const
    {
        switch (channel)
        {
        case 'R':
        case 'r':
            return encodeCurrent(_settings.current.redMilliAmps);

        case 'G':
        case 'g':
            return encodeCurrent(_settings.current.greenMilliAmps);

        case 'B':
        case 'b':
            return encodeCurrent(_settings.current.blueMilliAmps);

        case 'W':
        case 'w':
        default:
            return encodeCurrent(_settings.current.whiteMilliAmps);
        }
    }

    void encodeSettings()
    {
        for (size_t i = 0; i < ChannelCount; ++i)
        {
            _frameBuffer[i] = currentForChannel(_settings.channelOrder[i]);
        }

        for (size_t i = 0; i < ChannelCount; ++i)
        {
            _frameBuffer[ChannelCount + i] = static_cast<uint8_t>(~_frameBuffer[i]);
        }
    }

    void serializePixels(span<const InterfaceColorType> colors)
    {
        size_t offset = SettingsSize;
        const size_t pixelLimit = std::min(colors.size(), static_cast<size_t>(this->pixelCount()));
        for (size_t index = 0; index < pixelLimit; ++index)
        {
            const auto& color = colors[index];
            for (size_t channel = 0; channel < ChannelCount; ++channel)
            {
                _frameBuffer[offset++] = toWireComponent8(color[_settings.channelOrder[channel]]);
            }
        }
    }

    static constexpr uint8_t toWireComponent8(typename InterfaceColorType::ComponentType value)
    {
        if constexpr (std::is_same<typename InterfaceColorType::ComponentType, uint8_t>::value)
        {
            return value;
        }

        return static_cast<uint8_t>(value >> 8);
    }

    SettingsType _settings;
    size_t _requiredBufferSize{0};
    span<uint8_t> _frameBuffer{};
};

using Tm1814Protocol = Tm1814ProtocolT<Rgbw8Color>;

} // namespace lw


