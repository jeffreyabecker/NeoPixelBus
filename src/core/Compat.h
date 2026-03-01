#pragma once

#include <cstddef>
#include <type_traits>

#if defined(__cpp_lib_span) && (__cpp_lib_span >= 202002L)
#include <span>
#define LW_HAS_STD_SPAN 1
#elif defined(__has_include)
#if __has_include(<span>)
#include <span>
#if defined(__cpp_lib_span) && (__cpp_lib_span >= 202002L)
#define LW_HAS_STD_SPAN 1
#endif
#endif
#endif

#if !defined(LW_HAS_STD_SPAN)
#include "third_party/tcb/span.hpp"
#endif

namespace lw
{

#if defined(__cpp_lib_remove_cvref) && (__cpp_lib_remove_cvref >= 201711L)
    template <typename T>
    using remove_cvref_t = std::remove_cvref_t<T>;
#else
    template <typename T>
    using remove_cvref_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
#endif

#if defined(LW_HAS_STD_SPAN)
    static constexpr std::size_t dynamic_extent = std::dynamic_extent;
    template <typename T, std::size_t Extent = std::dynamic_extent>
    using span = std::span<T, Extent>;
#else
    static constexpr std::size_t dynamic_extent = tcb::dynamic_extent;

    template <typename T, std::size_t Extent = dynamic_extent>
    using span = tcb::span<T, Extent>;
#endif

} // namespace lw
