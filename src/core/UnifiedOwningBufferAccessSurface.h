#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <vector>

#include "core/BufferAccess.h"
#include "core/Compat.h"

namespace lw
{

    // Uses a single contiguous arena for root pixels, shader scratch, and protocol bytes.
    // Arena may be externally provided, and allocation is delayed until first access.
    template <typename TColor>
    class UnifiedOwningBufferAccessSurface : public IBufferAccess<TColor>
    {
    public:
        // protocolSizes: each entry is the number of protocol bytes for that strand (in order)
        UnifiedOwningBufferAccessSurface(size_t rootPixelCount,
                                         size_t shaderPixelCount,
                                         std::initializer_list<size_t> protocolSizes)
            : _rootPixels(rootPixelCount)
            , _shaderPixels(shaderPixelCount)
            , _protocolSizes(protocolSizes)
        {
            computeLayout();
        }

        UnifiedOwningBufferAccessSurface(size_t rootPixelCount,
                                         size_t shaderPixelCount,
                                         span<size_t> protocolSizes)
            : _rootPixels(rootPixelCount)
            , _shaderPixels(shaderPixelCount)
            , _protocolSizes(protocolSizes.begin(), protocolSizes.end())
        {
            computeLayout();
        }

        span<TColor> rootPixels() override
        {
            ensureInit();
            if (!_initialized || _rootBytes == 0)
            {
                return span<TColor>{};
            }
            auto ptr = reinterpret_cast<TColor *>(_storage.data() + _rootOffset);
            return span<TColor>{ptr, _rootPixels};
        }

        span<const TColor> rootPixels() const override
        {
            const_cast<UnifiedOwningBufferAccessSurface<TColor> *>(this)->ensureInit();
            if (!_initialized || _rootBytes == 0)
            {
                return span<const TColor>{};
            }
            auto ptr = reinterpret_cast<const TColor *>(_storage.data() + _rootOffset);
            return span<const TColor>{ptr, _rootPixels};
        }

        span<TColor> shaderScratch() override
        {
            ensureInit();
            if (!_initialized || _shaderBytes == 0)
            {
                return span<TColor>{};
            }
            auto ptr = reinterpret_cast<TColor *>(_storage.data() + _shaderOffset);
            return span<TColor>{ptr, _shaderPixels};
        }

        span<const TColor> shaderScratch() const override
        {
            const_cast<UnifiedOwningBufferAccessSurface<TColor> *>(this)->ensureInit();
            if (!_initialized || _shaderBytes == 0)
            {
                return span<const TColor>{};
            }
            auto ptr = reinterpret_cast<const TColor *>(_storage.data() + _shaderOffset);
            return span<const TColor>{ptr, _shaderPixels};
        }

        // The public seam does not expose the full protocol arena directly.
        // Per-strand slices are available via `protocolSlice()` which will
        // return a span for the requested strand based on pre-declared
        // slice sizes and assigned offsets.

        size_t protocolSliceCount() const override
        {
            return _protocolSizes.size();
        }

        span<uint8_t> protocolSlice(size_t strandIndex) override
        {
            ensureInit();
            if (!_initialized || _protocolTotalBytes == 0)
            {
                return span<uint8_t>{};
            }
            span<uint8_t> arena{_storage.data() + _protocolOffset, _protocolTotalBytes};
            return protocolSliceFrom(arena, strandIndex);
        }

        span<const uint8_t> protocolSlice(size_t strandIndex) const override
        {
            const_cast<UnifiedOwningBufferAccessSurface<TColor> *>(this)->ensureInit();
            if (!_initialized || _protocolTotalBytes == 0)
            {
                return span<const uint8_t>{};
            }
            span<const uint8_t> arena{_storage.data() + _protocolOffset, _protocolTotalBytes};
            return protocolSliceFrom(arena, strandIndex);
        }



    private:
        void computeLayout()
        {
            _rootBytes = _rootPixels * sizeof(TColor);
            _shaderBytes = _shaderPixels * sizeof(TColor);
            _protocolTotalBytes = 0;
            for (auto s : _protocolSizes)
            {
                _protocolTotalBytes += s;
            }

            _rootOffset = 0;
            _shaderOffset = _rootOffset + _rootBytes;
            _protocolOffset = _shaderOffset + _shaderBytes;

            // prepare slices with sizes but offsets will be assigned by binder normally
            _slices.clear();
            _slices.resize(_protocolSizes.size());
            size_t running = 0;
            for (size_t i = 0; i < _protocolSizes.size(); ++i)
            {
                _slices[i] = ProtocolSliceRange{running, _protocolSizes[i]};
                running += _protocolSizes[i];
            }

            _totalBytes = _rootBytes + _shaderBytes + _protocolTotalBytes;
        }

        void ensureInit()
        {
            if (_initialized)
            {
                return;
            }
            allocateStorage();
            _initialized = true;
        }

        void allocateStorage()
        {
            if (_totalBytes == 0)
            {
                return;
            }

            _storage.clear();
            _storage.resize(_totalBytes);
            std::fill(_storage.begin(), _storage.end(), static_cast<uint8_t>(0));

            // Recompute slices offsets relative to protocolOffset so direct protocolSliceFrom works
            size_t running = 0;
            for (size_t i = 0; i < _protocolSizes.size(); ++i)
            {
                _slices[i].offset = running;
                _slices[i].size = _protocolSizes[i];
                running += _protocolSizes[i];
            }
        }

        template <typename TByte>
        span<TByte> protocolSliceFrom(span<TByte> arena,
                                      size_t strandIndex) const
        {
            if (strandIndex >= _slices.size())
            {
                return span<TByte>{};
            }

            const auto slice = _slices[strandIndex];
            if (slice.offset >= arena.size())
            {
                return span<TByte>{};
            }

            const size_t available = arena.size() - slice.offset;
            const size_t byteCount = std::min(available, slice.size);
            return span<TByte>{arena.data() + slice.offset, byteCount};
        }

        size_t _rootPixels{0};
        size_t _shaderPixels{0};
        std::vector<size_t> _protocolSizes{};

        size_t _rootBytes{0};
        size_t _shaderBytes{0};
        size_t _protocolTotalBytes{0};
        size_t _totalBytes{0};

        size_t _rootOffset{0};
        size_t _shaderOffset{0};
        size_t _protocolOffset{0};

        std::vector<uint8_t> _storage{};
        std::vector<ProtocolSliceRange> _slices{};
        bool _initialized{false};
    };

} // namespace lw
