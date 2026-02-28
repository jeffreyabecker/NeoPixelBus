#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <algorithm>

#include "core/IPixelBus.h"
#include "protocols/IProtocol.h"
#include "transports/ITransport.h"

namespace npb
{

    template <typename TColor>
    class OwningPixelBusT : public IAssignableBufferBus<TColor>
    {
    public:
        explicit OwningPixelBusT(IProtocol<TColor> *protocol,
                                 ITransport *transport = nullptr)
            : _ownedProtocol(protocol)
            , _ownedTransport(transport)
            , _protocol(_ownedProtocol.get())
            , _pixelCount(protocol != nullptr ? protocol->pixelCount() : 0)
            , _ownedColors(protocol != nullptr ? protocol->pixelCount() : 0)
            , _colors(_ownedColors.data(), _ownedColors.size())
        {
        }

        OwningPixelBusT(const OwningPixelBusT &) = delete;
        OwningPixelBusT &operator=(const OwningPixelBusT &) = delete;
        OwningPixelBusT(OwningPixelBusT &&) = default;
        OwningPixelBusT &operator=(OwningPixelBusT &&) = default;

        void begin() override
        {
            if (_protocol != nullptr)
            {
                _protocol->initialize();
            }
        }

        void show() override
        {
            if (_protocol == nullptr)
            {
                return;
            }

            if (!_dirty && !_protocol->alwaysUpdate())
            {
                return;
            }

            _protocol->update(span<const TColor>{_colors.data(), _colors.size()});
            _dirty = false;
        }

        bool canShow() const override
        {
            return _protocol != nullptr && _protocol->isReadyToUpdate();
        }

        void setBuffer(span<TColor> buffer) override
        {
            _colors = buffer;
            _dirty = true;
        }

        uint16_t pixelCount() const override
        {
            return _pixelCount;
        }

        span<TColor> pixelBuffer() override
        {
            _dirty = true;
            return span<TColor>{_colors.data(), _colors.size()};
        }

        span<const TColor> pixelBuffer() const override
        {
            return span<const TColor>{_colors.data(), _colors.size()};
        }

        span<TColor> colors()
        {
            return pixelBuffer();
        }

        span<const TColor> colors() const
        {
            return pixelBuffer();
        }

        IProtocol<TColor> *protocol()
        {
            return _ownedProtocol.get();
        }

        const IProtocol<TColor> *protocol() const
        {
            return _ownedProtocol.get();
        }

        ITransport *transport()
        {
            return _ownedTransport.get();
        }

        const ITransport *transport() const
        {
            return _ownedTransport.get();
        }

    private:
        std::unique_ptr<IProtocol<TColor>> _ownedProtocol;
        std::unique_ptr<ITransport> _ownedTransport;
        IProtocol<TColor> *_protocol{nullptr};
        uint16_t _pixelCount{0};
        std::vector<TColor> _ownedColors;
        span<TColor> _colors;
        bool _dirty{false};
    };

} // namespace npb


