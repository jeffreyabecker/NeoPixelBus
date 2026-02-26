#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include "core/Compat.h"
#include "colors/AggregateShader.h"
#include "colors/Color.h"
#include "colors/CurrentLimiterShader.h"
#include "colors/GammaShader.h"

namespace npb::factory
{

    template <typename TColor>
    struct CurrentLimiterTypeForColor;

    using ChannelMilliamps = CurrentLimiterChannelMilliamps;

    struct Gamma
    {
        float gamma = 2.6f;
        bool enableColorGamma = true;
        bool enableBrightnessGamma = false;
    };

    struct CurrentLimiterRgb
    {
        uint32_t maxMilliamps = 0;
        ChannelMilliamps milliampsPerChannel{};
        uint16_t controllerMilliamps = CurrentLimiterShader<Rgb8Color>::DefaultControllerMilliamps;
        uint16_t standbyMilliampsPerPixel = CurrentLimiterShader<Rgb8Color>::DefaultStandbyMilliampsPerPixel;
        bool rgbwDerating = true;
    };

    struct CurrentLimiterRgbw
    {
        uint32_t maxMilliamps = 0;
        ChannelMilliamps milliampsPerChannel{};
        uint16_t controllerMilliamps = CurrentLimiterShader<Rgbw8Color>::DefaultControllerMilliamps;
        uint16_t standbyMilliampsPerPixel = CurrentLimiterShader<Rgbw8Color>::DefaultStandbyMilliampsPerPixel;
        bool rgbwDerating = true;
    };

    struct CurrentLimiterRgbcw
    {
        uint32_t maxMilliamps = 0;
        ChannelMilliamps milliampsPerChannel{};
        uint16_t controllerMilliamps = CurrentLimiterShader<Rgbcw8Color>::DefaultControllerMilliamps;
        uint16_t standbyMilliampsPerPixel = CurrentLimiterShader<Rgbcw8Color>::DefaultStandbyMilliampsPerPixel;
        bool rgbwDerating = true;
    };

    template <>
    struct CurrentLimiterTypeForColor<Rgb8Color>
    {
        using Type = CurrentLimiterRgb;
    };

    template <>
    struct CurrentLimiterTypeForColor<Rgbw8Color>
    {
        using Type = CurrentLimiterRgbw;
    };

    template <>
    struct CurrentLimiterTypeForColor<Rgbcw8Color>
    {
        using Type = CurrentLimiterRgbcw;
    };

    template <typename TColor>
    using CurrentLimiter = typename CurrentLimiterTypeForColor<TColor>::Type;

    class GammaShaderFactory
    {
    public:
        explicit GammaShaderFactory(Gamma settings)
            : _settings{settings}
        {
        }

        template <typename TColor,
                  typename = std::enable_if_t<ColorComponentTypeIs<TColor, uint8_t>>>
        auto make() const
        {
            return GammaShader<TColor>(typename GammaShader<TColor>::SettingsType{
                .gamma = _settings.gamma,
                .enableColorGamma = _settings.enableColorGamma,
                .enableBrightnessGamma = _settings.enableBrightnessGamma,
            });
        }

    private:
        Gamma _settings;
    };

    template <size_t NChannels>
    class CurrentLimiterShaderFactory
    {
    public:
        CurrentLimiterShaderFactory(uint32_t maxMilliamps,
                                    ChannelMilliamps milliampsPerChannel,
                                    uint16_t controllerMilliamps,
                                    uint16_t standbyMilliampsPerPixel,
                                    bool rgbwDerating)
            : _maxMilliamps{maxMilliamps}
            , _milliampsPerChannel{std::move(milliampsPerChannel)}
            , _controllerMilliamps{controllerMilliamps}
            , _standbyMilliampsPerPixel{standbyMilliampsPerPixel}
            , _rgbwDerating{rgbwDerating}
        {
        }

        template <typename TColor,
                  typename = std::enable_if_t<(TColor::ChannelCount == NChannels)>>
        auto make() const
        {
            typename CurrentLimiterShader<TColor>::SettingsType settings{};
            settings.maxMilliamps = _maxMilliamps;
            settings.milliampsPerChannel = _milliampsPerChannel;
            settings.controllerMilliamps = _controllerMilliamps;
            settings.standbyMilliampsPerPixel = _standbyMilliampsPerPixel;
            settings.rgbwDerating = _rgbwDerating;
            return CurrentLimiterShader<TColor>(settings);
        }

    private:
        uint32_t _maxMilliamps;
        ChannelMilliamps _milliampsPerChannel;
        uint16_t _controllerMilliamps;
        uint16_t _standbyMilliampsPerPixel;
        bool _rgbwDerating;
    };

    template <typename... TShaderFactories>
    class AggregateShaderFactory
    {
    public:
        static_assert(sizeof...(TShaderFactories) > 0,
                      "AggregateShaderFactory requires at least one shader factory");

        explicit AggregateShaderFactory(TShaderFactories... shaders)
            : _shaders(std::move(shaders)...)
        {
        }

        template <typename TColor>
        auto make() const
        {
            return std::apply(
                [](const auto &...shaderFactory)
                {
                    return OwningAggregateShaderT<TColor,
                                                  decltype(shaderFactory.template make<TColor>())...>(
                        shaderFactory.template make<TColor>()...);
                },
                _shaders);
        }

    private:
        std::tuple<TShaderFactories...> _shaders;
    };

    template <typename TShaderConfig, typename TColor>
    struct ShaderFactoryTypeForConfig;

    template <typename TColor>
    struct ShaderFactoryTypeForConfig<Gamma, TColor>
    {
        using Type = GammaShaderFactory;
    };

