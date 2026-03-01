#pragma once

#include "colors/CurrentLimiterShader.h"
#include "factory/descriptors/ShaderDescriptors.h"
#include "factory/traits/ShaderDescriptorTraits.h"

namespace lw
{
namespace factory
{

    template <typename TColor = lw::Rgb8Color>
    using CurrentLimiterOptions = lw::CurrentLimiterShaderSettings<TColor>;

    template <typename TColor>
    struct ShaderDescriptorTraits<descriptors::CurrentLimiter<TColor>, void>
        : ShaderDescriptorTraitDefaults<CurrentLimiterOptions<TColor>>
    {
        using ShaderType = lw::CurrentLimiterShader<TColor>;
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
} // namespace lw
