#pragma once

#include "IShader.h"

namespace lw::shaders
{

    template <typename TColor>
    class NilShader : public IShader<TColor>
    {
    public:
        using ColorType = TColor;
        void apply(span<TColor>) override
        {
        }
    };

} // namespace lw::shaders

namespace lw
{

template <typename TColor>
using NilShader = shaders::NilShader<TColor>;

} // namespace lw

