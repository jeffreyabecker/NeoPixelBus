#pragma once

#include <type_traits>

namespace npb
{
namespace factory
{

    template <typename TTransportDesc,
              typename = void>
    struct TransportDescriptorTraits;

    template <typename TTransportDesc>
    struct TransportDescriptorTraits<TTransportDesc,
                                     std::void_t<typename TTransportDesc::TransportSettingsType>>
    {
        using TransportType = TTransportDesc;
        using SettingsType = typename TransportType::TransportSettingsType;

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
