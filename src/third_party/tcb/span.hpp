#pragma once

#include <array>
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace tcb
{

    static constexpr std::size_t dynamic_extent = static_cast<std::size_t>(-1);

    template <typename T, std::size_t Extent = dynamic_extent>
    class span
    {
    public:
        using element_type = T;
        using value_type = typename std::remove_cv<T>::type;
        using size_type = std::size_t;
        using pointer = T *;
        using const_pointer = const T *;
        using reference = T &;
        using const_reference = const T &;
        using iterator = pointer;
        using const_iterator = const_pointer;

        static constexpr size_type extent = Extent;

        constexpr span() noexcept
            : _data(nullptr),
              _size(0)
        {
        }

        constexpr span(pointer ptr, size_type count) noexcept
            : _data(ptr),
              _size(count)
        {
        }

        template <std::size_t N,
                  typename = typename std::enable_if<Extent == dynamic_extent || N == Extent>::type>
        constexpr span(element_type (&arr)[N]) noexcept
            : _data(arr),
              _size(N)
        {
        }

        template <typename U,
                  std::size_t N,
                  typename = typename std::enable_if<(Extent == dynamic_extent || N == Extent) &&
                                                     std::is_convertible<U (*)[], T (*)[]>::value>::type>
        constexpr span(std::array<U, N> &arr) noexcept
            : _data(arr.data()),
              _size(N)
        {
        }

        template <typename U,
                  std::size_t N,
                  typename = typename std::enable_if<(Extent == dynamic_extent || N == Extent) &&
                                                     std::is_convertible<const U (*)[], T (*)[]>::value>::type>
        constexpr span(const std::array<U, N> &arr) noexcept
            : _data(arr.data()),
              _size(N)
        {
        }

        template <typename U,
                  std::size_t OtherExtent,
                  typename = typename std::enable_if<(Extent == dynamic_extent || OtherExtent == Extent || OtherExtent == dynamic_extent) &&
                                                     std::is_convertible<U (*)[], T (*)[]>::value>::type>
        constexpr span(const span<U, OtherExtent> &other) noexcept
            : _data(other.data()),
              _size(other.size())
        {
        }

        constexpr iterator begin() const noexcept
        {
            return _data;
        }

        constexpr iterator end() const noexcept
        {
            return _data + _size;
        }

        constexpr reference operator[](size_type idx) const
        {
            return _data[idx];
        }

        constexpr reference front() const
        {
            return _data[0];
        }

        constexpr reference back() const
        {
            return _data[_size - 1];
        }

        constexpr pointer data() const noexcept
        {
            return _data;
        }

        constexpr size_type size() const noexcept
        {
            return _size;
        }

        constexpr size_type size_bytes() const noexcept
        {
            return _size * sizeof(element_type);
        }

        constexpr bool empty() const noexcept
        {
            return _size == 0;
        }

    private:
        pointer _data;
        size_type _size;
    };

} // namespace tcb
