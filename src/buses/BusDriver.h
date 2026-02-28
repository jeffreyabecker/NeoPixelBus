#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>
#include <algorithm>
#include <utility>

#include "core/IPixelBus.h"
#include "protocols/IProtocol.h"
#include "transports/ITransport.h"

namespace npb
{

    template <typename TProtocol, typename = void>
    struct BusDriverProtocolLikeImpl : std::false_type
    {
    };

    template <typename TProtocol>
    struct BusDriverProtocolLikeImpl<TProtocol,
                                     std::void_t<typename TProtocol::ColorType,
                                                 typename TProtocol::TransportCategory>>
        : std::integral_constant<bool,
                                 std::is_base_of<IProtocol<typename TProtocol::ColorType>, TProtocol>::value>
    {
    };

    template <typename TProtocol>
    static constexpr bool BusDriverProtocolLike = BusDriverProtocolLikeImpl<TProtocol>::value;

    template <typename TProtocol, typename TTransport>
    static constexpr bool BusDriverProtocolSettingsConstructible =
        ProtocolPixelSettingsConstructible<TProtocol> ||
        std::is_constructible<TProtocol,
                              uint16_t,
                              typename TProtocol::SettingsType,
                              TTransport &>::value;

    template <typename TProtocol, typename TTransport>
    static constexpr bool BusDriverProtocolTransportCompatible =
        BusDriverProtocolLike<TProtocol> &&
        TransportLike<TTransport> &&
        TransportCategoryCompatible<typename TProtocol::TransportCategory,
                                    typename TTransport::TransportCategory>;

    template <typename TDriver, typename = void>
    struct BusDriverLikeImpl : std::false_type
    {
    };

    template <typename TDriver>
    struct BusDriverLikeImpl<TDriver,
                             std::void_t<typename TDriver::ColorType,
                                         decltype(std::declval<TDriver &>().initialize()),
                                         decltype(std::declval<TDriver &>().update(std::declval<span<const typename TDriver::ColorType>>())),
                                         decltype(std::declval<const TDriver &>().isReadyToUpdate()),
                                         decltype(std::declval<const TDriver &>().alwaysUpdate())>>
        : std::integral_constant<bool,
                                 std::is_convertible<decltype(std::declval<const TDriver &>().isReadyToUpdate()), bool>::value &&
                                     std::is_convertible<decltype(std::declval<const TDriver &>().alwaysUpdate()), bool>::value>
    {
    };

    template <typename TDriver>
    static constexpr bool BusDriverLike = BusDriverLikeImpl<TDriver>::value;

    template <typename TProtocol,
              typename TTransport,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TTransport> &&
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport>>>
    class ProtocolBusDriverT
    {
    public:
        using ColorType = typename TProtocol::ColorType;
        using ProtocolSettingsType = typename TProtocol::SettingsType;
        using TransportSettingsType = typename TTransport::TransportSettingsType;

        ProtocolBusDriverT(uint16_t pixelCount,
                           TransportSettingsType transportSettings,
                           ProtocolSettingsType settings)
            : _transport(std::move(transportSettings))
            , _protocol(makeProtocol(pixelCount, _transport, std::move(settings)))
        {
        }

        void initialize()
        {
            _protocol.initialize();
        }

        void update(span<const ColorType> colors)
        {
            _protocol.update(colors);
        }

        bool isReadyToUpdate() const
        {
            return _protocol.isReadyToUpdate();
        }

        bool alwaysUpdate() const
        {
            return _protocol.alwaysUpdate();
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
    };

    template <typename TDriver,
              typename = std::enable_if_t<BusDriverLike<TDriver>>>
    class BusDriverPixelBusT : public IPixelBus<typename TDriver::ColorType>
    {
    public:
        using ColorType = typename TDriver::ColorType;

        BusDriverPixelBusT() = default;

        explicit BusDriverPixelBusT(size_t pixelCount)
            : _colors(pixelCount)
        {
        }

        explicit BusDriverPixelBusT(TDriver &driver)
            : _colors(driver.protocol().pixelCount())
            , _driver{&driver}
        {
        }

