#pragma once

#include <cstdint>
#include <cstddef>
#include <memory>
#include <vector>
#include <algorithm>

#include <Arduino.h>

#include "IProtocol.h"
#include "transports/ITransport.h"
#include "transports/OneWireTiming.h"

namespace lw
{

enum class Tm1914Mode : uint8_t
{
    DinFdinAutoSwitch,
    DinOnly,
    FdinOnly
};

struct Tm1914ProtocolSettings
{
    ITransport *bus = nullptr;
    const char* channelOrder = ChannelOrder::GRB::value;
    OneWireTiming timing = timing::Tm1914;
    Tm1914Mode mode = Tm1914Mode::DinOnly;
};

class Tm1914Protocol : public IProtocol<Rgb8Color>
{
public:
    using SettingsType = Tm1914ProtocolSettings;
    using TransportCategory = OneWireTransportTag;

    static size_t requiredBufferSize(uint16_t pixelCount,
                                     const SettingsType &)
    {
        return SettingsSize + (static_cast<size_t>(pixelCount) * ChannelCount);
    }

    Tm1914Protocol(uint16_t pixelCount,
                   SettingsType settings)
        : IProtocol<Rgb8Color>(pixelCount)
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

    void update(span<const Rgb8Color> colors) override
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
    static constexpr size_t ChannelCount = 3;
    static constexpr size_t SettingsSize = 6;

    uint8_t encodedMode() const
    {
        switch (_settings.mode)
        {
        case Tm1914Mode::DinFdinAutoSwitch:
            return 0xff;

        case Tm1914Mode::FdinOnly:
            return 0xfa;

        case Tm1914Mode::DinOnly:
        default:
            return 0xf5;
        }
    }

    void encodeSettings()
    {
        _frameBuffer[0] = 0xff;
        _frameBuffer[1] = 0xff;
        _frameBuffer[2] = encodedMode();

        _frameBuffer[3] = static_cast<uint8_t>(~_frameBuffer[0]);
        _frameBuffer[4] = static_cast<uint8_t>(~_frameBuffer[1]);
        _frameBuffer[5] = static_cast<uint8_t>(~_frameBuffer[2]);
    }

    void serializePixels(span<const Rgb8Color> colors)
    {
        size_t offset = SettingsSize;
        const size_t pixelLimit = std::min(colors.size(), static_cast<size_t>(this->pixelCount()));
        for (size_t index = 0; index < pixelLimit; ++index)
        {
            const auto& color = colors[index];
            for (size_t channel = 0; channel < ChannelCount; ++channel)
            {
                _frameBuffer[offset++] = color[_settings.channelOrder[channel]];
            }
        }
    }

    SettingsType _settings;
    size_t _requiredBufferSize{0};
    span<uint8_t> _frameBuffer{};
};

} // namespace lw


