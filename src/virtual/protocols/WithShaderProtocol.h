#pragma once

#include <cstdint>
#include <vector>
#include <span>
#include <algorithm>
#include <concepts>
#include <utility>

#include "IProtocol.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"

namespace npb
{

template<typename TProtocol>
    requires std::derived_from<TProtocol, IProtocol>
class WithShaderProtocol : public TProtocol
{
public:
    template<typename... TArgs>
    WithShaderProtocol(uint16_t pixelCount,
                       ResourceHandle<IShader> shader,
                       TArgs&&... args)
        : TProtocol(pixelCount, std::forward<TArgs>(args)...)
        , _shader{std::move(shader)}
        , _scratchColors(pixelCount)
    {
    }

    void update(std::span<const Color> colors) override
    {
        std::span<const Color> source = colors;
        if (nullptr != _shader)
        {
            std::copy(colors.begin(), colors.end(), _scratchColors.begin());
            _shader->apply(_scratchColors);
            source = _scratchColors;
        }

        TProtocol::update(source);
    }

private:
    ResourceHandle<IShader> _shader;
    std::vector<Color> _scratchColors;
};

} // namespace npb
