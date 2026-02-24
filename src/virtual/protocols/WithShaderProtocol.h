#pragma once

#include <cstdint>
#include <vector>
#include <span>
#include <algorithm>
#include <concepts>
#include <type_traits>
#include <utility>

#include "IProtocol.h"
#include "../colors/IShader.h"
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
               ResourceHandle<IShader<TColor>> shader,
                        TArgs&&... args)
        : TProtocol(pixelCount, std::forward<TArgs>(args)...)
        , _shader{std::move(shader)}
        , _scratchColors(pixelCount)
    {
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
    ResourceHandle<IShader<TColor>> _shader;
    std::vector<TColor> _scratchColors;
};

} // namespace npb
