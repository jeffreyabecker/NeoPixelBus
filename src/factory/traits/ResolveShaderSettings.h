#pragma once

#include <utility>

#include "factory/traits/ShaderDescriptorTraits.h"

namespace npb
{
namespace factory
{

    template <typename TShaderDesc>
    inline typename ShaderDescriptorTraits<TShaderDesc>::SettingsType resolveShaderSettings(
        typename ShaderDescriptorTraits<TShaderDesc>::SettingsType settings)
    {
        using Traits = ShaderDescriptorTraits<TShaderDesc>;
        return Traits::normalize(std::move(settings));
    }

    template <typename TShaderDesc,
              typename TShaderConfig>
    inline typename ShaderDescriptorTraits<TShaderDesc>::SettingsType resolveShaderSettings(TShaderConfig &&config)
    {
        using Traits = ShaderDescriptorTraits<TShaderDesc>;
        return Traits::normalize(Traits::fromConfig(std::forward<TShaderConfig>(config)));
    }

} // namespace factory
} // namespace npb
