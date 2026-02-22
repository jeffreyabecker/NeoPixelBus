#pragma once

#include <cstdint>
#include <span>

#include "../colors/Color.h"

namespace npb
{

class IShader
{
public:
    virtual ~IShader() = default;

    virtual void apply(std::span<Color> /*colors*/) = 0;
    
};

class NilShader : public IShader
{   
public:
    void apply(std::span<Color> /*colors*/) override
    {
        // NilShader does nothing
    }
};


} // namespace npb
