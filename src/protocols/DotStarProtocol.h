#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <vector>

#include <Arduino.h>

#include "IProtocol.h"
#include "transports/ITransport.h"
#include "colors/Color.h"
#include "protocols/serializers/GeneralDotStarSerializer.h"

namespace npb
{
    struct DotStarProtocolSettings
    {
        ITransport *bus = nullptr;
        const char *channelOrder = ChannelOrder::BGR;
    };

    // DotStar / APA102 protocol.
    //
    // Wire format per pixel: [prefix] [ch1] [ch2] [ch3]  (4 bytes)
    // Framing:
    //   Start: 4 x 0x00
    //   End:   4 x 0x00 + ceil(N/16) x 0x00
    //
    template <typename TColor = Rgb8Color,
              typename TSerializer = GeneralDotStarSerializer<TColor>>
    class DotStarProtocolT : public IProtocol<TColor>
    {
    public:
        using SettingsType = DotStarProtocolSettings;
        using TransportCategory = TransportTag;
        using ColorType = TColor;
        using SerializerType = TSerializer;

        static_assert(std::is_same<typename ColorType::ComponentType, uint8_t>::value,
                      "DotStarProtocol requires uint8_t color components.");
        static_assert(ColorType::ChannelCount >= 3 && ColorType::ChannelCount <= 5,
                      "DotStarProtocol requires color channel count in [3, 5].");

        DotStarProtocolT(uint16_t pixelCount,
                         SettingsType settings)
                        : IProtocol<ColorType>(pixelCount),
                          _settings{std::move(settings)},
                          _byteBuffer(SerializerType::getBufferSize(pixelCount), 0)
        {
        }


        void initialize() override
        {
            SerializerType::initialize(span<uint8_t>{_byteBuffer.data(), _byteBuffer.size()},
                                       this->pixelCount());
            _settings.bus->begin();
        }

        void update(span<const ColorType> colors) override
        {
            SerializerType::serialize(span<uint8_t>{_byteBuffer.data(), _byteBuffer.size()},
                                      colors,
                                      this->pixelCount(),
                                      _settings.channelOrder);

            _settings.bus->beginTransaction();
            _settings.bus->transmitBytes(span<uint8_t>(_byteBuffer.data(), _byteBuffer.size()));
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
        SettingsType _settings;
        std::vector<uint8_t> _byteBuffer;
    };

    using DotStarProtocol = DotStarProtocolT<Rgb8Color>;


} // namespace npb


