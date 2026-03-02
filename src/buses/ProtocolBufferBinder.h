#pragma once

#include <cstddef>
#include <cstdint>

#include "buses/PixelBus.h"
#include "core/BufferAccess.h"

namespace lw
{

    template <typename TColor>
    class ProtocolBufferBinder
    {
    public:
        static size_t calculateRequiredProtocolBytes(span<StrandExtent<TColor>> strands)
        {
            size_t totalBytes = 0;
            for (const auto &strand : strands)
            {
                if (strand.protocol == nullptr)
                {
                    continue;
                }

                totalBytes += strand.protocol->requiredBufferSizeBytes();
            }

            return totalBytes;
        }

        template <typename TBufferSurface>
        static bool bind(span<StrandExtent<TColor>> strands,
                 TBufferSurface &bufferAccess)
        {
            size_t totalBytes = 0;
            for (const auto &strand : strands)
            {
                if (strand.protocol == nullptr)
                {
                    continue;
                }

                strand.protocol->bindTransport(strand.transport);
                totalBytes += strand.protocol->requiredBufferSizeBytes();
            }

            if (!bufferAccess.reserveProtocolArenaBytes(totalBytes))
            {
                return false;
            }

            auto protocolArena = bufferAccess.protocolArena();
            if (protocolArena.size() < totalBytes)
            {
                return false;
            }

            size_t runningOffset = 0;
            for (size_t strandIndex = 0; strandIndex < strands.size(); ++strandIndex)
            {
                const auto &strand = strands[strandIndex];
                if (strand.protocol == nullptr)
                {
                    bufferAccess.assignProtocolSliceLayout(strandIndex, ProtocolSliceRange{});
                    continue;
                }

                const size_t protocolBytes = strand.protocol->requiredBufferSizeBytes();
                if (protocolBytes == 0)
                {
                    bufferAccess.assignProtocolSliceLayout(strandIndex,
                                                           ProtocolSliceRange{runningOffset, 0});
                    strand.protocol->setBuffer(span<uint8_t>{});
                    continue;
                }

                if ((runningOffset + protocolBytes) > protocolArena.size())
                {
                    return false;
                }

                auto slice = span<uint8_t>{protocolArena.data() + runningOffset,
                                           protocolBytes};
                bufferAccess.assignProtocolSliceLayout(strandIndex,
                                                       ProtocolSliceRange{runningOffset, protocolBytes});
                strand.protocol->setBuffer(slice);
                runningOffset += protocolBytes;
            }

            return true;
        }
    };

} // namespace lw
