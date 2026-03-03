#pragma once

#include <cstddef>
#include <cstdint>
#include <algorithm>

#include "buses/Topology.h"
#include "colors/IShader.h"
#include "core/IPixelBus.h"
#include "protocols/IProtocol.h"
#include "transports/ITransport.h"
#include "core/BufferAccess.h"

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
    private:
        IBufferAccess<TColor> &_accessor;
        Topology _topology;
        span<StrandExtent<TColor>> _strands;
        bool _dirty{true};

    public:
        PixelBus(IBufferAccess<TColor> &accessor,
                 Topology topology,
                 span<StrandExtent<TColor>> strands)
            : _accessor(accessor), _topology(std::move(topology)), _strands(strands)
        {
        }

        void begin() override
        {
            _accessor.init();
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
                    strand.protocol->begin();
                }
            }
        }

        void show() override
        {

            if (!_dirty && !anyAlwaysUpdate())
            {
                return;
            }

            auto root = _accessor.rootPixels();
            auto shaderScratch = _accessor.shaderScratch();
            for (size_t strandIndex = 0; strandIndex < _strands.size(); ++strandIndex)
            {
                const auto &strand = _strands[strandIndex];
                if (strand.length == 0)
                {
                    continue;
                }

                if (strand.transport == nullptr || !strand.transport->isReadyToUpdate())
                {
                    continue;
                }

                span<TColor> segment{root.data() + strand.offset, strand.length};
                if (strand.shader != nullptr)
                {
                    if (shaderScratch.size() >= strand.length)
                    {
                        span<TColor> shaderSegment{shaderScratch.data(), strand.length};
                        std::copy_n(segment.begin(), strand.length, shaderSegment.begin());
                        segment = shaderSegment;
                    }
                    strand.shader->apply(segment);
                }
                auto strandBuffer = _accessor.protocolSlice(strandIndex);

                strand.protocol->update(static_cast<span<const TColor>>(segment), strandBuffer);

                if (strand.transport != nullptr && strandBuffer.size() > 0)
                {
                    strand.transport->beginTransaction();
                    strand.transport->transmitBytes(strandBuffer);
                    strand.transport->endTransaction();
                }
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

                                   return true;
                               });
        }

        span<TColor> pixelBuffer() override
        {
            _dirty = true;
            return _accessor.rootPixels();
        }

        void setBuffer(span<TColor> buffer)
        {
            auto root = _accessor.rootPixels();
            const size_t copyCount = std::min(root.size(), buffer.size());
            if (copyCount > 0)
            {
                std::copy_n(buffer.begin(), copyCount, root.begin());
            }
            _dirty = true;
        }

        uint16_t pixelCount() const
        {
            return static_cast<uint16_t>(_accessor.rootPixels().size());
        }

        span<const TColor> pixelBuffer() const override
        {
            return _accessor.rootPixels();
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
    };

} // namespace lw
