#pragma once

#include <type_traits>

#include "protocols/NilProtocol.h"
#include "factory/descriptors/ProtocolDescriptors.h"
#include "factory/traits/ProtocolDescriptorTraits.h"

namespace lw
{
namespace factory
{

    struct NoneOptions
    {
    };

    template <typename TColor>
    struct ProtocolDescriptorTraits<descriptors::None<TColor>, void>
        : ProtocolDescriptorTraitDefaults<typename lw::NilProtocol<TColor>::SettingsType>
    {
        using ProtocolType = lw::NilProtocol<TColor>;
        using SettingsType = typename ProtocolType::SettingsType;
        using ColorType = typename ProtocolType::ColorType;
        using Base = ProtocolDescriptorTraitDefaults<SettingsType>;
        using Base::defaultSettings;
        using Base::fromConfig;
        using Base::mutateTransportSettings;

        static SettingsType normalize(SettingsType settings)
        {
            return settings;
        }

        static SettingsType fromConfig(const NoneOptions &)
        {
            return defaultSettings();
        }
    };

} // namespace factory
} // namespace lw
