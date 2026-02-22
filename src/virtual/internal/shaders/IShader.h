#pragma once

#include <cstdint>
#include <span>

#include "../colors/Color.h"

namespace npb
{

class IShader
{
public:
    virtual ~IShader() = default;
    virtual void begin(std::span<const Color> /*colors*/){}
    virtual const Color apply(uint16_t index, const Color color) = 0;
    virtual void end(){}
};



} // namespace npb
