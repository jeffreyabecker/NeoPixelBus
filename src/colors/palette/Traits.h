#pragma once

#include <type_traits>
#include <utility>

#include "colors/palette/Types.h"

namespace lw
{
    template <typename TPaletteLike,
              typename = void>
    struct IsPaletteLike : std::false_type
    {
    };

    template <typename TPaletteLike>
    struct IsPaletteLike<TPaletteLike,
                         std::void_t<typename TPaletteLike::StopType,
                                     decltype(std::declval<const TPaletteLike &>().stops())>>
        : std::is_same<decltype(std::declval<const TPaletteLike &>().stops()),
                       span<const typename TPaletteLike::StopType>>
    {
    };

    template <typename TPaletteLike>
    using EnableIfPaletteLike = std::enable_if_t<IsPaletteLike<TPaletteLike>::value>;

} // namespace lw
