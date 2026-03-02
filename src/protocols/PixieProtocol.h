#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <vector>
#include <type_traits>

#include <Arduino.h>

#include "IProtocol.h"
#include "transports/ITransport.h"

namespace lw
{

    struct PixieProtocolSettings
    {
        ITransport *bus = nullptr;
        const char *channelOrder = ChannelOrder::RGB::value;
    };

    template <typename TInterfaceColor = Rgb8Color>
    class PixieProtocolT : public IProtocol<TInterfaceColor>
    {
    public:
        using InterfaceColorType = TInterfaceColor;
        using StripColorType = Rgb8Color;
        using SettingsType = PixieProtocolSettings;
        using TransportCategory = OneWireTransportTag;

        static_assert((std::is_same<typename InterfaceColorType::ComponentType, uint8_t>::value ||
                       std::is_same<typename InterfaceColorType::ComponentType, uint16_t>::value),
                      "PixieProtocol requires uint8_t or uint16_t interface components.");
        static_assert(InterfaceColorType::ChannelCount >= 3,
                      "PixieProtocol requires at least 3 interface channels.");

        static size_t requiredBufferSize(uint16_t pixelCount,
                                         const SettingsType &)
        {
            return static_cast<size_t>(pixelCount) * BytesPerPixel;
        }

        PixieProtocolT(uint16_t pixelCount,
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

            _settings.bus->begin();
        }

        void update(span<const InterfaceColorType> colors) override
        {
            if (_settings.bus == nullptr || _byteBuffer.size() != _requiredBufferSize)
            {
                return;
            }

            while (!isReadyToUpdate())
            {
                yield();
            }

            size_t offset = 0;
            const size_t pixelLimit = std::min(colors.size(), static_cast<size_t>(this->pixelCount()));
            for (size_t index = 0; index < pixelLimit; ++index)
            {
                const auto &color = colors[index];
                for (size_t channel = 0; channel < BytesPerPixel; ++channel)
                {
                    _byteBuffer[offset++] = toWireComponent8(color[_settings.channelOrder[channel]]);
                }
            }

            _settings.bus->beginTransaction();
            _settings.bus->transmitBytes(_byteBuffer);
            _settings.bus->endTransaction();

            _endTime = micros();
        }

        bool isReadyToUpdate() const override
        {
            if (_settings.bus == nullptr || _byteBuffer.size() != _requiredBufferSize)
            {
                return false;
            }

            return _settings.bus->isReadyToUpdate() && ((micros() - _endTime) >= LatchDelayUs);
        }

        bool alwaysUpdate() const override
        {
            return true;
        }

        size_t requiredBufferSizeBytes() const override
        {
            return _requiredBufferSize;
        }

    private:
        static constexpr size_t BytesPerPixel = ChannelOrder::RGB::length;
        static constexpr uint32_t LatchDelayUs = 1000;

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
        uint32_t _endTime{0};
    };

    using PixieProtocol = PixieProtocolT<Rgb8Color>;

} // namespace lw


