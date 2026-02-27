#pragma once

#include "colors/GammaShader.h"
#include "factory/descriptors/ShaderDescriptors.h"
#include "factory/traits/ShaderDescriptorTraits.h"

namespace npb
{
namespace factory
{

    template <typename TColor = npb::Rgb8Color>
    using GammaOptions = npb::GammaShaderSettings<TColor>;

    template <typename TColor>
    struct ShaderDescriptorTraits<descriptors::Gamma<TColor>, void>
        : ShaderDescriptorTraitDefaults<GammaOptions<TColor>>
    {
        using ShaderType = npb::GammaShader<TColor>;
        using SettingsType = typename ShaderType::SettingsType;
        using Base = ShaderDescriptorTraitDefaults<SettingsType>;
        using Base::defaultSettings;
        using Base::fromConfig;

        static SettingsType normalize(SettingsType settings)
        {
            return settings;
        }

        static SettingsType fromConfig(const GammaOptions<TColor> &config)
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