    template <typename TColor>
    struct ShaderFactoryTypeForConfig<CurrentLimiter<TColor>, TColor>
    {
        using Type = CurrentLimiterShaderFactory<TColor::ChannelCount>;
    };

    template <typename TProtocolConfig, typename... TShaderConfigs>
    using Shader = AggregateShaderFactory<
        typename ShaderFactoryTypeForConfig<remove_cvref_t<TShaderConfigs>, typename remove_cvref_t<TProtocolConfig>::ColorType>::Type...>;

    inline GammaShaderFactory makeShader(Gamma settings = {})
    {
        return GammaShaderFactory{settings};
    }

    inline CurrentLimiterShaderFactory<Rgb8Color::ChannelCount> makeShader(CurrentLimiterRgb settings)
    {
        return CurrentLimiterShaderFactory<Rgb8Color::ChannelCount>{
            settings.maxMilliamps,
            settings.milliampsPerChannel,
            settings.controllerMilliamps,
            settings.standbyMilliampsPerPixel,
            settings.rgbwDerating};
    }

    inline CurrentLimiterShaderFactory<Rgbw8Color::ChannelCount> makeShader(CurrentLimiterRgbw settings)
    {
        return CurrentLimiterShaderFactory<Rgbw8Color::ChannelCount>{
            settings.maxMilliamps,
            settings.milliampsPerChannel,
            settings.controllerMilliamps,
            settings.standbyMilliampsPerPixel,
            settings.rgbwDerating};
    }

    inline CurrentLimiterShaderFactory<Rgbcw8Color::ChannelCount> makeShader(CurrentLimiterRgbcw settings)
    {
        return CurrentLimiterShaderFactory<Rgbcw8Color::ChannelCount>{
            settings.maxMilliamps,
            settings.milliampsPerChannel,
            settings.controllerMilliamps,
            settings.standbyMilliampsPerPixel,
            settings.rgbwDerating};
    }

    namespace detail
    {

        template <typename TShaderFactory,
                  typename TColor>
        using ShaderTypeFromFactory = decltype(std::declval<const remove_cvref_t<TShaderFactory> &>().template make<TColor>());

        inline GammaShaderFactory toShaderFactory(Gamma settings)
        {
            return makeShader(settings);
        }

        inline CurrentLimiterShaderFactory<Rgb8Color::ChannelCount> toShaderFactory(CurrentLimiterRgb settings)
        {
            return makeShader(settings);
        }

        inline CurrentLimiterShaderFactory<Rgbw8Color::ChannelCount> toShaderFactory(CurrentLimiterRgbw settings)
        {
            return makeShader(settings);
        }

        inline CurrentLimiterShaderFactory<Rgbcw8Color::ChannelCount> toShaderFactory(CurrentLimiterRgbcw settings)
        {
            return makeShader(settings);
        }

        template <typename TShaderFactory>
        remove_cvref_t<TShaderFactory> toShaderFactory(TShaderFactory &&shaderFactory)
        {
            return std::forward<TShaderFactory>(shaderFactory);
        }

    } // namespace detail

    template <typename TShaderA,
              typename TShaderB,
              typename... TShaders>
    auto makeShader(TShaderA shaderA,
                    TShaderB shaderB,
                    TShaders... shaders)
    {
        return makeAggregateShader(
            detail::toShaderFactory(std::move(shaderA)),
            detail::toShaderFactory(std::move(shaderB)),
            detail::toShaderFactory(std::move(shaders))...);
    }

    inline GammaShaderFactory makeGammaShader(Gamma settings = {})
    {
        return makeShader(settings);
    }

    inline CurrentLimiterShaderFactory<Rgb8Color::ChannelCount> makeCurrentLimiterShader(CurrentLimiterRgb settings)
    {
        return makeShader(settings);
    }

    inline CurrentLimiterShaderFactory<Rgbw8Color::ChannelCount> makeCurrentLimiterShader(CurrentLimiterRgbw settings)
    {
        return makeShader(settings);
    }

    inline CurrentLimiterShaderFactory<Rgbcw8Color::ChannelCount> makeCurrentLimiterShader(CurrentLimiterRgbcw settings)
    {
        return makeShader(settings);
    }

    template <typename... TShaderFactories,
              typename = std::enable_if_t<(sizeof...(TShaderFactories) > 0)>>
    auto makeAggregateShader(TShaderFactories... shaderFactories)
    {
        return AggregateShaderFactory<remove_cvref_t<TShaderFactories>...>(
            std::move(shaderFactories)...);
    }

    template <typename TColor,
              typename... TShaders,
              typename = std::enable_if_t<(sizeof...(TShaders) > 0) &&
                                          std::conjunction<std::is_base_of<IShader<TColor>, remove_cvref_t<TShaders>>...>::value>>
    std::unique_ptr<IShader<TColor>> makeDynamicAggregateShader(TShaders... shaders)
    {
        using AggregateType = OwningAggregateShaderT<TColor, remove_cvref_t<TShaders>...>;
        return std::make_unique<AggregateType>(std::move(shaders)...);
    }

    template <typename TColor,
              typename... TShaderFactories,
              typename = std::enable_if_t<(sizeof...(TShaderFactories) > 0)>,
              typename = std::void_t<detail::ShaderTypeFromFactory<TShaderFactories, TColor>...>>
    std::unique_ptr<IShader<TColor>> makeDynamicAggregateShader(TShaderFactories... shaderFactories)
    {
        using AggregateType = OwningAggregateShaderT<TColor,
                                                     detail::ShaderTypeFromFactory<TShaderFactories, TColor>...>;

        return std::make_unique<AggregateType>(
            detail::toShaderFactory(std::move(shaderFactories)).template make<TColor>()...);
    }

} // namespace npb::factory


