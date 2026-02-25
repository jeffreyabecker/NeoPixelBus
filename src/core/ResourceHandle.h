#pragma once

#include <memory>
#include <concepts>
#include <type_traits>

namespace npb
{

    /// Holds a pointer to T that is either owned (destroyed on destruction) or
    /// borrowed (caller manages lifetime).  Move-only.  Zero overhead beyond
    /// one pointer + one bool.
    ///
    /// Ownership modes:
    ///   Owning    ? constructed from std::unique_ptr<U>.  Calls delete on destruction.
    ///   Borrowing ? constructed from U& reference.  No cleanup.
    ///   Null      ? default-constructed or from nullptr.  No cleanup.
    ///
    /// This enables PixelBus to be the root owner of an entire resource tree
    /// (protocol -> bus, shader) when constructed dynamically via factories,
    /// while still supporting the classic Arduino pattern of static globals
    /// with externally managed lifetimes.
    ///
    template <typename T>
    class ResourceHandle
    {
    public:
        /// Null handle ? no resource.
        ResourceHandle() noexcept
            : _ptr{nullptr}, _owning{false}
        {
        }

        /// Null handle from nullptr literal.
        ResourceHandle(std::nullptr_t) noexcept
            : _ptr{nullptr}, _owning{false}
        {
        }

        /// Owning handle ? takes ownership from unique_ptr.
        /// Accepts unique_ptr<U> where U* is implicitly convertible to T*.
        template <typename U = T>
            requires(std::convertible_to<U *, T *> &&
                     std::has_virtual_destructor_v<T>)
        ResourceHandle(std::unique_ptr<U> p) noexcept
            : _ptr{p.release()}, _owning{true}
        {
        }

        /// Borrowing handle ? references an existing lvalue.
        /// Accepts U& where U* is implicitly convertible to T*.
        /// The caller must ensure the referenced object outlives this handle.
        template <typename U>
            requires std::convertible_to<U *, T *>
        ResourceHandle(U &ref) noexcept
            : _ptr{&ref}, _owning{false}
        {
        }

        ~ResourceHandle()
        {
            if (_owning)
            {
                destroyOwned(_ptr);
            }
        }

        // ---- move-only ----

        ResourceHandle(ResourceHandle &&other) noexcept
            : _ptr{other._ptr}, _owning{other._owning}
        {
            other._ptr = nullptr;
            other._owning = false;
        }

        ResourceHandle &operator=(ResourceHandle &&other) noexcept
        {
            if (this != &other)
            {
                if (_owning)
                {
                    destroyOwned(_ptr);
                }
                _ptr = other._ptr;
                _owning = other._owning;
                other._ptr = nullptr;
                other._owning = false;
            }
            return *this;
        }

        ResourceHandle(const ResourceHandle &) = delete;
        ResourceHandle &operator=(const ResourceHandle &) = delete;

        // ---- observers ----

        /// True if this handle references a valid object.
        explicit operator bool() const noexcept { return _ptr != nullptr; }

        friend bool operator==(const ResourceHandle &lhs, std::nullptr_t) noexcept { return lhs._ptr == nullptr; }
        friend bool operator!=(const ResourceHandle &lhs, std::nullptr_t) noexcept { return lhs._ptr != nullptr; }
        friend bool operator==(std::nullptr_t, const ResourceHandle &rhs) noexcept { return rhs._ptr == nullptr; }
        friend bool operator!=(std::nullptr_t, const ResourceHandle &rhs) noexcept { return rhs._ptr != nullptr; }

        T &operator*() noexcept { return *_ptr; }
        const T &operator*() const noexcept { return *_ptr; }

        T *operator->() noexcept { return _ptr; }
        const T *operator->() const noexcept { return _ptr; }

        T &get() noexcept { return *_ptr; }
        const T &get() const noexcept { return *_ptr; }

    private:
        static void destroyOwned(T *ptr) noexcept
        {
            if constexpr (std::has_virtual_destructor_v<T>)
            {
                delete ptr;
            }
            else
            {
                (void)ptr;
            }
        }

        T *_ptr;
        bool _owning;
    };

} // namespace npb

