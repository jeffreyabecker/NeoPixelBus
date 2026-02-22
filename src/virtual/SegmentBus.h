#pragma once

#include <cstddef>
#include <cstdint>
#include <algorithm>

#include "IPixelBus.h"

namespace npb
{

    // -------------------------------------------------------------------
    // SegmentBus — non-owning subsegment view of an IPixelBus
    //
    // Exposes a contiguous range of pixels from a parent bus as an
    // independent IPixelBus.  Does NOT own the parent — the parent
    // must outlive this SegmentBus.
    //
    // Useful for assigning logical zones on a single physical strip
    // to different animation controllers, effect engines, etc.
    //
    // Usage:
    //   PixelBus strip(60, emitter);
    //   SegmentBus head(strip, 0, 20);        // pixels 0..19
    //   SegmentBus body(strip, 20, 30);       // pixels 20..49
    //   SegmentBus tail(strip, 50, 10);       // pixels 50..59
    //
    //   head.setPixelColor(5, Color(255,0,0));  // → strip pixel 5
    //   tail.setPixelColor(0, Color(0,0,255));  // → strip pixel 50
    // -------------------------------------------------------------------
    class SegmentBus : public IPixelBus
    {
    public:
        /// @param parent  The parent bus to create a view into.
        /// @param offset  Starting pixel index in the parent bus.
        /// @param length  Number of pixels in this segment.
        SegmentBus(IPixelBus& parent, size_t offset, size_t length)
            : _parent(parent),
              _offset(offset),
              _length(length)
        {
        }

        // --- IPixelBus lifecycle ----------------------------------------
        // Lifecycle methods delegate to the parent.  If multiple
        // SegmentBus instances share one parent, begin()/show() will
        // be called once per segment — safe but the caller may prefer
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

        size_t pixelCount() const override
        {
            return _length;
        }

        // --- IPixelBus primary interface --------------------------------

        void setPixelColors(size_t offset,
                            ColorIterator first,
                            ColorIterator last) override
        {
            // Clamp to our segment bounds
            auto count = static_cast<size_t>(last - first);
            if (offset >= _length)
            {
                return;
            }
            size_t available = _length - offset;
            if (count > available)
            {
                count = available;
            }

            // Delegate to parent with translated offset
            _parent.setPixelColors(_offset + offset, first,
                                   first + static_cast<std::ptrdiff_t>(count));
        }

        void getPixelColors(size_t offset,
                            ColorIterator first,
                            ColorIterator last) const override
        {
            auto count = static_cast<size_t>(last - first);
            if (offset >= _length)
            {
                return;
            }
            size_t available = _length - offset;
            if (count > available)
            {
                count = available;
            }

            _parent.getPixelColors(_offset + offset, first,
                                   first + static_cast<std::ptrdiff_t>(count));
        }

    private:
        IPixelBus& _parent;
        size_t _offset;
        size_t _length;
    };

} // namespace npb
