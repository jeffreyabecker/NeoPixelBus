#pragma once

#include "colors/GammaShader.h"
#include "factory/descriptors/ShaderDescriptors.h"
#include "factory/traits/ShaderDescriptorTraits.h"

namespace lw
{
namespace factory
{

    template <typename TColor = lw::Rgb8Color>
    using GammaOptions = lw::GammaShaderSettings<TColor>;

    template <typename TColor>
    struct ShaderDescriptorTraits<descriptors::Gamma<TColor>, void>
        : ShaderDescriptorTraitDefaults<GammaOptions<TColor>>
    {
        using ShaderType = lw::GammaShader<TColor>;
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
} // namespace lw
