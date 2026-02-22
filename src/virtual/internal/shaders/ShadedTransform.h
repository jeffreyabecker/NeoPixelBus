#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

#include "../colors/Color.h"
#include "IShader.h"

namespace npb
{

/// Chains multiple IShader instances into a single IShader.
/// apply() runs each shader in order on the color for a given pixel index.
class ShaderChain : public IShader
{
public:
    explicit ShaderChain(std::span<IShader* const> shaders)
        : _shaders{shaders}
    {
    }

    void begin(std::span<const Color> colors) override
    {
        for (auto* shader : _shaders)
        {
            shader->begin(colors);
        }
    }

    const Color apply(uint16_t index, const Color color) override
    {
        Color result = color;
        for (auto* shader : _shaders)
        {
            result = shader->apply(index, result);
        }
        return result;
    }

    void end() override
    {
        for (auto* shader : _shaders)
        {
            shader->end();
        }
    }

private:
    std::span<IShader* const> _shaders;
};

} // namespace npb
