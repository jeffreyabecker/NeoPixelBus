#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>

namespace npb
{

    template <typename TWritable>
    concept Writable = requires(TWritable &writable,
                                const uint8_t *data,
                                size_t length) {
                           {
                               writable.write(data, length)
                               } -> std::convertible_to<size_t>;
                       };

} // namespace npb

