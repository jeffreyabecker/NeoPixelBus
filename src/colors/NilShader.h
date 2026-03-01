#pragma once

#include "IShader.h"

namespace lw
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

} // namespace lw

