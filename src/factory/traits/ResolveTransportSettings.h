#pragma once

#include <utility>

#include "factory/traits/TransportDescriptorTraits.h"

namespace npb
{
namespace factory
{

    template <typename TTransportDesc,
              typename TTransportConfig>
    inline typename TransportDescriptorTraits<TTransportDesc>::SettingsType resolveTransportSettings(TTransportConfig &&config)
    {
        using Traits = TransportDescriptorTraits<TTransportDesc>;
        return Traits::normalize(Traits::fromConfig(std::forward<TTransportConfig>(config)));
    }

    template <typename TTransportDesc>
    inline typename TransportDescriptorTraits<TTransportDesc>::SettingsType resolveTransportSettings()
    {
        using Traits = TransportDescriptorTraits<TTransportDesc>;
        return Traits::normalize(Traits::defaultSettings());
    }

} // namespace factory
} // namespace npb
