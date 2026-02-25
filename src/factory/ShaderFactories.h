#pragma once

#include <array>
#include <cstdint>
#include <tuple>
#include <utility>

#include "colors/AggregateShader.h"
#include "colors/Color.h"
#include "colors/CurrentLimiterShader.h"
#include "colors/GammaShader.h"

namespace npb::factory
{

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

    class GammaShaderFactory
    {
    public:
        explicit GammaShaderFactory(Gamma settings)
            : _settings{settings}
        {
        }

        template <typename TColor>
            requires ColorComponentTypeIs<TColor, uint8_t>
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

        template <typename TColor>
            requires(TColor::ChannelCount == NChannels)
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
        requires(sizeof...(TShaderFactories) > 0)
    class AggregateShaderFactory
    {
    public:
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

    inline GammaShaderFactory makeGammaShader(Gamma settings = {})
    {
        return GammaShaderFactory{settings};
    }

    inline CurrentLimiterShaderFactory<Rgb8Color::ChannelCount> makeCurrentLimiterShader(CurrentLimiterRgb settings)
    {
        return CurrentLimiterShaderFactory<Rgb8Color::ChannelCount>{
            settings.maxMilliamps,
            settings.milliampsPerChannel,
            settings.controllerMilliamps,
            settings.standbyMilliampsPerPixel,
            settings.rgbwDerating};
    }

    inline CurrentLimiterShaderFactory<Rgbw8Color::ChannelCount> makeCurrentLimiterShader(CurrentLimiterRgbw settings)
    {
        return CurrentLimiterShaderFactory<Rgbw8Color::ChannelCount>{
            settings.maxMilliamps,
            settings.milliampsPerChannel,
            settings.controllerMilliamps,
            settings.standbyMilliampsPerPixel,
            settings.rgbwDerating};
    }

    inline CurrentLimiterShaderFactory<Rgbcw8Color::ChannelCount> makeCurrentLimiterShader(CurrentLimiterRgbcw settings)
    {
        return CurrentLimiterShaderFactory<Rgbcw8Color::ChannelCount>{
            settings.maxMilliamps,
            settings.milliampsPerChannel,
            settings.controllerMilliamps,
            settings.standbyMilliampsPerPixel,
            settings.rgbwDerating};
    }

    template <typename... TShaderFactories>
        requires(sizeof...(TShaderFactories) > 0)
    auto makeAggregateShader(TShaderFactories... shaderFactories)
    {
        return AggregateShaderFactory<std::remove_cvref_t<TShaderFactories>...>(
            std::move(shaderFactories)...);
    }

} // namespace npb::factory


