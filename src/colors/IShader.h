#pragma once

#include <cstdint>

#include "Color.h"

namespace lw
{

template<typename TColor>
class IShader
{
public:
    virtual ~IShader() = default;

    virtual void apply(span<TColor> /*colors*/) = 0;
    
};

} // namespace lw

