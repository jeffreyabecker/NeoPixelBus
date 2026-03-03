#pragma once

#include <cstddef>
#include <utility>
#include <vector>

#include "buses/impl/BufferAccessor.h"
#include "buses/PixelBus.h"

namespace lw
{

    inline Topology normalizeOwningBusTopology(Topology topology, size_t rootLength)
    {
        if (topology.empty())
        {
            return Topology::linear(rootLength);
        }

        return topology;
    }

    template <typename TColor>
    class OwningBufferContext
    {
    protected:
        OwningBufferContext(size_t rootPixelCount,
                            size_t shaderPixelCount,
                            std::vector<size_t> protocolSizes)
            : _protocolSizes(std::move(protocolSizes))
            , _buffer(rootPixelCount,
                      shaderPixelCount,
                      span<size_t>{_protocolSizes.data(), _protocolSizes.size()})
        {
        }

        BufferAccessor<TColor> &bufferAccess()
        {
            return _buffer;
        }

        const BufferAccessor<TColor> &bufferAccess() const
        {
            return _buffer;
        }

    private:
        std::vector<size_t> _protocolSizes;
        BufferAccessor<TColor> _buffer;
    };

} // namespace lw
