#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <utility>
#include <vector>

#include "core/BufferAccess.h"
#include "core/Compat.h"

namespace lw
{

    template <typename TColor>
    class FixedBufferAccessor : public IBufferAccess<TColor>
    {
    public:
        FixedBufferAccessor(const FixedBufferAccessor &) = delete;
        FixedBufferAccessor &operator=(const FixedBufferAccessor &) = delete;
        FixedBufferAccessor(FixedBufferAccessor &&other) noexcept
        {
            moveFrom(std::move(other));
        }

        FixedBufferAccessor &operator=(FixedBufferAccessor &&other) noexcept
        {
            if (this != &other)
            {
                releaseStorage();
                moveFrom(std::move(other));
            }

            return *this;
        }

        static constexpr size_t pixelBytes(size_t pixelCount)
        {
            return pixelCount * sizeof(TColor);
        }

        static constexpr size_t totalBytes(size_t rootPixelCount,
                                           size_t shaderPixelCount,
                                           size_t protocolTotalBytes)
        {
            return pixelBytes(rootPixelCount) + pixelBytes(shaderPixelCount) + protocolTotalBytes;
        }

        template <size_t TCount>
        static constexpr size_t protocolTotalBytes(const std::array<size_t, TCount> &protocolSizes)
        {
            size_t total = 0;
            for (size_t size : protocolSizes)
            {
                total += size;
            }
            return total;
        }

        FixedBufferAccessor(size_t rootPixelCount,
                    size_t shaderPixelCount,
                    std::initializer_list<size_t> protocolSizes,
                    uint8_t *buffer = nullptr,
                    bool ownsBuffer = true)
            : _rootPixels(rootPixelCount)
            , _shaderPixels(shaderPixelCount)
            , _protocolSizes(protocolSizes)
            , _buffer(buffer)
            , _ownsBuffer(ownsBuffer)
            , _initialized(buffer != nullptr)
        {
            computeLayout();
        }

        FixedBufferAccessor(size_t rootPixelCount,
                    size_t shaderPixelCount,
                    span<size_t> protocolSizes,
                    uint8_t *buffer = nullptr,
                    bool ownsBuffer = true)
            : _rootPixels(rootPixelCount)
            , _shaderPixels(shaderPixelCount)
            , _protocolSizes(protocolSizes.begin(), protocolSizes.end())
            , _buffer(buffer)
            , _ownsBuffer(ownsBuffer)
            , _initialized(buffer != nullptr)
        {
            computeLayout();
        }

        ~FixedBufferAccessor() override
        {
            releaseStorage();
        }

        span<TColor> rootPixels() override
        {
            ensureInit();
            if (!_initialized || _rootBytes == 0)
            {
                return span<TColor>{};
            }
            auto ptr = reinterpret_cast<TColor *>(_buffer + _rootOffset);
            return span<TColor>{ptr, _rootPixels};
        }

        span<const TColor> rootPixels() const override
        {
            const_cast<FixedBufferAccessor<TColor> *>(this)->ensureInit();
            if (!_initialized || _rootBytes == 0)
            {
                return span<const TColor>{};
            }
            auto ptr = reinterpret_cast<const TColor *>(_buffer + _rootOffset);
            return span<const TColor>{ptr, _rootPixels};
        }

        span<TColor> shaderScratch() override
        {
            ensureInit();
            if (!_initialized || _shaderBytes == 0)
            {
                return span<TColor>{};
            }
            auto ptr = reinterpret_cast<TColor *>(_buffer + _shaderOffset);
            return span<TColor>{ptr, _shaderPixels};
        }

        span<const TColor> shaderScratch() const override
        {
            const_cast<FixedBufferAccessor<TColor> *>(this)->ensureInit();
            if (!_initialized || _shaderBytes == 0)
            {
                return span<const TColor>{};
            }
            auto ptr = reinterpret_cast<const TColor *>(_buffer + _shaderOffset);
            return span<const TColor>{ptr, _shaderPixels};
        }

