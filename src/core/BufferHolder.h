#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

#include "core/Compat.h"

namespace lw
{

    template <typename T>
    struct BufferHolder
    {
        static_assert(std::is_trivially_constructible<T>::value,
                      "BufferHolder<T> requires T to be trivially constructible");

        size_t size{0};
        T *buffer{nullptr};
        bool owns{false};

        BufferHolder() = default;

        BufferHolder(size_t bufferSize,
                     T *bufferPtr,
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
                buffer = new T[size]{};
            }
        }
        span<T> getSpan(size_t offset = 0, size_t size = std::numeric_limits<size_t>::max())
        {
            init();

            if (buffer == nullptr || offset >= this->size)
            {
                return span<T>{};
            }

            size_t count = std::min(size, this->size - offset);
            return span<T>{buffer + offset, count};
        }

        span<const T> getSpan(size_t offset = 0, size_t size = std::numeric_limits<size_t>::max()) const
        {
            const_cast<BufferHolder<T> *>(this)->init();

            if (buffer == nullptr || offset >= this->size)
            {
                return span<const T>{};
            }

            size_t count = std::min(size, this->size - offset);
            return span<const T>{buffer + offset, count};
        }
        constexpr static BufferHolder<T> empty()
        {
            return BufferHolder<T>{0, nullptr, true};
        }
        constexpr static BufferHolder<T> nil()
        {
            return BufferHolder<T>{0, nullptr, false};
        }
        static BufferHolder<T> create(size_t size)
        {
            T *buffer = new T[size]{};
            return BufferHolder<T>{size, buffer, true};
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

} // namespace lw
