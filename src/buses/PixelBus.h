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
                  BufferHolder<uint8_t> protocolBuffer,
                 Topology topology,
                 span<StrandExtent<TColor>> strands)
              : _rootBuffer(std::move(rootBuffer)), _shaderBuffer(std::move(shaderBuffer)), _protocolBuffer(std::move(protocolBuffer)), _topology(std::move(topology)), _strands(strands)
        {
        }

           PixelBus(BufferHolder<TColor> rootBuffer,
                  BufferHolder<TColor> shaderBuffer,
                  Topology topology,
                  span<StrandExtent<TColor>> strands)
              : PixelBus(std::move(rootBuffer),
                       std::move(shaderBuffer),
                       BufferHolder<uint8_t>::nil(),
                       std::move(topology),
                       strands)
           {
           }

        PixelBus(BufferHolder<TColor> rootBuffer,
                 Topology topology,
                 span<StrandExtent<TColor>> strands)
            : PixelBus(std::move(rootBuffer),
                       BufferHolder<TColor>::nil(),
                       BufferHolder<uint8_t>::nil(),
                       std::move(topology),
                       strands)
        {
        }

        PixelBus(BufferHolder<TColor> rootBuffer,
                 BufferHolder<TColor> shaderBuffer,
                  BufferHolder<uint8_t> protocolBuffer,
                 Topology topology)
              : _rootBuffer(std::move(rootBuffer)), _shaderBuffer(std::move(shaderBuffer)), _protocolBuffer(std::move(protocolBuffer)), _topology(std::move(topology)), _strands{}
           {
           }

           PixelBus(BufferHolder<TColor> rootBuffer,
                  BufferHolder<TColor> shaderBuffer,
                  Topology topology)
              : PixelBus(std::move(rootBuffer),
                       std::move(shaderBuffer),
                       BufferHolder<uint8_t>::nil(),
                       std::move(topology))
        {
        }

        PixelBus(BufferHolder<TColor> rootBuffer,
                 Topology topology)
            : PixelBus(std::move(rootBuffer),
                       BufferHolder<TColor>::nil(),
                       BufferHolder<uint8_t>::nil(),
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

                span<TColor> segment{root.data() + strand.offset, strand.length};
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

        bool isReadyToUpdate() const override
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

        void setBuffer(span<TColor> buffer)
        {
            _rootBuffer = BufferHolder<TColor>{buffer.size(), buffer.data(), false};
            _dirty = true;
        }

        uint16_t pixelCount() const
        {
            return static_cast<uint16_t>(_rootBuffer.size);
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

        void assignRootBufferHolder(BufferHolder<TColor> rootBuffer)
        {
            _rootBuffer = std::move(rootBuffer);
            _dirty = true;
        }

        void assignShaderBufferHolder(BufferHolder<TColor> shaderBuffer)
        {
            _shaderBuffer = std::move(shaderBuffer);
            _dirty = true;
        }

        void assignProtocolBufferHolder(BufferHolder<uint8_t> protocolBuffer)
        {
            _protocolBuffer = std::move(protocolBuffer);
        }

        BufferHolder<uint8_t>& protocolBufferHolder()
        {
            return _protocolBuffer;
        }

        const BufferHolder<uint8_t>& protocolBufferHolder() const
        {
            return _protocolBuffer;
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
        BufferHolder<uint8_t> _protocolBuffer;
        Topology _topology;
        span<StrandExtent<TColor>> _strands;
        bool _dirty{true};
    };

} // namespace lw
