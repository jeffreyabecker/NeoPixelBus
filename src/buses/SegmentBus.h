#pragma once

#include <cstddef>
#include <cstdint>
#include <algorithm>

#include "core/IPixelBus.h"

namespace npb
{

    // -------------------------------------------------------------------
    // SegmentBus ? non-owning subsegment view of an IPixelBus
    //
    // Exposes a contiguous range of pixels from a parent bus as an
    // independent IPixelBus.  Does NOT own the parent ? the parent
    // must outlive this SegmentBus.
    //
    // Useful for assigning logical zones on a single physical strip
    // to different animation controllers, effect engines, etc.
    //
    // Usage:
    //   PixelBus strip(60, protocol);
    //   SegmentBus<> head(strip, 0, 20);      // pixels 0..19
    //   SegmentBus<> body(strip, 20, 30);     // pixels 20..49
    //   SegmentBus<> tail(strip, 50, 10);     // pixels 50..59
    //
    //   head.pixelBuffer()[5] = Color(255,0,0);  // ? strip pixel 5
    //   tail.pixelBuffer()[0] = Color(0,0,255);  // ? strip pixel 50
    // -------------------------------------------------------------------
    template <typename TColor>
    class SegmentBus : public IAssignableBufferBus<TColor>
    {
    public:
        /// @param parent  The parent bus to create a view into.
        /// @param offset  Starting pixel index in the parent bus.
        /// @param length  Number of pixels in this segment.
                SegmentBus(IPixelBus<TColor>& parent, size_t offset, size_t length)
            : _parent(parent),
              _offset(offset),
              _length(length)
        {
        }

        // --- IPixelBus lifecycle ----------------------------------------
        // Lifecycle methods delegate to the parent.  If multiple
        // SegmentBus instances share one parent, begin()/show() will
        // be called once per segment ? safe but the caller may prefer
        // to call them once on the parent directly.

        void begin() override
        {
            _parent.begin();
        }

        void show() override
        {
            _parent.show();
        }

        bool canShow() const override
        {
            return _parent.canShow();
        }

        uint16_t pixelCount() const override
        {
            return static_cast<uint16_t>(_length);
        }

        void setBuffer(span<TColor> buffer) override
        {
            _assignedBuffer = buffer;
            _hasAssignedBuffer = true;
        }

        span<TColor> pixelBuffer() override
        {
            if (_hasAssignedBuffer)
            {
                return _assignedBuffer;
            }

            auto parentBuffer = _parent.pixelBuffer();
            if (_offset >= parentBuffer.size())
            {
                return span<TColor>{};
            }

            size_t available = parentBuffer.size() - _offset;
            size_t count = std::min(_length, available);
            return span<TColor>{parentBuffer.data() + _offset, count};
        }

        span<const TColor> pixelBuffer() const override
        {
            if (_hasAssignedBuffer)
            {
                return span<const TColor>{_assignedBuffer.data(), _assignedBuffer.size()};
            }

            auto parentBuffer = _parent.pixelBuffer();
            if (_offset >= parentBuffer.size())
            {
                return span<const TColor>{};
            }

            size_t available = parentBuffer.size() - _offset;
            size_t count = std::min(_length, available);
            return span<const TColor>{parentBuffer.data() + _offset, count};
        }

    private:
        IPixelBus<TColor>& _parent;
        size_t _offset;
        size_t _length;
        span<TColor> _assignedBuffer;
        bool _hasAssignedBuffer = false;
    };

    // ---------------------------------------------------------------
    // Free function ? creates a SegmentBus view returned by value.
    // ---------------------------------------------------------------
    template <typename TColor>
    inline SegmentBus<TColor> getSegment(IPixelBus<TColor>& bus,
                                          size_t offset,
                                          size_t count)
    {
        return SegmentBus<TColor>(bus, offset, count);
    }

} // namespace npb


