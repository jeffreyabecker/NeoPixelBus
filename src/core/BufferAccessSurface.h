#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>

#include "core/BufferAccess.h"
#include "core/BufferHolder.h"
#include "core/Compat.h"

namespace lw
{

    template <typename TColor>
    class BufferAccessSurface : public IBufferAccess<TColor>
    {
    public:
        BufferAccessSurface(BufferHolder<TColor> &rootBuffer,
                            BufferHolder<TColor> &shaderBuffer,
                            BufferHolder<uint8_t> &protocolBuffer,
                            span<ProtocolSliceRange> protocolSlices)
            : _rootBuffer(rootBuffer)
            , _shaderBuffer(shaderBuffer)
            , _protocolBuffer(protocolBuffer)
            , _protocolSlices(protocolSlices)
        {
        }

        BufferAccessSurface(BufferHolder<TColor> &rootBuffer,
                            BufferHolder<TColor> &shaderBuffer,
                            BufferHolder<uint8_t> &protocolBuffer)
            : BufferAccessSurface(rootBuffer,
                                  shaderBuffer,
                                  protocolBuffer,
                                  span<ProtocolSliceRange>{})
        {
        }

        span<TColor> rootPixels() override
        {
            return _rootBuffer.getSpan();
        }

        span<const TColor> rootPixels() const override
        {
            return _rootBuffer.getSpan();
        }

        span<TColor> shaderScratch() override
        {
            return _shaderBuffer.getSpan();
        }

        span<const TColor> shaderScratch() const override
        {
            return _shaderBuffer.getSpan();
        }

        span<uint8_t> protocolArena() override
        {
            return _protocolBuffer.getSpan();
        }

        span<const uint8_t> protocolArena() const override
        {
            return _protocolBuffer.getSpan();
        }

        size_t protocolSliceCount() const override
        {
            return _protocolSlices.size();
        }

        span<uint8_t> protocolSlice(size_t strandIndex) override
        {
            auto arena = _protocolBuffer.getSpan();
            return protocolSliceFrom(arena, strandIndex);
        }

        span<const uint8_t> protocolSlice(size_t strandIndex) const override
        {
            auto arena = _protocolBuffer.getSpan();
            return protocolSliceFrom(arena, strandIndex);
        }

        bool assignProtocolSliceLayout(size_t strandIndex,
                           ProtocolSliceRange slice)
        {
            if (strandIndex >= _protocolSlices.size())
            {
                return false;
            }

            _protocolSlices[strandIndex] = slice;
            return true;
        }

        bool reserveProtocolArenaBytes(size_t minimumBytes)
        {
            if (_protocolBuffer.size >= minimumBytes)
            {
                return true;
            }

            if (!_protocolBuffer.owns)
            {
                return false;
            }

            _protocolBuffer = BufferHolder<uint8_t>{minimumBytes, nullptr, true};
            clearSlices();
            return true;
        }

        void setProtocolSlices(span<ProtocolSliceRange> protocolSlices)
        {
            _protocolSlices = protocolSlices;
        }

    private:
        template <typename TByte>
        span<TByte> protocolSliceFrom(span<TByte> arena,
                                      size_t strandIndex) const
        {
            if (strandIndex >= _protocolSlices.size())
            {
                return span<TByte>{};
            }

            const auto slice = _protocolSlices[strandIndex];
            if (slice.offset >= arena.size())
            {
                return span<TByte>{};
            }

            const size_t available = arena.size() - slice.offset;
            const size_t byteCount = std::min(available, slice.size);
            return span<TByte>{arena.data() + slice.offset, byteCount};
        }

        void clearSlices()
        {
            for (auto &slice : _protocolSlices)
            {
                slice = ProtocolSliceRange{};
            }
        }

        BufferHolder<TColor> &_rootBuffer;
        BufferHolder<TColor> &_shaderBuffer;
        BufferHolder<uint8_t> &_protocolBuffer;
        span<ProtocolSliceRange> _protocolSlices;
    };

} // namespace lw
