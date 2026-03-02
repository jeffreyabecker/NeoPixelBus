#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <memory>
#include <vector>
#include <algorithm>
#include <type_traits>

#include <Arduino.h>

#include "IProtocol.h"
#include "transports/ITransport.h"

namespace lw
{

struct Lpd8806ProtocolSettings
{
    ITransport *bus = nullptr;
    const char* channelOrder = ChannelOrder::GRB::value;
};



// LPD8806 protocol.
//
// Wire format: 7-bit color with MSB set ? (value >> 1) | 0x80 per channel.
// Framing:
//   Start: ceil(N / 32) bytes of 0x00
//   Pixel data: 3 bytes per pixel
//   End:   ceil(N / 32) bytes of 0xFF
//
template <typename TInterfaceColor = Rgb8Color>
class Lpd8806ProtocolT : public IProtocol<TInterfaceColor>
{
public:
    using InterfaceColorType = TInterfaceColor;
    using StripColorType = Rgb8Color;
    using SettingsType = Lpd8806ProtocolSettings;

    static_assert((std::is_same<typename InterfaceColorType::ComponentType, uint8_t>::value ||
                   std::is_same<typename InterfaceColorType::ComponentType, uint16_t>::value),
                  "Lpd8806Protocol requires uint8_t or uint16_t interface components.");
    static_assert(InterfaceColorType::ChannelCount >= 3,
                  "Lpd8806Protocol requires at least 3 interface channels.");

    static size_t requiredBufferSize(uint16_t pixelCount,
                                     const SettingsType &)
    {
        const size_t frameSize = (static_cast<size_t>(pixelCount) + 31u) / 32u;
        return (frameSize * 2u) + (static_cast<size_t>(pixelCount) * BytesPerPixel);
    }

    Lpd8806ProtocolT(uint16_t pixelCount,
                     SettingsType settings)
        : IProtocol<InterfaceColorType>(pixelCount)
        , _settings{std::move(settings)}
        , _requiredBufferSize(requiredBufferSize(pixelCount, _settings))
        , _frameSize{(pixelCount + 31u) / 32u}
    {
    }

    void setBuffer(span<uint8_t> buffer) override
    {
        if (buffer.size() < _requiredBufferSize)
        {
            _byteBuffer = span<uint8_t>{};
            return;
        }

        _byteBuffer = span<uint8_t>{buffer.data(), _requiredBufferSize};
    }

    void bindTransport(ITransport *transport) override
    {
        _settings.bus = transport;
    }


    void initialize() override
    {
        if (_settings.bus == nullptr || _byteBuffer.size() != _requiredBufferSize)
        {
            return;
        }

        std::fill(_byteBuffer.begin(), _byteBuffer.begin() + _frameSize, 0x00);
        std::fill(_byteBuffer.end() - _frameSize, _byteBuffer.end(), 0xFF);
        _settings.bus->begin();
    }

    void update(span<const InterfaceColorType> colors) override
    {
        if (_settings.bus == nullptr || _byteBuffer.size() != _requiredBufferSize)
        {
            return;
        }

        // Serialize: 7-bit per channel with MSB set
        size_t offset = _frameSize;
        const size_t pixelLimit = std::min(colors.size(), static_cast<size_t>(this->pixelCount()));
        for (size_t index = 0; index < pixelLimit; ++index)
        {
            const auto& color = colors[index];
            for (size_t channel = 0; channel < BytesPerPixel; ++channel)
            {
                _byteBuffer[offset++] = (toWireComponent8(color[_settings.channelOrder[channel]]) >> 1) | 0x80;
            }
        }

        _settings.bus->beginTransaction();
        _settings.bus->transmitBytes(_byteBuffer);
        _settings.bus->endTransaction();
    }

    bool isReadyToUpdate() const override
    {
        if (_settings.bus == nullptr || _byteBuffer.size() != _requiredBufferSize)
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
    static constexpr size_t BytesPerPixel = ChannelOrder::GRB::length;

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
    span<uint8_t> _byteBuffer{};
    size_t _frameSize;
};

using Lpd8806Protocol = Lpd8806ProtocolT<Rgb8Color>;

} // namespace lw


