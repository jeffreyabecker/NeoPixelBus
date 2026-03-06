#pragma once

#include <type_traits>

#include "colors/Color.h"

namespace lw::colors
{

    template <typename TColor,
              typename TValue = typename TColor::ComponentType,
              typename = void>
    struct ChannelSource;

    template <typename TColor, typename TValue>
    struct ChannelSource<TColor,
                         TValue,
                         std::enable_if_t<(TColor::ChannelCount == 3)>>
    {
        TValue R{};
        TValue G{};
        TValue B{};
    };

    template <typename TColor, typename TValue>
    struct ChannelSource<TColor,
                         TValue,
                         std::enable_if_t<(TColor::ChannelCount == 4)>>
    {
        TValue R{};
        TValue G{};
        TValue B{};
        TValue W{};
    };

    template <typename TColor, typename TValue>
    struct ChannelSource<TColor,
                         TValue,
                         std::enable_if_t<(TColor::ChannelCount == 5)>>
    {
        TValue R{};
        TValue G{};
        TValue B{};
        TValue W{};
        TValue C{};
    };

} // namespace lw::colors

namespace lw
{

template <typename TColor,
          typename TValue = typename TColor::ComponentType,
          typename Enable = void>
using ChannelSource = colors::ChannelSource<TColor, TValue, Enable>;

} // namespace lw
