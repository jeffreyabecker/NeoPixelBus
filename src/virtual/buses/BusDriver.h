#pragma once

#include <cstddef>
#include <cstdint>
#include <concepts>
#include <type_traits>
#include <vector>
#include <algorithm>
#include <utility>

#include "../IPixelBus.h"
#include "../ResourceHandle.h"
#include "../protocols/IProtocol.h"
#include "../transports/ITransport.h"

namespace npb::factory
{

    template <typename TProtocol>
    concept BusDriverProtocolLike = requires {
                                      typename TProtocol::ColorType;
                                      typename TProtocol::TransportCategory;
                                  } && std::derived_from<TProtocol, IProtocol<typename TProtocol::ColorType>>;

    template <typename TProtocol, typename TTransport>
    concept BusDriverProtocolSettingsConstructible = ProtocolPixelSettingsConstructible<TProtocol> ||
                                                     std::constructible_from<TProtocol,
                                                                             uint16_t,
                                                                             typename TProtocol::SettingsType,
                                                                             TTransport &>;

    template <typename TProtocol, typename TTransport>
    concept BusDriverProtocolTransportCompatible = BusDriverProtocolLike<TProtocol> &&
                                                   TransportLike<TTransport> &&
                                                   TransportCategoryCompatible<typename TProtocol::TransportCategory,
                                                                               typename TTransport::TransportCategory>;

    template <typename TDriver>
    concept BusDriverLike = requires(TDriver &driver,
                                     const TDriver &constDriver,
                                     std::span<const typename TDriver::ColorType> colors) {
                                typename TDriver::ColorType;
                                driver.initialize();
                                driver.update(colors);
                                {
                                    constDriver.isReadyToUpdate()
                                } -> std::convertible_to<bool>;
                                {
                                    constDriver.alwaysUpdate()
                                } -> std::convertible_to<bool>;
                            };

    template <typename TProtocol, typename TTransport>
        requires BusDriverProtocolTransportCompatible<TProtocol, TTransport> &&
                 BusDriverProtocolSettingsConstructible<TProtocol, TTransport>
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

        void update(std::span<const ColorType> colors)
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
                settings.bus = transport;
                return TProtocol(pixelCount, std::move(settings));
            }
            else if constexpr (std::constructible_from<TProtocol,
                                                       uint16_t,
                                                       ProtocolSettingsType,
                                                       TTransport &>)
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

    template <typename TDriver>
        requires BusDriverLike<TDriver>
    class BusDriverPixelBusT : public IPixelBus<typename TDriver::ColorType>
    {
    public:
        using ColorType = typename TDriver::ColorType;

        BusDriverPixelBusT(size_t pixelCount,
                           ResourceHandle<TDriver> driver)
            : _colors(pixelCount)
            , _driver{std::move(driver)}
        {
        }

        void begin() override
        {
            _driver->initialize();
        }

        void show() override
        {
            if (!_dirty && !_driver->alwaysUpdate())
            {
                return;
            }

            _driver->update(_colors);
            _dirty = false;
        }

        bool canShow() const override
        {
            return _driver->isReadyToUpdate();
        }

        size_t pixelCount() const override
        {
            return _colors.size();
        }

        std::span<ColorType> colors()
        {
            return _colors;
        }

        std::span<const ColorType> colors() const
        {
            return _colors;
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
            std::copy_n(first, count, _colors.begin() + offset);
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
            std::copy_n(_colors.cbegin() + offset, count, first);
        }

        void setPixelColors(size_t offset,
                            std::span<const ColorType> pixelData) override
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
                            std::span<ColorType> pixelData) const override
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
        std::vector<ColorType> _colors;
        ResourceHandle<TDriver> _driver;
        bool _dirty{false};
    };

    template <typename TTransport, typename TProtocol>
        requires BusDriverProtocolTransportCompatible<TProtocol, TTransport>
    class OwningBusDriverPixelBusT
        : private ProtocolBusDriverT<TProtocol, TTransport>
        , public BusDriverPixelBusT<ProtocolBusDriverT<TProtocol, TTransport>>
    {
    public:
        using ColorType = typename TProtocol::ColorType;
        using DriverType = ProtocolBusDriverT<TProtocol, TTransport>;
        using BusType = BusDriverPixelBusT<DriverType>;
        using ProtocolSettingsType = typename TProtocol::SettingsType;
        using TransportSettingsType = typename TTransport::TransportSettingsType;

        OwningBusDriverPixelBusT(uint16_t pixelCount,
                                 TransportSettingsType transportSettings,
                                 ProtocolSettingsType settings)
            : DriverType(pixelCount, std::move(transportSettings), std::move(settings))
            , BusType(pixelCount, static_cast<DriverType &>(*this))
        {
        }

        TTransport &transport()
        {
            return static_cast<DriverType &>(*this).transport();
        }

        const TTransport &transport() const
        {
            return static_cast<const DriverType &>(*this).transport();
        }

        TProtocol &protocol()
        {
            return static_cast<DriverType &>(*this).protocol();
        }

        const TProtocol &protocol() const
        {
            return static_cast<const DriverType &>(*this).protocol();
        }
    };

    template <typename TTransport, typename TProtocol>
        requires BusDriverProtocolTransportCompatible<TProtocol, TTransport> &&
                 BusDriverProtocolSettingsConstructible<TProtocol, TTransport>
    OwningBusDriverPixelBusT<TTransport, TProtocol> makeOwningDriverPixelBus(uint16_t pixelCount,
                                                                               typename TTransport::TransportSettingsType transportSettings,
                                                                               typename TProtocol::SettingsType settings)
    {
        return OwningBusDriverPixelBusT<TTransport, TProtocol>(pixelCount,
                                                                std::move(transportSettings),
                                                                std::move(settings));
    }

    template <typename TTransport, typename TProtocol, typename TBaseSettings>
        requires BusDriverProtocolTransportCompatible<TProtocol, TTransport> &&
                 BusDriverProtocolSettingsConstructible<TProtocol, TTransport> &&
                 std::derived_from<typename TProtocol::SettingsType, std::remove_cvref_t<TBaseSettings>> &&
                 std::constructible_from<typename TProtocol::SettingsType, typename TProtocol::SettingsType>
    OwningBusDriverPixelBusT<TTransport, TProtocol> makeOwningDriverPixelBus(uint16_t pixelCount,
                                                                               typename TTransport::TransportSettingsType transportSettings,
                                                                               typename TProtocol::SettingsType settings,
                                                                               TBaseSettings &&baseSettings)
    {
        using BaseSettingsType = std::remove_cvref_t<TBaseSettings>;
        static_cast<BaseSettingsType &>(settings) = std::forward<TBaseSettings>(baseSettings);

        return makeOwningDriverPixelBus<TTransport, TProtocol>(pixelCount,
                                                                std::move(transportSettings),
                                                                std::move(settings));
    }

} // namespace npb::factory