        BusDriverPixelBusT(size_t pixelCount,
                           TDriver &driver)
            : _colors(pixelCount)
            , _driver{&driver}
        {
        }

        void begin() override
        {
            if (nullptr == _driver)
            {
                return;
            }

            _driver->initialize();
        }

        void show() override
        {
            if (nullptr == _driver)
            {
                return;
            }

            if (!_dirty && !_driver->alwaysUpdate())
            {
                return;
            }

            _driver->update(span<const ColorType>{_colors.data(), _colors.size()});
            _dirty = false;
        }

        bool canShow() const override
        {
            if (nullptr == _driver)
            {
                return false;
            }

            return _driver->isReadyToUpdate();
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

    private:
    protected:
        void bindDriver(TDriver &driver)
        {
            _driver = &driver;
        }

        void resizePixelBuffer(size_t pixelCount)
        {
            _colors.assign(pixelCount, ColorType{});
            _dirty = true;
        }

    private:
        std::vector<ColorType> _colors;
        TDriver *_driver{nullptr};
        bool _dirty{false};
    };

    template <typename TTransport,
              typename TProtocol,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TTransport>>>
    class HeapBusDriverPixelBusT
        : public BusDriverPixelBusT<ProtocolBusDriverT<TProtocol, TTransport>>
    {
    public:
        using ColorType = typename TProtocol::ColorType;
        using DriverType = ProtocolBusDriverT<TProtocol, TTransport>;
        using BusType = BusDriverPixelBusT<DriverType>;
        using ProtocolSettingsType = typename TProtocol::SettingsType;
        using TransportSettingsType = typename TTransport::TransportSettingsType;

        HeapBusDriverPixelBusT(uint16_t pixelCount,
                               TransportSettingsType transportSettings,
                               ProtocolSettingsType settings)
            : BusType()
            , _driver(std::make_unique<DriverType>(pixelCount,
                                                   std::move(transportSettings),
                                                   std::move(settings)))
        {
            this->bindDriver(*_driver);
            this->resizePixelBuffer(_driver->protocol().pixelCount());
        }

        HeapBusDriverPixelBusT(const HeapBusDriverPixelBusT &) = delete;
        HeapBusDriverPixelBusT &operator=(const HeapBusDriverPixelBusT &) = delete;
        HeapBusDriverPixelBusT(HeapBusDriverPixelBusT &&) = default;
        HeapBusDriverPixelBusT &operator=(HeapBusDriverPixelBusT &&) = default;

        TTransport &transport()
        {
            return _driver->transport();
        }

        const TTransport &transport() const
        {
            return _driver->transport();
        }

        TProtocol &protocol()
        {
            return _driver->protocol();
        }

        const TProtocol &protocol() const
        {
            return _driver->protocol();
        }

    private:
        std::unique_ptr<DriverType> _driver;
    };

    template <typename TTransport,
              typename TProtocol,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TTransport> &&
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport>>>
    HeapBusDriverPixelBusT<TTransport, TProtocol> makeHeapDriverPixelBus(uint16_t pixelCount,
                                                                          typename TTransport::TransportSettingsType transportSettings,
                                                                          typename TProtocol::SettingsType settings)
    {
        return HeapBusDriverPixelBusT<TTransport, TProtocol>(pixelCount,
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
    HeapBusDriverPixelBusT<TTransport, TProtocol> makeHeapDriverPixelBus(uint16_t pixelCount,
                                                                          typename TTransport::TransportSettingsType transportSettings,
                                                                          typename TProtocol::SettingsType settings,
                                                                          TBaseSettings &&baseSettings)
    {
        using BaseSettingsType = typename std::remove_cv<typename std::remove_reference<TBaseSettings>::type>::type;
        static_cast<BaseSettingsType &>(settings) = std::forward<TBaseSettings>(baseSettings);

        return makeHeapDriverPixelBus<TTransport, TProtocol>(pixelCount,
                                                             std::move(transportSettings),
                                                             std::move(settings));
    }

} // namespace npb

#include "factory/busses/StaticBusDriverPixelBus.h"


