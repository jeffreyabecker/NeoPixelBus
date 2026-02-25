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

namespace npb
{

    template <typename TColor>
    class IBusDriver
    {
    public:
        using ColorType = TColor;
        virtual ~IBusDriver() = default;

        virtual void initialize() = 0;
        virtual void update(std::span<const TColor> colors) = 0;
        virtual bool isReadyToUpdate() const = 0;
        virtual bool alwaysUpdate() const = 0;
    };

    template <typename TProtocol>
    concept BusDriverProtocolLike = requires {
                                      typename TProtocol::ColorType;
                                      typename TProtocol::TransportCategory;
                                  } && std::derived_from<TProtocol, IProtocol<typename TProtocol::ColorType>>;

    template <typename TProtocol, typename TTransport>
    concept BusDriverProtocolTransportCompatible = BusDriverProtocolLike<TProtocol> &&
                                                   TransportLike<TTransport> &&
                                                   TransportCategoryCompatible<typename TProtocol::TransportCategory,
                                                                               typename TTransport::TransportCategory>;

    template <typename TProtocol, typename TTransport>
        requires BusDriverProtocolTransportCompatible<TProtocol, TTransport>
    class ProtocolBusDriverT : public IBusDriver<typename TProtocol::ColorType>
    {
    public:
        using ColorType = typename TProtocol::ColorType;
        using TransportConfigType = typename TTransport::TransportConfigType;

        template <typename... TProtocolArgs>
        ProtocolBusDriverT(TransportConfigType transportConfig,
                           TProtocolArgs &&...protocolArgs)
            : _transport(std::move(transportConfig))
            , _protocol(makeProtocol(_transport, std::forward<TProtocolArgs>(protocolArgs)...))
        {
        }

        void initialize() override
        {
            _protocol.initialize();
        }

        void update(std::span<const ColorType> colors) override
        {
            _protocol.update(colors);
        }

        bool isReadyToUpdate() const override
        {
            return _protocol.isReadyToUpdate();
        }

        bool alwaysUpdate() const override
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
        template <typename TArg>
        static constexpr bool IsTransportBindableSettings = requires(std::remove_cvref_t<TArg> &settings, ResourceHandle<ITransport> bus) {
                                                                  settings.bus = std::move(bus);
                                                              };

        template <typename TArg>
        static auto bindProtocolArg(TArg &&arg,
                                    TTransport &transport)
        {
            using ArgType = std::remove_cvref_t<TArg>;

            if constexpr (IsTransportBindableSettings<TArg>)
            {
                ArgType bound = std::forward<TArg>(arg);
                bound.bus = transport;
                return bound;
            }
            else
            {
                return ArgType(std::forward<TArg>(arg));
            }
        }

        template <typename... TProtocolArgs>
        static TProtocol makeProtocol(TTransport &transport,
                                      TProtocolArgs &&...protocolArgs)
        {
            if constexpr ((IsTransportBindableSettings<TProtocolArgs> || ...))
            {
                return TProtocol(bindProtocolArg(std::forward<TProtocolArgs>(protocolArgs), transport)...);
            }
            else if constexpr (std::constructible_from<TProtocol, TProtocolArgs..., TTransport &>)
            {
                return TProtocol(std::forward<TProtocolArgs>(protocolArgs)..., transport);
            }
            else
            {
                return TProtocol(bindProtocolArg(std::forward<TProtocolArgs>(protocolArgs), transport)...);
            }
        }

        TTransport _transport;
        TProtocol _protocol;
    };

    template <typename TColor = Color>
    class BusDriverPixelBusT : public IPixelBus<TColor>
    {
    public:
        BusDriverPixelBusT(size_t pixelCount,
                           ResourceHandle<IBusDriver<TColor>> driver)
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

        std::span<TColor> colors()
        {
            return _colors;
        }

        std::span<const TColor> colors() const
        {
            return _colors;
        }

        void setPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) override
        {
            auto available = static_cast<std::ptrdiff_t>(_colors.size() - offset);
            auto requested = last - first;
            auto count = std::min(requested, available);
            std::copy_n(first, count, _colors.begin() + offset);
            _dirty = true;
        }

        void getPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) const override
        {
            auto available = static_cast<std::ptrdiff_t>(_colors.size() - offset);
            auto requested = last - first;
            auto count = std::min(requested, available);
            std::copy_n(_colors.cbegin() + offset, count, first);
        }

        void setPixelColors(size_t offset,
                            std::span<const TColor> pixelData) override
        {
            auto available = _colors.size() - offset;
            auto count = std::min(pixelData.size(), available);
            std::copy_n(pixelData.begin(), count, _colors.begin() + offset);
            _dirty = true;
        }

        void getPixelColors(size_t offset,
                            std::span<TColor> pixelData) const override
        {
            auto available = _colors.size() - offset;
            auto count = std::min(pixelData.size(), available);
            std::copy_n(_colors.cbegin() + offset, count, pixelData.begin());
        }

        void setPixelColor(size_t index, const TColor &color) override
        {
            if (index < _colors.size())
            {
                _colors[index] = color;
                _dirty = true;
            }
        }

        TColor getPixelColor(size_t index) const override
        {
            if (index < _colors.size())
            {
                return _colors[index];
            }

            return TColor{};
        }

    private:
        std::vector<TColor> _colors;
        ResourceHandle<IBusDriver<TColor>> _driver;
        bool _dirty{false};
    };

    namespace factory
    {

        template <typename TTransport, typename TProtocol>
            requires BusDriverProtocolTransportCompatible<TProtocol, TTransport>
        class OwningBusDriverPixelBusT
            : private ProtocolBusDriverT<TProtocol, TTransport>
            , public BusDriverPixelBusT<typename TProtocol::ColorType>
        {
        public:
            using ColorType = typename TProtocol::ColorType;
            using BusType = BusDriverPixelBusT<ColorType>;
            using DriverType = ProtocolBusDriverT<TProtocol, TTransport>;
            using TransportConfigType = typename TTransport::TransportConfigType;

            template <typename... TProtocolArgs>
            OwningBusDriverPixelBusT(uint16_t pixelCount,
                                     TransportConfigType transportConfig,
                                     TProtocolArgs &&...protocolArgs)
                : DriverType(std::move(transportConfig), std::forward<TProtocolArgs>(protocolArgs)...)
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

        template <typename TTransport, typename TProtocol, typename... TProtocolArgs>
            requires BusDriverProtocolTransportCompatible<TProtocol, TTransport>
        OwningBusDriverPixelBusT<TTransport, TProtocol> makeOwningDriverPixelBus(uint16_t pixelCount,
                                                                                   typename TTransport::TransportConfigType transportConfig,
                                                                                   TProtocolArgs &&...protocolArgs)
        {
            return OwningBusDriverPixelBusT<TTransport, TProtocol>(pixelCount,
                                                                    std::move(transportConfig),
                                                                    std::forward<TProtocolArgs>(protocolArgs)...);
        }

    } // namespace factory

} // namespace npb
