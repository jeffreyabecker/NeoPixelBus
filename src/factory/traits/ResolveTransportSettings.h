#pragma once

#include <utility>

#include "factory/traits/TransportDescriptorTraits.h"

namespace npb
{
namespace factory
{

    template <typename TTransportDesc,
              typename TTransportConfig>
    inline typename TransportDescriptorTraits<TTransportDesc>::SettingsType resolveTransportSettings(uint16_t pixelCount,
                                                                                                     const OneWireTiming *timing,
                                                                                                     TTransportConfig &&config)
    {
        using Traits = TransportDescriptorTraits<TTransportDesc>;
        return Traits::normalize(Traits::fromConfig(std::forward<TTransportConfig>(config), pixelCount, timing),
                                 pixelCount,
                                 timing);
    }

    template <typename TTransportDesc,
              typename TTransportConfig>
    inline typename TransportDescriptorTraits<TTransportDesc>::SettingsType resolveTransportSettings(uint16_t pixelCount,
                                                                                                     TTransportConfig &&config)
    {
        return resolveTransportSettings<TTransportDesc>(pixelCount,
                                                        static_cast<const OneWireTiming *>(nullptr),
                                                        std::forward<TTransportConfig>(config));
    }

    template <typename TTransportDesc>
    inline typename TransportDescriptorTraits<TTransportDesc>::SettingsType resolveTransportSettings(uint16_t pixelCount)
    {
        using Traits = TransportDescriptorTraits<TTransportDesc>;
        return Traits::normalize(Traits::defaultSettings(pixelCount),
                                 pixelCount,
                                 nullptr);
    }

    template <typename TTransportDesc>
    inline typename TransportDescriptorTraits<TTransportDesc>::SettingsType resolveTransportSettings(uint16_t pixelCount,
                                                                                                     const OneWireTiming *timing)
    {
        using Traits = TransportDescriptorTraits<TTransportDesc>;
        return Traits::normalize(Traits::defaultSettings(pixelCount),
                                 pixelCount,
                                 timing);
    }

} // namespace factory
} // namespace npb
