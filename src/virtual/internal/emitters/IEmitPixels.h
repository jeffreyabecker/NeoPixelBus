#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

namespace npb
{

class IEmitPixels
{
public:
    virtual ~IEmitPixels() = default;

    virtual void initialize() = 0;
    virtual void update(std::span<const uint8_t> data) = 0;
    virtual bool isReadyToUpdate() const = 0;
    virtual bool alwaysUpdate() const = 0;
};

} // namespace npb
