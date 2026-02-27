#pragma once

#include "protocols/DotStarProtocol.h"
#include "factory/descriptors/ProtocolDescriptors.h"
#include "factory/traits/ProtocolDescriptorTraits.h"

namespace npb
{
namespace factory
{

    template <>
    struct ProtocolDescriptorTraits<descriptors::APA102, void>
    {
        using ProtocolType = npb::DotStarProtocol;
        using SettingsType = typename ProtocolType::SettingsType;
        using ColorType = typename ProtocolType::ColorType;

        static SettingsType defaultSettings()
        {
            return SettingsType{};
        }

        static SettingsType normalize(SettingsType settings)
        {
            return settings;
        }

        static SettingsType fromConfig(SettingsType settings)
        {
            return settings;
        }
    };

} // namespace factory
} // namespace npb
