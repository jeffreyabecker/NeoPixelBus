#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>

#include <Arduino.h>

#include "IProtocol.h"
#include "transports/ITransport.h"
#include "colors/Color.h"
#include "protocols/serializers/GeneralDotStarSerializer.h"
#include "protocols/serializers/Hd108Serializer.h"

namespace lw
{
    struct DotStarProtocolSettings
    {
        ITransport *bus = nullptr;
        const char *channelOrder = ChannelOrder::BGR::value;
    };

    template <typename TInterfaceColor,
              typename TStripColor,
              typename TSerializer>
    class DotStarProtocolBaseT : public IProtocol<TInterfaceColor>
    {
    public:
        using SettingsType = DotStarProtocolSettings;
        using TransportCategory = TransportTag;
        using InterfaceColorType = TInterfaceColor;
        using StripColorType = TStripColor;
        using SerializerType = TSerializer;

        static_assert((std::is_same<typename InterfaceColorType::ComponentType, uint8_t>::value ||
                       std::is_same<typename InterfaceColorType::ComponentType, uint16_t>::value),
                      "DotStarProtocol interface color supports uint8_t or uint16_t components.");
        static_assert((std::is_same<typename StripColorType::ComponentType, uint8_t>::value ||
                       std::is_same<typename StripColorType::ComponentType, uint16_t>::value),
                      "DotStarProtocol strip color supports uint8_t or uint16_t components.");
        static_assert(InterfaceColorType::ChannelCount >= 3 && InterfaceColorType::ChannelCount <= 5,
                      "DotStarProtocol interface color requires channel count in [3, 5].");
        static_assert(StripColorType::ChannelCount >= 3 && StripColorType::ChannelCount <= 5,
                      "DotStarProtocol strip color requires channel count in [3, 5].");

        static size_t requiredBufferSize(uint16_t pixelCount,
                                         const SettingsType &)
        {
            return SerializerType::getBufferSize(pixelCount);
        }

        DotStarProtocolBaseT(uint16_t pixelCount,
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

            SerializerType::initialize(_byteBuffer);
            _settings.bus->begin();
        }

        void update(span<const InterfaceColorType> colors) override
        {
            if (_settings.bus == nullptr || _byteBuffer.size() != _requiredBufferSize)
            {
                return;
            }

            SerializerType::serialize(_byteBuffer,
                                      colors,
                                      this->pixelCount(),
                                      _settings.channelOrder);

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
        SettingsType _settings;
        size_t _requiredBufferSize{0};
        span<uint8_t> _byteBuffer{};
    };

    template <typename TInterfaceColor = Rgb8Color,
              typename TStripColor = TInterfaceColor>
    class Apa102ProtocolT : public DotStarProtocolBaseT<TInterfaceColor,
                                                         TStripColor,
                                                         GeneralDotStarSerializer<TStripColor>>
    {
    public:
        using BaseType = DotStarProtocolBaseT<TInterfaceColor,
                                              TStripColor,
                                              GeneralDotStarSerializer<TStripColor>>;
        using SettingsType = typename BaseType::SettingsType;

        static_assert(std::is_same<typename TStripColor::ComponentType, uint8_t>::value,
                      "Apa102Protocol requires uint8_t strip components.");

        Apa102ProtocolT(uint16_t pixelCount,
                        SettingsType settings)
            : BaseType(pixelCount, std::move(settings))
        {
        }
    };

    template <typename TInterfaceColor = Rgb8Color,
              typename TStripColor = Rgb16Color>
    class Hd108ProtocolT : public DotStarProtocolBaseT<TInterfaceColor,
                                                        TStripColor,
                                                        Hd108Serializer<TStripColor>>
    {
    public:
        using BaseType = DotStarProtocolBaseT<TInterfaceColor,
                                              TStripColor,
                                              Hd108Serializer<TStripColor>>;
        using SettingsType = typename BaseType::SettingsType;

        static_assert(std::is_same<typename TStripColor::ComponentType, uint16_t>::value,
                      "Hd108Protocol requires uint16_t strip components.");

        Hd108ProtocolT(uint16_t pixelCount,
                       SettingsType settings)
            : BaseType(pixelCount, std::move(settings))
        {
        }
    };

    using DotStarProtocol = Apa102ProtocolT<Rgb8Color>;
    using Apa102Protocol = Apa102ProtocolT<Rgb8Color>;
    using Hd108Protocol = Hd108ProtocolT<Rgb8Color, Rgb16Color>;

    template <typename TInterfaceColor = Rgb8Color,
              typename TStripColor = TInterfaceColor>
    using DotStarProtocolT = std::conditional_t<
        std::is_same<typename TStripColor::ComponentType, uint16_t>::value,
        Hd108ProtocolT<TInterfaceColor, TStripColor>,
        Apa102ProtocolT<TInterfaceColor, TStripColor>>;

} // namespace lw
