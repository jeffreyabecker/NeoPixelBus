#pragma once

#include <cstdint>

#include "Color.h"

namespace lw::shaders
{

template<typename TColor>
class IShader
{
public:
    virtual ~IShader() = default;

    virtual void apply(span<TColor> /*colors*/) = 0;
    
};

} // namespace lw::shaders

namespace lw
{

template <typename TColor>
using IShader = shaders::IShader<TColor>;

} // namespace lw

