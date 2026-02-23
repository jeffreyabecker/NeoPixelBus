#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <memory>

#include <Print.h>

#include "PixieProtocol.h"
#include "../buses/PrintClockDataTransport.h"

namespace npb
{

#ifndef NPB_DEPRECATED
    #if defined(__has_cpp_attribute)
        #if __has_cpp_attribute(deprecated)
            #define NPB_DEPRECATED(msg) [[deprecated(msg)]]
        #else
            #define NPB_DEPRECATED(msg)
        #endif
    #elif defined(_MSC_VER)
        #define NPB_DEPRECATED(msg) __declspec(deprecated(msg))
    #elif defined(__GNUC__) || defined(__clang__)
        #define NPB_DEPRECATED(msg) __attribute__((deprecated(msg)))
    #else
        #define NPB_DEPRECATED(msg)
    #endif
#endif

struct PixieStreamProtocolSettings
{
    Print& output;
    std::array<uint8_t, 3> channelOrder = {Color::IdxR, Color::IdxG, Color::IdxB};
};

// Backward-compatibility wrapper. Prefer PixieProtocol + PrintClockDataTransport.
class NPB_DEPRECATED("Use PixieProtocol with PrintClockDataTransport instead of PixieStreamProtocol") PixieStreamProtocol : public PixieProtocol
{
public:
    PixieStreamProtocol(uint16_t pixelCount,
                       ResourceHandle<IShader> shader,
                       PixieStreamProtocolSettings settings)
        : PixieProtocol(
            pixelCount,
            std::move(shader),
            PixieProtocolSettings{
                ResourceHandle<IClockDataTransport>{
                    std::make_unique<PrintClockDataTransport>(settings.output)},
                settings.channelOrder})
    {
    }
};

#undef NPB_DEPRECATED

} // namespace npb
