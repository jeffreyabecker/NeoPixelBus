#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

#include "../colors/Color.h"
namespace npb
{

class IProtocol
{
public:
    virtual ~IProtocol() = default;

    virtual void initialize() = 0;
    virtual void update(std::span<const Color> colors) = 0;
    virtual bool isReadyToUpdate() const = 0;
    virtual bool alwaysUpdate() const = 0;
};

} // namespace npb
