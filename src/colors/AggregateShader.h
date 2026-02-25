#pragma once

#include <concepts>
#include <cstddef>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "IShader.h"
#include "core/ResourceHandle.h"

namespace npb
{

    template <typename TColor>
    struct AggregateShaderSettings
    {
        std::vector<ResourceHandle<IShader<TColor>>> shaders;
    };

    template <typename TColor>
    class AggregateShader : public IShader<TColor>
    {
    public:
        using SettingsType = AggregateShaderSettings<TColor>;

        explicit AggregateShader(SettingsType settings)
            : _shaders(std::move(settings.shaders))
        {
        }

        void apply(std::span<TColor> colors) override
        {
            for (auto &shader : _shaders)
            {
                if (shader != nullptr)
                {
                    shader->apply(colors);
                }
            }
        }

    private:
        std::vector<ResourceHandle<IShader<TColor>>> _shaders;
    };

    template <typename TColor, typename... TShaders>
        requires(sizeof...(TShaders) > 0 && (std::derived_from<TShaders, IShader<TColor>> && ...))
    class OwningAggregateShaderT : public IShader<TColor>
    {
    public:
        explicit OwningAggregateShaderT(TShaders... shaders)
            : _shaders(std::move(shaders)...)
            , _aggregate(makeAggregateSettings(_shaders))
        {
        }

        void apply(std::span<TColor> colors) override
        {
            _aggregate.apply(colors);
        }

    private:
        static typename AggregateShader<TColor>::SettingsType makeAggregateSettings(std::tuple<TShaders...> &shaders)
        {
            typename AggregateShader<TColor>::SettingsType settings{};
            settings.shaders.reserve(sizeof...(TShaders));

            std::apply([&](auto &...shader) {
                (settings.shaders.emplace_back(shader), ...);
            }, shaders);

            return settings;
        }

        std::tuple<TShaders...> _shaders;
        AggregateShader<TColor> _aggregate;
    };

} // namespace npb


