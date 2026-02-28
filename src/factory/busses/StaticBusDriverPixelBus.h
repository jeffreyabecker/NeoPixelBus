#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

#include "buses/BusDriverConstraints.h"
#include "core/IPixelBus.h"

namespace npb
{

    template <typename TTransport,
              typename TProtocol,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TTransport>>>
    class StaticBusDriverPixelBusT
        : public IPixelBus<typename TProtocol::ColorType>
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
            , _colors(pixelCount)
        {
        }

        void begin() override
        {
            _protocol.initialize();
        }

        void show() override
        {
            if (!_dirty && !_protocol.alwaysUpdate())
            {
                return;
            }

            _protocol.update(span<const ColorType>{_colors.data(), _colors.size()});
            _dirty = false;
        }

        bool canShow() const override
        {
            return _protocol.isReadyToUpdate();
        }

        size_t pixelCount() const override
        {
            return _colors.size();
        }

        span<ColorType> colors()
        {
            return span<ColorType>{_colors.data(), _colors.size()};
        }

        span<const ColorType> colors() const
        {
            return span<const ColorType>{_colors.data(), _colors.size()};
        }

        void setPixelColors(size_t offset,
                            ColorIteratorT<ColorType> first,
                            ColorIteratorT<ColorType> last) override
        {
            if (offset >= _colors.size())
            {
                return;
            }

            auto available = static_cast<std::ptrdiff_t>(_colors.size() - offset);
            auto requested = last - first;
            auto count = std::min(requested, available);

            auto src = first;
            auto dest = _colors.begin() + offset;
            for (std::ptrdiff_t index = 0; index < count; ++index, ++src, ++dest)
            {
                *dest = *src;
            }

            _dirty = true;
        }

        void getPixelColors(size_t offset,
                            ColorIteratorT<ColorType> first,
                            ColorIteratorT<ColorType> last) const override
        {
            if (offset >= _colors.size())
            {
                return;
            }

            auto available = static_cast<std::ptrdiff_t>(_colors.size() - offset);
            auto requested = last - first;
            auto count = std::min(requested, available);

            auto src = _colors.cbegin() + offset;
            auto dest = first;
            for (std::ptrdiff_t index = 0; index < count; ++index, ++src, ++dest)
            {
                *dest = *src;
            }
        }

        void setPixelColors(size_t offset,
                            span<const ColorType> pixelData) override
        {
            if (offset >= _colors.size())
            {
                return;
            }

            auto available = _colors.size() - offset;
            auto count = std::min(pixelData.size(), available);
            std::copy_n(pixelData.begin(), count, _colors.begin() + offset);
            _dirty = true;
        }

        void getPixelColors(size_t offset,
                            span<ColorType> pixelData) const override
        {
            if (offset >= _colors.size())
            {
                return;
            }

            auto available = _colors.size() - offset;
            auto count = std::min(pixelData.size(), available);
            std::copy_n(_colors.cbegin() + offset, count, pixelData.begin());
        }

        void setPixelColor(size_t index, const ColorType &color) override
        {
            if (index < _colors.size())
            {
                _colors[index] = color;
                _dirty = true;
            }
        }

        ColorType getPixelColor(size_t index) const override
        {
            if (index < _colors.size())
            {
                return _colors[index];
            }

            return ColorType{};
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
        std::vector<ColorType> _colors;
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

    template <typename TTransport,
              typename TProtocol,
              typename TBaseSettings,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TTransport> &&
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport> &&
                                          std::is_base_of<typename std::remove_cv<typename std::remove_reference<TBaseSettings>::type>::type,
                                                          typename TProtocol::SettingsType>::value &&
                                          std::is_constructible<typename TProtocol::SettingsType,
                                                                typename TProtocol::SettingsType>::value>>
    StaticBusDriverPixelBusT<TTransport, TProtocol> makeStaticDriverPixelBus(uint16_t pixelCount,
                                                                              typename TTransport::TransportSettingsType transportSettings,
                                                                              typename TProtocol::SettingsType settings,
                                                                              TBaseSettings &&baseSettings)
    {
        using BaseSettingsType = typename std::remove_cv<typename std::remove_reference<TBaseSettings>::type>::type;
        static_cast<BaseSettingsType &>(settings) = std::forward<TBaseSettings>(baseSettings);

        return makeStaticDriverPixelBus<TTransport, TProtocol>(pixelCount,
                                                               std::move(transportSettings),
                                                               std::move(settings));
    }

} // namespace npb
