#pragma once

#include "IShader.h"

namespace npb
{

    template <typename TColor = Color>
    class NilShader : public IShader<TColor>
    {
    public:
        void apply(std::span<TColor>) override
        {
        }
    };

} // namespace npb
