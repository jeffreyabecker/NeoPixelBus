#pragma once

#include <type_traits>
#include <utility>

#include "colors/AggregateShader.h"
#include "colors/IShader.h"
#include "core/Compat.h"
#include "factory/descriptors/ShaderDescriptors.h"
#include "factory/Traits.h"

namespace npb
{
namespace factory
{

    template <typename TColor = npb::Rgb8Color>
    using Gamma = descriptors::Gamma<TColor>;

    template <typename TColor = npb::Rgb8Color>
    using CurrentLimiter = descriptors::CurrentLimiter<TColor>;

    template <typename TColor = npb::Rgbw8Color>
    using WhiteBalance = descriptors::WhiteBalance<TColor>;

    template <typename TColor = npb::Rgb8Color>
    using NoShader = descriptors::NilShader<TColor>;

    template <typename TColor,
              template <typename>
              class TShaderDesc,
              template <typename>
              class... TOtherShaderDesc>
    struct ShaderTypeResolver;

    template <typename TColor,
              template <typename>
              class TShaderDesc>
    struct ShaderTypeResolver<TColor, TShaderDesc>
    {
        using Type = typename ShaderDescriptorTraits<TShaderDesc<TColor>>::ShaderType;
    };

    template <typename TColor,
              template <typename>
              class TFirstShaderDesc,
              template <typename>
              class TSecondShaderDesc,
              template <typename>
              class... TOtherShaderDesc>
    struct ShaderTypeResolver<TColor,
                              TFirstShaderDesc,
                              TSecondShaderDesc,
                              TOtherShaderDesc...>
    {
        using Type = OwningAggregateShaderT<TColor,
                                            typename ShaderDescriptorTraits<TFirstShaderDesc<TColor>>::ShaderType,
                                            typename ShaderDescriptorTraits<TSecondShaderDesc<TColor>>::ShaderType,
                                            typename ShaderDescriptorTraits<TOtherShaderDesc<TColor>>::ShaderType...>;
    };

    template <typename TColor,
              template <typename>
              class TShaderDesc,
              template <typename>
              class... TOtherShaderDesc>
    using Shader = typename ShaderTypeResolver<TColor, TShaderDesc, TOtherShaderDesc...>::Type;

    template <typename TShaderDesc,
              typename TShaderTraits = ShaderDescriptorTraits<TShaderDesc>,
              typename TShader = typename TShaderTraits::ShaderType,
              typename TSettings = typename TShaderTraits::SettingsType,
              typename TShaderConfig,
              typename = std::enable_if_t<std::is_convertible<npb::remove_cvref_t<decltype(resolveShaderSettings<TShaderDesc>(std::declval<TShaderConfig>()))>,
                                                              TSettings>::value>>
    TShader makeShader(TShaderConfig &&shaderConfig)
    {
        auto shaderSettings = resolveShaderSettings<TShaderDesc>(std::forward<TShaderConfig>(shaderConfig));
        return TShaderTraits::make(std::move(shaderSettings));
    }

    template <typename TShaderDesc,
              typename TShaderTraits = ShaderDescriptorTraits<TShaderDesc>,
              typename TShader = typename TShaderTraits::ShaderType,
              typename TSettings = typename TShaderTraits::SettingsType,
              typename = std::enable_if_t<std::is_default_constructible<TSettings>::value>>
    TShader makeShader()
    {
        auto shaderSettings = resolveShaderSettings<TShaderDesc>(TSettings{});
        return TShaderTraits::make(std::move(shaderSettings));
    }

    template <typename TFirstShader,
              typename TSecondShader,
              typename... TOtherShaders,
              typename TFirstShaderType = npb::remove_cvref_t<TFirstShader>,
              typename TSecondShaderType = npb::remove_cvref_t<TSecondShader>,
              typename TColor = typename TFirstShaderType::ColorType,
              typename = std::enable_if_t<std::is_base_of<IShader<TColor>, TFirstShaderType>::value &&
                                          std::is_base_of<IShader<TColor>, TSecondShaderType>::value &&
                                          std::conjunction<std::is_base_of<IShader<TColor>, npb::remove_cvref_t<TOtherShaders>>...>::value>>
    OwningAggregateShaderT<TColor,
                           TFirstShaderType,
                           TSecondShaderType,
                           npb::remove_cvref_t<TOtherShaders>...>
    makeShader(TFirstShader &&firstShader,
               TSecondShader &&secondShader,
               TOtherShaders &&...otherShaders)
    {
        using AggregateType = OwningAggregateShaderT<TColor,
                                                     TFirstShaderType,
                                                     TSecondShaderType,
                                                     npb::remove_cvref_t<TOtherShaders>...>;
        return AggregateType{std::forward<TFirstShader>(firstShader),
                             std::forward<TSecondShader>(secondShader),
                             std::forward<TOtherShaders>(otherShaders)...};
    }

} // namespace factory
} // namespace npb
