#pragma once

#include "colors/NilShader.h"
#include "factory/descriptors/ShaderDescriptors.h"
#include "factory/traits/ShaderDescriptorTraits.h"

namespace npb
{
namespace factory
{

    template <typename TColor = npb::Rgb8Color>
    struct NilShaderOptions
    {
    };

    template <typename TColor>
    struct ShaderDescriptorTraits<descriptors::NilShader<TColor>, void>
        : ShaderDescriptorTraitDefaults<NilShaderOptions<TColor>>
    {
        using ShaderType = npb::NilShader<TColor>;
        using SettingsType = NilShaderOptions<TColor>;
        using Base = ShaderDescriptorTraitDefaults<SettingsType>;
        using Base::defaultSettings;
        using Base::fromConfig;

        static SettingsType normalize(SettingsType settings)
        {
            return settings;
        }

        static ShaderType make(SettingsType)
        {
            return ShaderType{};
        }
    };

} // namespace factory
} // namespace npb
