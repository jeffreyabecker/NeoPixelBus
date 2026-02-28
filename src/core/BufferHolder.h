#pragma once

#include <algorithm>
#include <cstddef>
#include <utility>

#include "core/Compat.h"

namespace npb
{

    template <typename TColor>
    struct BufferHolder
    {
        size_t size{0};
        TColor *buffer{nullptr};
        bool owns{false};

        BufferHolder() = default;

        BufferHolder(size_t bufferSize,
                     TColor *bufferPtr,
                     bool ownsBuffer)
            : size(bufferSize), buffer(bufferPtr), owns(ownsBuffer)
        {
        }

        BufferHolder(const BufferHolder &) = delete;
        BufferHolder &operator=(const BufferHolder &) = delete;

        BufferHolder(BufferHolder &&other) noexcept
            : size(other.size), buffer(other.buffer), owns(other.owns)
        {
            other.size = 0;
            other.buffer = nullptr;
            other.owns = false;
        }

        BufferHolder &operator=(BufferHolder &&other) noexcept
        {
            if (this != &other)
            {
                release();

                size = other.size;
                buffer = other.buffer;
                owns = other.owns;

                other.size = 0;
                other.buffer = nullptr;
                other.owns = false;
            }

            return *this;
        }

        ~BufferHolder()
        {
            release();
        }

        void init()
        {
            if (buffer == nullptr && owns && size > 0)
            {
                buffer = new TColor[size]{};
            }
        }

        span<TColor> getSpan(size_t offset, size_t size)
        {
            if (buffer == nullptr || offset >= this->size)
            {
                return span<TColor>{};
            }

            size_t count = std::min(size, this->size - offset);
            return span<TColor>{buffer + offset, count};
        }

        span<const TColor> getSpan(size_t offset, size_t size) const
        {
            if (buffer == nullptr || offset >= this->size)
            {
                return span<const TColor>{};
            }

            size_t count = std::min(size, this->size - offset);
            return span<const TColor>{buffer + offset, count};
        }
        constexpr static BufferHolder<TColor> empty()
        {
            return BufferHolder<TColor>{0, nullptr, true};
        }

        bool operator==(const BufferHolder &other) const
        {
            return size == other.size &&
                   buffer == other.buffer &&
                   owns == other.owns;
        }

        bool operator!=(const BufferHolder &other) const
        {
            return !(*this == other);
        }

    private:
        void release()
        {
            if (owns && buffer != nullptr)
            {
                delete[] buffer;
            }

            buffer = nullptr;
            size = 0;
            owns = false;
        }
    };

} // namespace npb
