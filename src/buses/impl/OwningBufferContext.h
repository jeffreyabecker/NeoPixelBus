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
                            std::vector<size_t> protocolSizes,
                            uint8_t *buffer = nullptr,
                            ssize_t bufferSize = -1,
                            bool ownsBuffer = true)
            : _protocolSizes(std::move(protocolSizes))
            , _buffer(rootPixelCount,
                      shaderPixelCount,
                      span<size_t>{_protocolSizes.data(), _protocolSizes.size()},
                      selectBuffer(rootPixelCount,
                                   shaderPixelCount,
                                   _protocolSizes,
                                   buffer,
                                   bufferSize),
                      selectOwns(rootPixelCount,
                                 shaderPixelCount,
                                 _protocolSizes,
                                 buffer,
                                 bufferSize,
                                 ownsBuffer))
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
        static bool hasUsableExternalBuffer(size_t rootPixelCount,
                                            size_t shaderPixelCount,
                                            const std::vector<size_t> &protocolSizes,
                                            uint8_t *buffer,
                                            ssize_t bufferSize)
        {
            if (buffer == nullptr)
            {
                return false;
            }

            if (bufferSize < 0)
            {
                return true;
            }

            size_t protocolTotal = 0;
            for (size_t size : protocolSizes)
            {
                protocolTotal += size;
            }

            const size_t required = BufferAccessor<TColor>::totalBytes(rootPixelCount,
                                                                       shaderPixelCount,
                                                                       protocolTotal);
            return static_cast<size_t>(bufferSize) >= required;
        }

        static uint8_t *selectBuffer(size_t rootPixelCount,
                                     size_t shaderPixelCount,
                                     const std::vector<size_t> &protocolSizes,
                                     uint8_t *buffer,
                                     ssize_t bufferSize)
        {
            return hasUsableExternalBuffer(rootPixelCount,
                                           shaderPixelCount,
                                           protocolSizes,
                                           buffer,
                                           bufferSize)
                       ? buffer
                       : nullptr;
        }

        static bool selectOwns(size_t rootPixelCount,
                               size_t shaderPixelCount,
                               const std::vector<size_t> &protocolSizes,
                               uint8_t *buffer,
                               ssize_t bufferSize,
                               bool ownsBuffer)
        {
            return hasUsableExternalBuffer(rootPixelCount,
                                           shaderPixelCount,
                                           protocolSizes,
                                           buffer,
                                           bufferSize)
                       ? ownsBuffer
                       : true;
        }

        std::vector<size_t> _protocolSizes;
        BufferAccessor<TColor> _buffer;
    };

} // namespace lw
