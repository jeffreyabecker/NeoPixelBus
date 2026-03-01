#pragma once

#include <cstddef>
#include <string>

namespace lw
{

    namespace ChannelOrder
    {

#define DECLARE_CHANNEL_ORDER(name) \
        struct name \
        { \
            static constexpr const char *value = #name; \
            static constexpr size_t length = std::char_traits<char>::length(value); \
            constexpr operator const char *() const { return value; } \
        }

        DECLARE_CHANNEL_ORDER(RGB);
        DECLARE_CHANNEL_ORDER(GRB);
        DECLARE_CHANNEL_ORDER(BGR);
        DECLARE_CHANNEL_ORDER(RGBW);
        DECLARE_CHANNEL_ORDER(GRBW);
        DECLARE_CHANNEL_ORDER(BGRW);
        DECLARE_CHANNEL_ORDER(WRGB);
        DECLARE_CHANNEL_ORDER(W);
        DECLARE_CHANNEL_ORDER(CW);
        DECLARE_CHANNEL_ORDER(RGBCW);
        DECLARE_CHANNEL_ORDER(GRBCW);
        DECLARE_CHANNEL_ORDER(BGRCW);

#undef DECLARE_CHANNEL_ORDER

    } // namespace ChannelOrder

} // namespace lw
