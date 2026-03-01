#pragma once

#include <utility>

#include "factory/traits/ProtocolDescriptorTraits.h"
#include "factory/traits/TransportDescriptorTraits.h"

namespace lw
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
        return Traits::normalize(Traits::fromConfig(std::forward<TTransportConfig>(config), pixelCount),
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

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocolSettings,
              typename TTransportConfig>
    inline typename TransportDescriptorTraits<TTransportDesc>::SettingsType resolveTransportSettingsForProtocol(uint16_t pixelCount,
                                                                                                                const TProtocolSettings &protocolSettings,
                                                                                                                const OneWireTiming *timing,
                                                                                                                TTransportConfig &&config)
    {
        using ProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>;
        using TransportTraits = TransportDescriptorTraits<TTransportDesc>;

        auto transportSettings = TransportTraits::fromConfig(std::forward<TTransportConfig>(config),
                                     pixelCount);
        ProtocolTraits::mutateTransportSettings(pixelCount,
                                                protocolSettings,
                                                transportSettings);

        return TransportTraits::normalize(std::move(transportSettings),
                                          pixelCount,
                                          timing);
    }

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocolSettings,
              typename TTransportConfig>
    inline typename TransportDescriptorTraits<TTransportDesc>::SettingsType resolveTransportSettingsForProtocol(uint16_t pixelCount,
                                                                                                                const TProtocolSettings &protocolSettings,
                                                                                                                TTransportConfig &&config)
    {
        return resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                                                   protocolSettings,
                                                                                   static_cast<const OneWireTiming *>(nullptr),
                                                                                   std::forward<TTransportConfig>(config));
    }

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocolSettings>
    inline typename TransportDescriptorTraits<TTransportDesc>::SettingsType resolveTransportSettingsForProtocol(uint16_t pixelCount,
                                                                                                                const TProtocolSettings &protocolSettings,
                                                                                                                const OneWireTiming *timing)
    {
        using ProtocolTraits = ProtocolDescriptorTraits<TProtocolDesc>;
        using TransportTraits = TransportDescriptorTraits<TTransportDesc>;

        auto transportSettings = TransportTraits::defaultSettings(pixelCount);
        ProtocolTraits::mutateTransportSettings(pixelCount,
                                                protocolSettings,
                                                transportSettings);

        return TransportTraits::normalize(std::move(transportSettings),
                                          pixelCount,
                                          timing);
    }

    template <typename TProtocolDesc,
              typename TTransportDesc,
              typename TProtocolSettings>
    inline typename TransportDescriptorTraits<TTransportDesc>::SettingsType resolveTransportSettingsForProtocol(uint16_t pixelCount,
                                                                                                                const TProtocolSettings &protocolSettings)
    {
        return resolveTransportSettingsForProtocol<TProtocolDesc, TTransportDesc>(pixelCount,
                                                                                   protocolSettings,
                                                                                   static_cast<const OneWireTiming *>(nullptr));
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
} // namespace lw
