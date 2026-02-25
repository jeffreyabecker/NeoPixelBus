#pragma once

#include <cstddef>
#include <cstdint>
#include <concepts>
#include <vector>
#include <algorithm>
#include <memory>
#include <utility>

#include "../IPixelBus.h"
#include "../protocols/IProtocol.h"
#include "../ResourceHandle.h"
#include "../transports/ITransport.h"

namespace npb
{

    template <typename TColor>
    class PixelBusT : public IPixelBus<TColor>
    {
    public:
        PixelBusT(size_t pixelCount,
                  ResourceHandle<IProtocol<TColor>> protocol)
            : _colors(pixelCount), _protocol{std::move(protocol)}
        {
        }

        void begin() override
        {
            _protocol->initialize();
        }

        void show() override
        {
            if (!_dirty && !_protocol->alwaysUpdate())
            {
                return;
            }
            _protocol->update(_colors);
            _dirty = false;
        }

        bool canShow() const override
        {
            return _protocol->isReadyToUpdate();
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

        // -----------------------------------------------------------------
        // Primary interface overrides (iterator pair)
        // -----------------------------------------------------------------
        void setPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) override
        {
            if (offset >= _colors.size())
            {
                return;
            }

            auto available = static_cast<std::ptrdiff_t>(_colors.size() - offset);
            auto requested = last - first;
            auto count     = std::min(requested, available);
            std::copy_n(first, count, _colors.begin() + offset);
            _dirty = true;
        }

        void getPixelColors(size_t offset,
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) const override
        {
            if (offset >= _colors.size())
            {
                return;
            }

            auto available = static_cast<std::ptrdiff_t>(_colors.size() - offset);
            auto requested = last - first;
            auto count     = std::min(requested, available);
            std::copy_n(_colors.cbegin() + offset, count, first);
        }

        // -----------------------------------------------------------------
        // Convenience overrides – span (direct copy, no iterator wrapper)
        // -----------------------------------------------------------------
        void setPixelColors(size_t offset,
                            std::span<const TColor> pixelData) override
        {
            if (offset >= _colors.size())
            {
                return;
            }

            auto available = _colors.size() - offset;
            auto count     = std::min(pixelData.size(), available);
            std::copy_n(pixelData.begin(), count, _colors.begin() + offset);
            _dirty = true;
        }

        void getPixelColors(size_t offset,
                            std::span<TColor> pixelData) const override
        {
            if (offset >= _colors.size())
            {
                return;
            }

            auto available = _colors.size() - offset;
            auto count     = std::min(pixelData.size(), available);
            std::copy_n(_colors.cbegin() + offset, count, pixelData.begin());
        }

        // -----------------------------------------------------------------
        // Convenience overrides – single pixel (direct vector access)
        // -----------------------------------------------------------------
        void setPixelColor(size_t index, const TColor& color) override
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
        ResourceHandle<IProtocol<TColor>> _protocol;
        bool _dirty{false};
    };

    namespace factory
    {

        template <typename TProtocol>
        concept ProtocolLike = requires {
                                   typename TProtocol::ColorType;
                                   typename TProtocol::TransportCategory;
                               } && std::derived_from<TProtocol, IProtocol<typename TProtocol::ColorType>>;

        template <typename TTransport, typename TProtocol>
        concept ProtocolTransportCompatible = ProtocolLike<TProtocol> &&
                                              TransportLike<TTransport> &&
                                              TransportCategoryCompatible<typename TProtocol::TransportCategory,
                                                                          typename TTransport::TransportCategory>;

        template <typename TTransport, typename TProtocol>
            requires ProtocolTransportCompatible<TTransport, TProtocol>
        class ProtocolStateT
        {
        public:
            using TransportSettingsType = typename TTransport::TransportSettingsType;

            template <typename... TProtocolArgs>
            ProtocolStateT(TransportSettingsType transportSettings,
                           TProtocolArgs&&... protocolArgs)
                : _transport(std::move(transportSettings))
                , _protocol(std::forward<TProtocolArgs>(protocolArgs)..., _transport)
            {
            }

            TTransport& transport()
            {
                return _transport;
            }

            const TTransport& transport() const
            {
                return _transport;
            }

            TProtocol& protocol()
            {
                return _protocol;
            }

            const TProtocol& protocol() const
            {
                return _protocol;
            }

        private:
            TTransport _transport;
            TProtocol _protocol;
        };

        template <typename TTransport, typename TProtocol>
            requires ProtocolTransportCompatible<TTransport, TProtocol>
        class OwningPixelBusT
            : private ProtocolStateT<TTransport, TProtocol>
            , public PixelBusT<typename TProtocol::ColorType>
        {
        public:
            using ColorType = typename TProtocol::ColorType;
            using PixelBusType = PixelBusT<ColorType>;
            using ProtocolStateType = ProtocolStateT<TTransport, TProtocol>;
            using TransportSettingsType = typename TTransport::TransportSettingsType;

            template <typename... TProtocolArgs>
            OwningPixelBusT(uint16_t pixelCount,
                            TransportSettingsType transportSettings,
                            TProtocolArgs&&... protocolArgs)
                : ProtocolStateType(std::move(transportSettings), std::forward<TProtocolArgs>(protocolArgs)...)
                , PixelBusType(pixelCount, static_cast<ProtocolStateType&>(*this).protocol())
            {
            }

            TTransport& transport()
            {
                return static_cast<ProtocolStateType&>(*this).transport();
            }

            const TTransport& transport() const
            {
                return static_cast<const ProtocolStateType&>(*this).transport();
            }

            TProtocol& protocol()
            {
                return static_cast<ProtocolStateType&>(*this).protocol();
            }

            const TProtocol& protocol() const
            {
                return static_cast<const ProtocolStateType&>(*this).protocol();
            }
        };

        template <typename TTransport, typename TProtocol, typename... TProtocolArgs>
            requires ProtocolTransportCompatible<TTransport, TProtocol>
        OwningPixelBusT<TTransport, TProtocol> makeOwningPixelBus(uint16_t pixelCount,
                                                                   typename TTransport::TransportSettingsType transportSettings,
                                                                   TProtocolArgs&&... protocolArgs)
        {
            return OwningPixelBusT<TTransport, TProtocol>(pixelCount,
                                                          std::move(transportSettings),
                                                          std::forward<TProtocolArgs>(protocolArgs)...);
        }

    } // namespace factory

} // namespace npb
