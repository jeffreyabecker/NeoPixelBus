#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

#include "../colors/Color.h"

namespace npb
{

class ITransformColorToBytes
{
public:
    virtual ~ITransformColorToBytes() = default;

    virtual void apply(std::span<uint8_t> pixels,
                       std::span<const Color> colors) = 0;

    virtual size_t bytesNeeded(size_t pixelCount) const = 0;
};

} // namespace npb