        size_t protocolSliceCount() const override
        {
            return _protocolSizes.size();
        }

        size_t totalBytes() const
        {
            return _totalBytes;
        }

        span<uint8_t> protocolSlice(size_t strandIndex) override
        {
            ensureInit();
            if (!_initialized || _protocolTotalBytes == 0)
            {
                return span<uint8_t>{};
            }
            span<uint8_t> arena{_buffer + _protocolOffset, _protocolTotalBytes};
            return protocolSliceFrom(arena, strandIndex);
        }

        span<const uint8_t> protocolSlice(size_t strandIndex) const override
        {
            const_cast<FixedBufferAccessor<TColor> *>(this)->ensureInit();
            if (!_initialized || _protocolTotalBytes == 0)
            {
                return span<const uint8_t>{};
            }
            span<const uint8_t> arena{_buffer + _protocolOffset, _protocolTotalBytes};
            return protocolSliceFrom(arena, strandIndex);
        }

    private:
        void computeLayout()
        {
            _rootBytes = pixelBytes(_rootPixels);
            _shaderBytes = pixelBytes(_shaderPixels);
            _protocolTotalBytes = 0;
            for (auto s : _protocolSizes)
            {
                _protocolTotalBytes += s;
            }

            _rootOffset = 0;
            _shaderOffset = _rootOffset + _rootBytes;
            _protocolOffset = _shaderOffset + _shaderBytes;

            _slices.clear();
            _slices.resize(_protocolSizes.size());
            size_t running = 0;
            for (size_t i = 0; i < _protocolSizes.size(); ++i)
            {
                _slices[i] = ProtocolSliceRange{running, _protocolSizes[i]};
                running += _protocolSizes[i];
            }

            _totalBytes = totalBytes(_rootPixels, _shaderPixels, _protocolTotalBytes);
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
            if (_buffer != nullptr)
            {
                return;
            }

            if (_totalBytes == 0)
            {
                return;
            }

            _buffer = new uint8_t[_totalBytes];
            std::fill_n(_buffer, _totalBytes, static_cast<uint8_t>(0));

            size_t running = 0;
            for (size_t i = 0; i < _protocolSizes.size(); ++i)
            {
                _slices[i].offset = running;
                _slices[i].size = _protocolSizes[i];
                running += _protocolSizes[i];
            }
        }

        void releaseStorage()
        {
            if (_ownsBuffer && _buffer != nullptr)
            {
                delete[] _buffer;
            }

            _buffer = nullptr;
            _initialized = false;
        }

        void moveFrom(FixedBufferAccessor &&other)
        {
            _rootPixels = other._rootPixels;
            _shaderPixels = other._shaderPixels;
            _protocolSizes = std::move(other._protocolSizes);

            _rootBytes = other._rootBytes;
            _shaderBytes = other._shaderBytes;
            _protocolTotalBytes = other._protocolTotalBytes;
            _totalBytes = other._totalBytes;

            _rootOffset = other._rootOffset;
            _shaderOffset = other._shaderOffset;
            _protocolOffset = other._protocolOffset;

            _buffer = other._buffer;
            _ownsBuffer = other._ownsBuffer;
            _slices = std::move(other._slices);
            _initialized = other._initialized;

            other._rootPixels = 0;
            other._shaderPixels = 0;
            other._rootBytes = 0;
            other._shaderBytes = 0;
            other._protocolTotalBytes = 0;
            other._totalBytes = 0;
            other._rootOffset = 0;
            other._shaderOffset = 0;
            other._protocolOffset = 0;
            other._buffer = nullptr;
            other._ownsBuffer = true;
            other._initialized = false;
            other._protocolSizes.clear();
            other._slices.clear();
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

        uint8_t *_buffer{nullptr};
        bool _ownsBuffer{true};
        std::vector<ProtocolSliceRange> _slices{};
        bool _initialized{false};
    };

} // namespace lw
