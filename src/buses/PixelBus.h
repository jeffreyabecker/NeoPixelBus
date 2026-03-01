#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <algorithm>

#include "buses/Topology.h"
#include "colors/IShader.h"
#include "core/BufferHolder.h"
#include "core/IPixelBus.h"
#include "protocols/IProtocol.h"
#include "transports/ITransport.h"

namespace lw
{

    template <typename TColor>
    struct StrandExtent
    {
        IProtocol<TColor> *protocol = nullptr;
        ITransport *transport = nullptr;
        IShader<TColor> *shader = nullptr;
        size_t offset = 0;
        size_t length = 0;
    };

    template <typename TColor>
    class PixelBus : public IPixelBus<TColor>
    {
    public:
        PixelBus(BufferHolder<TColor> rootBuffer,
                 BufferHolder<TColor> shaderBuffer,
                 Topology topology,
                 span<StrandExtent<TColor>> strands)
            : _rootBuffer(std::move(rootBuffer)), _shaderBuffer(std::move(shaderBuffer)), _topology(std::move(topology)), _strands(strands)
        {
        }

        PixelBus(BufferHolder<TColor> rootBuffer,
                 Topology topology,
                 span<StrandExtent<TColor>> strands)
            : PixelBus(std::move(rootBuffer),
                       BufferHolder<TColor>::nil(),
                       std::move(topology),
                       strands)
        {
        }

        PixelBus(BufferHolder<TColor> rootBuffer,
                 BufferHolder<TColor> shaderBuffer,
                 Topology topology)
            : _rootBuffer(std::move(rootBuffer)), _shaderBuffer(std::move(shaderBuffer)), _topology(std::move(topology)), _strands{}
        {
        }

        PixelBus(BufferHolder<TColor> rootBuffer,
                 Topology topology)
            : PixelBus(std::move(rootBuffer),
                       BufferHolder<TColor>::nil(),
                       std::move(topology))
        {
        }

        void begin() override
        {
            _rootBuffer.init();
            _shaderBuffer.init();
            for (const auto &strand : _strands)
            {
                if (strand.length == 0)
                {
                    continue;
                }

                if (strand.transport != nullptr)
                {
                    strand.transport->begin();
                }

                if (strand.protocol != nullptr)
                {
                    strand.protocol->initialize();
                }
            }
        }

        void show() override
        {
            if (!_dirty && !anyAlwaysUpdate())
            {
                return;
            }

            auto root = _rootBuffer.getSpan();

            for (const auto &strand : _strands)
            {
                if (strand.length == 0)
                {
                    continue;
                }

                if (!strand.protocol->isReadyToUpdate())
                {
                    continue;
                }

                span<TColor> segment = root.subspan(strand.offset, strand.length);
                if (strand.shader != nullptr)
                {
                    if (_shaderBuffer.size >= strand.length)
                    {
                        span<TColor> shaderSegment = _shaderBuffer.getSpan(0, strand.length);
                        std::copy_n(segment.begin(), strand.length, shaderSegment.begin());
                        segment = shaderSegment;
                    }
                    strand.shader->apply(segment);
                }

                strand.protocol->update(static_cast<span<const TColor>>(segment));
            }

            _dirty = false;
        }

        bool canShow() const override
        {
            return std::all_of(_strands.begin(),
                               _strands.end(),
                               [](const StrandExtent<TColor> &strand)
                               {
                                   if (strand.transport != nullptr && !strand.transport->isReadyToUpdate())
                                   {
                                       return false;
                                   }

                                   return strand.protocol->isReadyToUpdate();
                               });
        }

        span<TColor> pixelBuffer() override
        {
            _dirty = true;
            return _rootBuffer.getSpan();
        }

        span<const TColor> pixelBuffer() const override
        {
            return _rootBuffer.getSpan();
        }

        const Topology *topologyOrNull() const override
        {
            return &_topology;
        }

    protected:
        void setStrands(span<StrandExtent<TColor>> strands)
        {
            _strands = strands;
            _dirty = true;
        }

    private:
        bool anyAlwaysUpdate() const
        {
            return std::any_of(_strands.begin(),
                               _strands.end(),
                               [](const StrandExtent<TColor> &strand)
                               {
                                   return strand.protocol->alwaysUpdate();
                               });
        }

        BufferHolder<TColor> _rootBuffer;
        BufferHolder<TColor> _shaderBuffer;
        Topology _topology;
        span<StrandExtent<TColor>> _strands;
        bool _dirty{true};
    };

    template <typename TColor>
    class OwningPixelBusT : public IAssignableBufferBus<TColor>
    {
    public:
        explicit OwningPixelBusT(IProtocol<TColor> *protocol,
                                 ITransport *transport = nullptr)
            : _ownedProtocol(protocol), _ownedTransport(transport), _protocol(_ownedProtocol.get()), _pixelCount(protocol != nullptr ? protocol->pixelCount() : 0), _ownedColors(protocol != nullptr ? protocol->pixelCount() : 0), _colors(_ownedColors.data(), _ownedColors.size())
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

} // namespace lw
