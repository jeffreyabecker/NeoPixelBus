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
    class OwningPixelBusT : public IPixelBus<TColor>
    {
    public:
        explicit OwningPixelBusT(IProtocol<TColor> *protocol,
                                 ITransport *transport = nullptr)
            : _ownedProtocol(protocol)
            , _ownedTransport(transport)
            , _protocol(_ownedProtocol.get())
            , _colors(protocol != nullptr ? protocol->pixelCount() : 0)
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

        size_t pixelCount() const override
        {
            return _colors.size();
        }

        span<TColor> colors()
        {
            return span<TColor>{_colors.data(), _colors.size()};
        }

        span<const TColor> colors() const
        {
            return span<const TColor>{_colors.data(), _colors.size()};
        }

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
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) const override
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
                            span<const TColor> pixelData) override
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
                            span<TColor> pixelData) const override
        {
            if (offset >= _colors.size())
            {
                return;
            }

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
        std::vector<TColor> _colors;
        bool _dirty{false};
    };

} // namespace npb


