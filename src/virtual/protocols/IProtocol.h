#pragma once

#include <cstdint>
#include <cstddef>
#include <span>

#include "../colors/Color.h"
namespace npb
{

template <typename TColor>
class IProtocol
{
public:
    using ColorType = TColor;
    using SettingsType = void;
    virtual ~IProtocol() = default;

    virtual void initialize() = 0;
    virtual void update(std::span<const TColor> colors) = 0;
    virtual bool isReadyToUpdate() const = 0;
    virtual bool alwaysUpdate() const = 0;
};

} // namespace npb
