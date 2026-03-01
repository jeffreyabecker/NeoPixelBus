#pragma once

#include "colors/WhiteBalanceShader.h"
#include "factory/descriptors/ShaderDescriptors.h"
#include "factory/traits/ShaderDescriptorTraits.h"

namespace lw
{
namespace factory
{

    template <typename TColor = lw::Rgbw8Color>
    using WhiteBalanceOptions = lw::WhiteBalanceShaderSettings<TColor>;

    template <typename TColor>
    struct ShaderDescriptorTraits<descriptors::WhiteBalance<TColor>, void>
        : ShaderDescriptorTraitDefaults<WhiteBalanceOptions<TColor>>
    {
        using ShaderType = lw::WhiteBalanceShader<TColor>;
        using SettingsType = typename ShaderType::SettingsType;
        using Base = ShaderDescriptorTraitDefaults<SettingsType>;
        using Base::defaultSettings;
        using Base::fromConfig;

        static SettingsType normalize(SettingsType settings)
        {
            return settings;
        }

        static SettingsType fromConfig(const WhiteBalanceOptions<TColor> &config)
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
