#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

#include "../colors/Color.h"
#include "IShader.h"

namespace npb
{

/// Chains multiple IShader instances into a single IShader.
/// apply() runs each shader in sequence over the entire span.
class ShaderChain : public IShader
{
public:
    explicit ShaderChain(std::span<IShader* const> shaders)
        : _shaders{shaders}
    {
    }



    void apply(std::span<Color> colors) override
    {
        for (auto* shader : _shaders)
        {
            shader->apply(colors);
        }
    }



private:
    std::span<IShader* const> _shaders;
};

} // namespace npb
