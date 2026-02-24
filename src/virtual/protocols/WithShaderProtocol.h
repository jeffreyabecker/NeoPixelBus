#pragma once

#include <cstdint>
#include <vector>
#include <span>
#include <algorithm>
#include <concepts>
#include <type_traits>
#include <utility>

#include "IProtocol.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"

namespace npb
{

template<typename TColor = Color, typename TProtocol = IProtocol<TColor>>
    requires std::derived_from<TProtocol, IProtocol<TColor>>
class WithShader : public TProtocol
{
public:
    template<typename... TArgs>
    WithShader(uint16_t pixelCount,
                        ResourceHandle<IShader> shader,
                        TArgs&&... args)
        : TProtocol(pixelCount, std::forward<TArgs>(args)...)
        , _shader{std::move(shader)}
        , _scratchColors(pixelCount)
    {
        static_assert(std::is_same<TColor, Color>::value,
                      "WithShader<TColor, TProtocol> requires IShader migration; use Color/Rgbcw8Color until shader templating is complete");
    }

    void update(std::span<const TColor> colors) override
    {
        std::span<const TColor> source = colors;
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
    std::vector<TColor> _scratchColors;
};

template<typename TProtocol>
using WithShaderProtocol = WithShader<Color, TProtocol>;

} // namespace npb
