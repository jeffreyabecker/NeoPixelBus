#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

#include "Color.h"
#include "IShader.h"

namespace npb
{

    /// Chains multiple IShader instances into a single IShader.
    /// apply() runs each shader in sequence over the entire span.
    template<typename TColor>
    class ShaderChain : public IShader<TColor>
    {
    public:
        explicit ShaderChain(std::span<IShader<TColor> *const> shaders)
            : _shaders{shaders}
        {
        }

        void apply(std::span<TColor> colors) override
        {
            for (auto *shader : _shaders)
            {
                shader->apply(colors);
            }
        }

    private:
        std::span<IShader<TColor> *const> _shaders;
    };

} // namespace npb
