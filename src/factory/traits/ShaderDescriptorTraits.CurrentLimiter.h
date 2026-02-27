#pragma once

#include "colors/CurrentLimiterShader.h"
#include "factory/descriptors/ShaderDescriptors.h"
#include "factory/traits/ShaderDescriptorTraits.h"

namespace npb
{
namespace factory
{

    template <typename TColor = npb::Rgb8Color>
    using CurrentLimiterOptions = npb::CurrentLimiterShaderSettings<TColor>;

    template <typename TColor>
    struct ShaderDescriptorTraits<descriptors::CurrentLimiter<TColor>, void>
        : ShaderDescriptorTraitDefaults<CurrentLimiterOptions<TColor>>
    {
        using ShaderType = npb::CurrentLimiterShader<TColor>;
        using SettingsType = typename ShaderType::SettingsType;
        using Base = ShaderDescriptorTraitDefaults<SettingsType>;
        using Base::defaultSettings;
        using Base::fromConfig;

        static SettingsType normalize(SettingsType settings)
        {
            return settings;
        }

        static SettingsType fromConfig(const CurrentLimiterOptions<TColor> &config)
        {
            return config;
        }

        static ShaderType make(SettingsType settings)
        {
            return ShaderType{std::move(settings)};
        }
    };

} // namespace factory
} // namespace npb
