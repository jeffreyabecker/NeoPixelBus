#pragma once

#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

#include "core/Compat.h"
#include "core/IPixelBus.h"

#include "buses/BusDriver.h"
#include "buses/PixelBus.h"

#include "protocols/WithShaderProtocol.h"

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

        template <typename TProtocol,
                  typename = std::enable_if_t<std::is_base_of<IProtocol<typename TProtocol::ColorType>, TProtocol>::value>>
        class OwningShaderProtocolT : public WithShader<typename TProtocol::ColorType, TProtocol>
        {
        public:
            using ColorType = typename TProtocol::ColorType;
            using BaseType = WithShader<ColorType, TProtocol>;
            using ShaderType = IShader<ColorType>;
            using SettingsType = typename BaseType::SettingsType;

            OwningShaderProtocolT(uint16_t pixelCount,
                                  SettingsType settings,
                                  std::unique_ptr<ShaderType> shader)
                : BaseType(pixelCount,
                           bindShader(std::move(settings), shader.get()))
                , _shader(std::move(shader))
            {
            }

            template <typename... TArgs,
                      typename = std::enable_if_t<(sizeof...(TArgs) > 0)>>
            OwningShaderProtocolT(uint16_t pixelCount,
                                  SettingsType settings,
                                  std::unique_ptr<ShaderType> shader,
                                  TArgs &&...args)
                : BaseType(pixelCount,
                           bindShader(std::move(settings), shader.get()),
                           std::forward<TArgs>(args)...)
                , _shader(std::move(shader))
            {
            }

        private:
            static SettingsType bindShader(SettingsType settings,
                                           ShaderType *shader)
            {
                settings.shader = shader;
                return settings;
            }

            std::unique_ptr<ShaderType> _shader;
        };

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

        OwningPixelBusT(std::unique_ptr<TProtocol> protocol,
                        std::unique_ptr<TTransport> transport)
            : _transport(std::move(transport))
            , _protocol(std::move(protocol))
            , _bus(*_protocol)
        {
        }

        OwningPixelBusT(uint16_t,
                        std::unique_ptr<TProtocol> protocol,
                        std::unique_ptr<TTransport> transport)
            : OwningPixelBusT(std::move(protocol), std::move(transport))
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

    template <typename TProtocol,
              typename = std::enable_if_t<std::is_base_of<IProtocol<typename TProtocol::ColorType>, TProtocol>::value>>
    class OwningErasedTransportPixelBusT : public IPixelBus<typename TProtocol::ColorType>
    {
    public:
        using ColorType = typename TProtocol::ColorType;

        OwningErasedTransportPixelBusT(std::unique_ptr<TProtocol> protocol,
                                       TransportPtr transport)
            : _transport(std::move(transport))
            , _protocol(std::move(protocol))
            , _bus(*_protocol)
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

        ITransport &transport()
        {
            return *_transport;
        }

        const ITransport &transport() const
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
        TransportPtr _transport;
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

    template <typename TProtocolConfig,
              typename TProtocolConfigDecay = remove_cvref_t<TProtocolConfig>,
              typename ProtocolType = typename ProtocolConfigTraits<TProtocolConfigDecay>::ProtocolType,
              typename = std::enable_if_t<FactoryProtocolConfig<TProtocolConfigDecay> &&
                                          ProtocolSettingsTransportBindable<ProtocolType>>>
    ProtocolPtr<TProtocolConfigDecay> makeProtocol(uint16_t pixelCount,
                                                   TProtocolConfig protocolConfig,
                                                   TransportPtr &transport)
    {
        using ProtocolTraits = ProtocolConfigTraits<TProtocolConfigDecay>;
        using SettingsType = typename ProtocolType::SettingsType;

        SettingsType settings = ProtocolTraits::toSettings(std::move(protocolConfig));
        settings.bus = transport.get();
        return std::make_unique<ProtocolType>(pixelCount, std::move(settings));
    }

    template <typename TProtocolConfig,
              typename TTransport,
              typename TProtocolConfigDecay = remove_cvref_t<TProtocolConfig>,
              typename ProtocolType = typename ProtocolConfigTraits<TProtocolConfigDecay>::ProtocolType,
              typename ColorType = typename ProtocolType::ColorType,
              typename ShaderProtocolType = detail::OwningShaderProtocolT<ProtocolType>,
              typename = std::enable_if_t<FactoryProtocolConfig<TProtocolConfigDecay>>,
              typename = std::enable_if_t<TransportLike<TTransport>>,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<ProtocolType,
                                                                               TTransport>>,
              typename = std::enable_if_t<BusDriverProtocolSettingsConstructible<ProtocolType,
                                                                                TTransport>>>
    std::unique_ptr<ShaderProtocolType> makeProtocol(uint16_t pixelCount,
                                                     TProtocolConfig protocolConfig,
                                                     TTransport &transport,
                                                     std::unique_ptr<IShader<ColorType>> shader)
    {
        using ProtocolTraits = ProtocolConfigTraits<TProtocolConfigDecay>;
        using BaseSettingsType = typename ProtocolType::SettingsType;
        using SettingsType = typename ShaderProtocolType::SettingsType;

        BaseSettingsType baseSettings = ProtocolTraits::toSettings(std::move(protocolConfig));
        SettingsType settings{};
        static_cast<BaseSettingsType &>(settings) = std::move(baseSettings);

        if constexpr (ProtocolSettingsTransportBindable<ShaderProtocolType>)
        {
            settings.bus = &transport;
            return std::make_unique<ShaderProtocolType>(pixelCount,
                                                        std::move(settings),
                                                        std::move(shader));
        }
        else if constexpr (std::is_constructible<ShaderProtocolType,
                                                 uint16_t,
                                                 SettingsType,
                                                 TTransport &>::value)
        {
            return std::make_unique<ShaderProtocolType>(pixelCount,
                                                        std::move(settings),
                                                        std::move(shader),
                                                        transport);
        }
        else
        {
            return std::make_unique<ShaderProtocolType>(pixelCount,
                                                        std::move(settings),
                                                        std::move(shader));
        }
    }

    template <typename TProtocolConfig,
              typename TProtocolConfigDecay = remove_cvref_t<TProtocolConfig>,
              typename ProtocolType = typename ProtocolConfigTraits<TProtocolConfigDecay>::ProtocolType,
              typename ColorType = typename ProtocolType::ColorType,
              typename ShaderProtocolType = detail::OwningShaderProtocolT<ProtocolType>,
              typename = std::enable_if_t<FactoryProtocolConfig<TProtocolConfigDecay> &&
                                          ProtocolSettingsTransportBindable<ProtocolType>>>
    std::unique_ptr<ShaderProtocolType> makeProtocol(uint16_t pixelCount,
                                                     TProtocolConfig protocolConfig,
                                                     TransportPtr &transport,
                                                     std::unique_ptr<IShader<ColorType>> shader)
    {
        using ProtocolTraits = ProtocolConfigTraits<TProtocolConfigDecay>;
        using BaseSettingsType = typename ProtocolType::SettingsType;
        using SettingsType = typename ShaderProtocolType::SettingsType;

        BaseSettingsType baseSettings = ProtocolTraits::toSettings(std::move(protocolConfig));
        SettingsType settings{};
        static_cast<BaseSettingsType &>(settings) = std::move(baseSettings);
        settings.bus = transport.get();

        return std::make_unique<ShaderProtocolType>(pixelCount,
                                                    std::move(settings),
                                                    std::move(shader));
    }

    template <typename TProtocol,
              typename TTransport,
              typename = std::enable_if_t<BusDriverProtocolTransportCompatible<TProtocol, TTransport> &&
                                          BusDriverProtocolSettingsConstructible<TProtocol, TTransport>>>
    std::unique_ptr<IPixelBus<typename TProtocol::ColorType>> makeTypedBus(std::unique_ptr<TProtocol> protocol,
                                                                            std::unique_ptr<TTransport> transport)
    {
        return std::make_unique<OwningPixelBusT<TProtocol, TTransport>>(
            std::move(protocol),
            std::move(transport));
    }

    template <typename TProtocol,
              typename = std::enable_if_t<std::is_base_of<IProtocol<typename TProtocol::ColorType>, TProtocol>::value>>
    std::unique_ptr<IPixelBus<typename TProtocol::ColorType>> makeBus(std::unique_ptr<TProtocol> protocol,
                                                                       TransportPtr transport)
    {
        return std::make_unique<OwningErasedTransportPixelBusT<TProtocol>>(
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
        auto transport = makeTransport(std::move(transportConfig));
        auto protocol = makeProtocol(pixelCount, std::move(protocolConfig), *transport);
        return makeBus(std::move(protocol), std::move(transport));
    }

    template <typename TProtocolConfig,
              typename TTransportConfig,
              typename TProtocolConfigDecay = remove_cvref_t<TProtocolConfig>,
              typename TTransportConfigDecay = remove_cvref_t<TTransportConfig>,
              typename ProtocolType = typename ProtocolConfigTraits<TProtocolConfigDecay>::ProtocolType,
              typename ColorType = typename ProtocolType::ColorType,
              typename = std::enable_if_t<FactoryProtocolConfig<TProtocolConfigDecay> &&
                                          FactoryTransportConfig<TTransportConfigDecay>>>
    BusPointerType<TProtocolConfigDecay> makeRuntimeBus(uint16_t pixelCount,
                                                        TProtocolConfig protocolConfig,
                                                        TTransportConfig transportConfig,
                                                        std::unique_ptr<IShader<ColorType>> shader)
    {
        auto transport = makeTransport(std::move(transportConfig));
        auto protocol = makeProtocol(pixelCount,
                                     std::move(protocolConfig),
                                     *transport,
                                     std::move(shader));
        return makeBus(std::move(protocol), std::move(transport));
    }

} // namespace npb::factory
