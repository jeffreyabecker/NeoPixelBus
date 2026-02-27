#pragma once

#include <type_traits>

namespace npb
{
namespace factory
{

    template <typename TProtocolDesc,
              typename = void>
    struct ProtocolDescriptorTraits;

    template <typename TProtocolDesc>
    struct ProtocolDescriptorTraits<TProtocolDesc,
                                    std::void_t<typename TProtocolDesc::SettingsType,
                                                typename TProtocolDesc::ColorType>>
    {
        using ProtocolType = TProtocolDesc;
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
