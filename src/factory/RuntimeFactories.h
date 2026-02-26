#pragma once

#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

#include "core/Compat.h"
#include "core/IPixelBus.h"

#include "buses/BusDriver.h"
#include "buses/PixelBus.h"

#include "ProtocolConfigs.h"
#include "Traits.h"

namespace npb::factory
{

    template <typename TProtocolConfig>
    using ProtocolPtr = std::unique_ptr<typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType>;

    template <typename TTransportConfig>
    using TypedTransportPtr = std::unique_ptr<typename TransportConfigTraits<remove_cvref_t<TTransportConfig>>::TransportType>;

    namespace detail
    {

        template <typename TTransportConfig,
                  typename TTransportConfigDecay = remove_cvref_t<TTransportConfig>,
                  typename = std::enable_if_t<FactoryTransportConfig<TTransportConfigDecay>>>
        TypedTransportPtr<TTransportConfigDecay> makeTypedTransportImpl(TTransportConfig transportConfig)
        {
            using TransportTraits = TransportConfigTraits<TTransportConfigDecay>;
            using TransportType = typename TransportTraits::TransportType;

            return std::make_unique<TransportType>(
                TransportTraits::toSettings(std::move(transportConfig)));
        }

    } // namespace detail

    template <typename TProtocol,
              typename TTransport,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TTransport> &&
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport>>>
    class OwningPixelBusT : public IPixelBus<typename TProtocol::ColorType>
    {
    public:
        using ColorType = typename TProtocol::ColorType;

        OwningPixelBusT(uint16_t pixelCount,
                        std::unique_ptr<TProtocol> protocol,
                        std::unique_ptr<TTransport> transport)
            : _transport(std::move(transport))
            , _protocol(std::move(protocol))
            , _bus(pixelCount, *_protocol)
        {
        }

        void begin() override
        {
            _bus.begin();
        }

        void show() override
        {
            _bus.show();
        }

        bool canShow() const override
        {
            return _bus.canShow();
        }

        size_t pixelCount() const override
        {
            return _bus.pixelCount();
        }

        void setPixelColors(size_t offset,
                            ColorIteratorT<ColorType> first,
                            ColorIteratorT<ColorType> last) override
        {
            _bus.setPixelColors(offset, first, last);
        }

        void getPixelColors(size_t offset,
                            ColorIteratorT<ColorType> first,
                            ColorIteratorT<ColorType> last) const override
        {
            _bus.getPixelColors(offset, first, last);
        }

        void setPixelColors(size_t offset,
                            span<const ColorType> pixelData) override
        {
            _bus.setPixelColors(offset, pixelData);
        }

        void getPixelColors(size_t offset,
                            span<ColorType> pixelData) const override
        {
            _bus.getPixelColors(offset, pixelData);
        }

        void setPixelColor(size_t index, const ColorType &color) override
        {
            _bus.setPixelColor(index, color);
        }

        ColorType getPixelColor(size_t index) const override
        {
            return _bus.getPixelColor(index);
        }

        TTransport &transport()
        {
            return *_transport;
        }

        const TTransport &transport() const
        {
            return *_transport;
        }

        TProtocol &protocol()
        {
            return *_protocol;
        }

        const TProtocol &protocol() const
        {
            return *_protocol;
        }

    private:
        std::unique_ptr<TTransport> _transport;
        std::unique_ptr<TProtocol> _protocol;
        PixelBusT<ColorType> _bus;
    };

    template <typename TTransportConfig,
              typename TTransportConfigDecay = remove_cvref_t<TTransportConfig>,
              typename = std::enable_if_t<FactoryTransportConfig<TTransportConfigDecay>>>
    TypedTransportPtr<TTransportConfigDecay> makeTypedTransport(TTransportConfig transportConfig)
    {
        return detail::makeTypedTransportImpl(std::move(transportConfig));
    }

    template <typename TTransportConfig,
              typename TTransportConfigDecay = remove_cvref_t<TTransportConfig>,
              typename = std::enable_if_t<FactoryTransportConfig<TTransportConfigDecay>>>
    TransportPtr makeTransport(TTransportConfig transportConfig)
    {
        return detail::makeTypedTransportImpl(std::move(transportConfig));
    }

    template <typename TProtocolConfig,
              typename TTransport,
              typename TProtocolConfigDecay = remove_cvref_t<TProtocolConfig>,
              typename = std::enable_if_t<FactoryProtocolConfig<TProtocolConfigDecay>>,
              typename = std::enable_if_t<TransportLike<TTransport>>,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<typename ProtocolConfigTraits<TProtocolConfigDecay>::ProtocolType,
                                                                               TTransport>>,
              typename = std::enable_if_t<BusDriverProtocolSettingsConstructible<typename ProtocolConfigTraits<TProtocolConfigDecay>::ProtocolType,
                                                                                TTransport>>>
    ProtocolPtr<TProtocolConfigDecay> makeProtocol(uint16_t pixelCount,
                                                   TProtocolConfig protocolConfig,
                                                   TTransport &transport)
    {
        using ProtocolTraits = ProtocolConfigTraits<TProtocolConfigDecay>;
        using ProtocolType = typename ProtocolTraits::ProtocolType;
        using SettingsType = typename ProtocolType::SettingsType;

        SettingsType settings = ProtocolTraits::toSettings(std::move(protocolConfig));

        if constexpr (ProtocolSettingsTransportBindable<ProtocolType>)
        {
            settings.bus = &transport;
            return std::make_unique<ProtocolType>(pixelCount, std::move(settings));
        }
        else if constexpr (std::is_constructible<ProtocolType,
                                                 uint16_t,
                                                 SettingsType,
                                                 TTransport &>::value)
        {
            return std::make_unique<ProtocolType>(pixelCount, std::move(settings), transport);
        }
        else
        {
            return std::make_unique<ProtocolType>(pixelCount, std::move(settings));
        }
    }

    template <typename TProtocol,
              typename TTransport,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TTransport> &&
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport>>>
    std::unique_ptr<IPixelBus<typename TProtocol::ColorType>> makeBus(uint16_t pixelCount,
                                                                       std::unique_ptr<TProtocol> protocol,
                                                                       std::unique_ptr<TTransport> transport)
    {
        return std::make_unique<OwningPixelBusT<TProtocol, TTransport>>(
            pixelCount,
            std::move(protocol),
            std::move(transport));
    }

    template <typename TProtocolConfig,
              typename TTransportConfig,
              typename TProtocolConfigDecay = remove_cvref_t<TProtocolConfig>,
              typename TTransportConfigDecay = remove_cvref_t<TTransportConfig>,
              typename = std::enable_if_t<FactoryProtocolConfig<TProtocolConfigDecay> &&
                                          FactoryTransportConfig<TTransportConfigDecay>>>
    BusPointerType<TProtocolConfigDecay> makeRuntimeBus(uint16_t pixelCount,
                                                        TProtocolConfig protocolConfig,
                                                        TTransportConfig transportConfig)
    {
        auto transport = makeTypedTransport(std::move(transportConfig));
        auto protocol = makeProtocol(pixelCount, std::move(protocolConfig), *transport);
        return makeBus(pixelCount, std::move(protocol), std::move(transport));
    }

} // namespace npb::factory
