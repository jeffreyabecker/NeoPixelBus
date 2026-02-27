#pragma once

#include <utility>

#include "factory/traits/ProtocolDescriptorTraits.h"

namespace npb
{
namespace factory
{

    template <typename TProtocolDesc>
    inline typename ProtocolDescriptorTraits<TProtocolDesc>::SettingsType resolveProtocolSettings(
        typename ProtocolDescriptorTraits<TProtocolDesc>::SettingsType settings)
    {
        using Traits = ProtocolDescriptorTraits<TProtocolDesc>;
        return Traits::normalize(std::move(settings));
    }

    template <typename TProtocolDesc,
              typename TProtocolConfig>
    inline typename ProtocolDescriptorTraits<TProtocolDesc>::SettingsType resolveProtocolSettings(TProtocolConfig &&config)
    {
        using Traits = ProtocolDescriptorTraits<TProtocolDesc>;
        return Traits::normalize(Traits::fromConfig(std::forward<TProtocolConfig>(config)));
    }

} // namespace factory
} // namespace npb
