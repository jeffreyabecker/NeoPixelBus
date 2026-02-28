#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

#include "factory/busses/BusDriverConstraints.h"
#include "core/IPixelBus.h"
#include "core/BufferHolder.h"

namespace npb
{

    template <typename TTransport,
              typename TProtocol,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TTransport>>>
    class StaticBusDriverPixelBusT
        : public IAssignableBufferBus<typename TProtocol::ColorType>
    {
    public:
        using ColorType = typename TProtocol::ColorType;
        using ProtocolSettingsType = typename TProtocol::SettingsType;
        using TransportSettingsType = typename TTransport::TransportSettingsType;

        StaticBusDriverPixelBusT(uint16_t pixelCount,
                                 TransportSettingsType transportSettings,
                                 ProtocolSettingsType settings)
            : _transport(std::move(transportSettings))
            , _protocol(makeProtocol(pixelCount, _transport, std::move(settings)))
            , _pixelCount(pixelCount)
            , _colors(pixelCount, nullptr, true)
        {
        }

        void begin() override
        {
            _colors.init();
            _protocol.initialize();
        }

        void show() override
        {
            if (!_dirty && !_protocol.alwaysUpdate())
            {
                return;
            }

            _protocol.update(_colors.getSpan(0, _colors.size));
            _dirty = false;
        }

        bool canShow() const override
        {
            return _protocol.isReadyToUpdate();
        }

        void setBuffer(span<ColorType> buffer) override
        {
            _colors = BufferHolder<ColorType>{buffer.size(), buffer.data(), false};
            _dirty = true;
        }

        uint16_t pixelCount() const override
        {
            return _pixelCount;
        }

        span<ColorType> pixelBuffer() override
        {
            _dirty = true;
            return _colors.getSpan(0, _colors.size);
        }

        span<const ColorType> pixelBuffer() const override
        {
            return _colors.getSpan(0, _colors.size);
        }

        span<ColorType> colors()
        {
            return pixelBuffer();
        }

        span<const ColorType> colors() const
        {
            return pixelBuffer();
        }

        TTransport &transport()
        {
            return _transport;
        }

        const TTransport &transport() const
        {
            return _transport;
        }

        TProtocol &protocol()
        {
            return _protocol;
        }

        const TProtocol &protocol() const
        {
            return _protocol;
        }

    private:
        static TProtocol makeProtocol(uint16_t pixelCount,
                                      TTransport &transport,
                                      ProtocolSettingsType settings)
        {
            if constexpr (ProtocolSettingsTransportBindable<TProtocol>)
            {
                settings.bus = &transport;
                return TProtocol(pixelCount, std::move(settings));
            }
            else if constexpr (std::is_constructible<TProtocol,
                                                     uint16_t,
                                                     ProtocolSettingsType,
                                                     TTransport &>::value)
            {
                return TProtocol(pixelCount, std::move(settings), transport);
            }
            else
            {
                return TProtocol(pixelCount, std::move(settings));
            }
        }

        TTransport _transport;
        TProtocol _protocol;
        uint16_t _pixelCount{0};
        BufferHolder<ColorType> _colors;
        bool _dirty{false};
    };

    template <typename TTransport,
              typename TProtocol,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TTransport> &&
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport>>>
    StaticBusDriverPixelBusT<TTransport, TProtocol> makeStaticDriverPixelBus(uint16_t pixelCount,
                                                                              typename TTransport::TransportSettingsType transportSettings,
                                                                              typename TProtocol::SettingsType settings)
    {
        return StaticBusDriverPixelBusT<TTransport, TProtocol>(pixelCount,
                                                               std::move(transportSettings),
                                                               std::move(settings));
    }

} // namespace npb
