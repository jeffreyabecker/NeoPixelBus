#pragma once

#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <memory>

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
    //   PixelBus strip(60, protocol);
    //   SegmentBus head(strip, 0, 20);        // pixels 0..19
    //   SegmentBus body(strip, 20, 30);       // pixels 20..49
    //   SegmentBus tail(strip, 50, 10);       // pixels 50..59
    //
    //   head.setPixelColor(5, Color(255,0,0));  // → strip pixel 5
    //   tail.setPixelColor(0, Color(0,0,255));  // → strip pixel 50
    // -------------------------------------------------------------------
    template <typename TColor = Color>
    class SegmentBusT : public IPixelBusT<TColor>
    {
    public:
        /// @param parent  The parent bus to create a view into.
        /// @param offset  Starting pixel index in the parent bus.
        /// @param length  Number of pixels in this segment.
        SegmentBusT(IPixelBusT<TColor>& parent, size_t offset, size_t length)
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
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) override
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
                            ColorIteratorT<TColor> first,
                            ColorIteratorT<TColor> last) const override
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
        IPixelBusT<TColor>& _parent;
        size_t _offset;
        size_t _length;
    };

    using SegmentBus = SegmentBusT<Color>;

    // ---------------------------------------------------------------
    // Free function — creates a SegmentBus view returned as a
    // unique_ptr<IPixelBus>, keeping IPixelBus free of any
    // dependency on SegmentBus.
    // ---------------------------------------------------------------
    template <typename TColor = Color>
    inline std::unique_ptr<IPixelBusT<TColor>> getSegmentT(IPixelBusT<TColor>& bus,
                                                           size_t offset,
                                                           size_t count)
    {
        return std::make_unique<SegmentBusT<TColor>>(bus, offset, count);
    }

    inline std::unique_ptr<IPixelBus> getSegment(IPixelBus& bus,
                                                 size_t offset,
                                                 size_t count)
    {
        return getSegmentT<Color>(bus, offset, count);
    }

} // namespace npb
