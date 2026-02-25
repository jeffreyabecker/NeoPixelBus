#pragma once

#include "IShader.h"

namespace npb
{

    template <typename TColor>
    class NilShader : public IShader<TColor>
    {
    public:
        void apply(span<TColor>) override
        {
        }
    };

} // namespace npb

