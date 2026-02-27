#pragma once

#include <type_traits>
#include <utility>

namespace npb
{
namespace factory
{

    template <typename TSettingsType>
    struct ShaderDescriptorTraitDefaults
    {
        using SettingsType = TSettingsType;

        static SettingsType defaultSettings()
        {
            return SettingsType{};
        }

        static SettingsType fromConfig(SettingsType settings)
        {
            return settings;
        }
    };

    template <typename TShaderDesc,
              typename = void>
    struct ShaderDescriptorTraits;

    template <typename TShaderDesc>
    struct ShaderDescriptorTraits<TShaderDesc,
                                  std::void_t<typename TShaderDesc::SettingsType>>
        : ShaderDescriptorTraitDefaults<typename TShaderDesc::SettingsType>
    {
        using ShaderType = TShaderDesc;
        using SettingsType = typename ShaderType::SettingsType;
        using Base = ShaderDescriptorTraitDefaults<SettingsType>;
        using Base::defaultSettings;
        using Base::fromConfig;

        static SettingsType normalize(SettingsType settings)
        {
            return settings;
        }

        static ShaderType make(SettingsType settings)
        {
            return ShaderType{std::move(settings)};
        }
    };

} // namespace factory
} // namespace npb
