#pragma once

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "IShader.h"

namespace lw::shaders
{

template <typename TColor> struct AggregateShaderSettings
{
    std::vector<IShader<TColor>*> shaders;
};

template <typename TColor> class AggregateShader : public IShader<TColor>
{
  public:
    using ColorType = TColor;
    using SettingsType = AggregateShaderSettings<TColor>;

    explicit AggregateShader(SettingsType settings) : _shaders(std::move(settings.shaders)) {}

    void apply(span<TColor> colors) override
    {
        for (auto& shader : _shaders)
        {
            if (shader != nullptr)
            {
                shader->apply(colors);
            }
        }
    }

  private:
    std::vector<IShader<TColor>*> _shaders;
};

template <typename TColor, typename... TShaders> class CompositeShader : public IShader<TColor>
{
  public:
    using ColorType = TColor;
    static_assert(sizeof...(TShaders) > 0, "CompositeShader requires at least one shader");
    static_assert(std::conjunction<std::is_base_of<IShader<TColor>, TShaders>...>::value,
                  "All TShaders must derive from IShader<TColor>");

    explicit CompositeShader(TShaders... shaders)
        : _shaders(std::move(shaders)...), _aggregate(makeAggregateSettings(_shaders))
    {
    }

    void apply(span<TColor> colors) override { _aggregate.apply(colors); }

  private:
    static typename AggregateShader<TColor>::SettingsType makeAggregateSettings(std::tuple<TShaders...>& shaders)
    {
        typename AggregateShader<TColor>::SettingsType settings{};
        settings.shaders.reserve(sizeof...(TShaders));

        std::apply([&](auto&... shader) { (settings.shaders.emplace_back(&shader), ...); }, shaders);

        return settings;
    }

    std::tuple<TShaders...> _shaders;
    AggregateShader<TColor> _aggregate;
};

} // namespace lw::shaders

namespace lw
{

template <typename TColor> using AggregateShaderSettings = shaders::AggregateShaderSettings<TColor>;

template <typename TColor> using AggregateShader = shaders::AggregateShader<TColor>;

template <typename TColor, typename... TShaders>
using OwningAggregateShaderT = shaders::CompositeShader<TColor, TShaders...>;

} // namespace lw
