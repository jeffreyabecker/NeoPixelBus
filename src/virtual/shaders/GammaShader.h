#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

#include "../colors/Color.h"
#include "IShader.h"

namespace npb
{

    template <typename T_GAMMA>
    class GammaShader : public IShader
    {
    public:
        void apply(std::span<Color> colors) override
        {
            for (auto &color : colors)
            {
                for (size_t ch = 0; ch < Color::ChannelCount; ++ch)
                {
                    color[ch] = T_GAMMA::correct(color[ch]);
                }
            }
        }
    };

} // namespace npb
