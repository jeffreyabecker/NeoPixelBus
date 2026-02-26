#pragma once

#include <type_traits>
#include <utility>

#include "core/Compat.h"

#include "colors/IShader.h"

#include "ProtocolConfigs.h"
#include "TransportConfigs.h"

namespace npb::factory
{

    template <typename TConfig>
    struct ProtocolConfigTraits;

    template <typename TProtocol>
    struct ProtocolConfigTraits<ProtocolConfig<TProtocol>>
    {
        using ProtocolType = TProtocol;

        static typename TProtocol::SettingsType toSettings(ProtocolConfig<TProtocol> config)
        {
            return std::move(config.settings);
        }
    };

    template <typename TColor>
    struct ProtocolConfigTraits<Ws2812x<TColor>>
    {
        using ProtocolType = Ws2812xProtocol<TColor>;

        static typename ProtocolType::SettingsType toSettings(const Ws2812x<TColor> &config)
        {
            typename ProtocolType::SettingsType settings{};
            settings.channelOrder = config.colorOrder;
            return settings;
        }
    };

    template <>
    struct ProtocolConfigTraits<Sk6812>
    {
        using ProtocolType = Ws2812xProtocol<Rgbw8Color>;

        static typename ProtocolType::SettingsType toSettings(const Sk6812 &config)
        {
            typename ProtocolType::SettingsType settings{};
            settings.channelOrder = config.colorOrder;
            return settings;
        }
    };

    template <>
    struct ProtocolConfigTraits<Ucs8904>
    {
        using ProtocolType = Ws2812xProtocol<Rgbw16Color>;

        static typename ProtocolType::SettingsType toSettings(const Ucs8904 &config)
        {
            typename ProtocolType::SettingsType settings{};
            settings.channelOrder = config.colorOrder;
            return settings;
        }
    };

    template <>
    struct ProtocolConfigTraits<DotStar>
    {
        using ProtocolType = DotStarProtocol;

        static typename ProtocolType::SettingsType toSettings(const DotStar &config)
        {
            typename ProtocolType::SettingsType settings{};
            settings.channelOrder = config.colorOrder;
            settings.mode = config.mode;
            return settings;
        }
    };

    template <typename TColor>
    struct ProtocolConfigTraits<Hd108<TColor>>
    {
        using ProtocolType = Hd108Protocol<TColor>;

        static typename ProtocolType::SettingsType toSettings(const Hd108<TColor> &config)
        {
            typename ProtocolType::SettingsType settings{};
            settings.channelOrder = config.colorOrder;
            return settings;
        }
    };

    template <>
    struct ProtocolConfigTraits<Lpd6803>
    {
        using ProtocolType = Lpd6803Protocol;

        static typename ProtocolType::SettingsType toSettings(const Lpd6803 &config)
        {
            typename ProtocolType::SettingsType settings{};
            settings.channelOrder = config.colorOrder;
            return settings;
        }
    };

    template <>
    struct ProtocolConfigTraits<Lpd8806>
    {
        using ProtocolType = Lpd8806Protocol;

        static typename ProtocolType::SettingsType toSettings(const Lpd8806 &config)
        {
            typename ProtocolType::SettingsType settings{};
            settings.channelOrder = config.colorOrder;
            return settings;
        }
    };

    template <>
    struct ProtocolConfigTraits<Pixie>
    {
        using ProtocolType = PixieProtocol;

        static typename ProtocolType::SettingsType toSettings(const Pixie &config)
        {
            typename ProtocolType::SettingsType settings{};
            settings.channelOrder = config.colorOrder;
            return settings;
        }
    };

    template <>
    struct ProtocolConfigTraits<Sm16716>
    {
        using ProtocolType = Sm16716Protocol;

        static typename ProtocolType::SettingsType toSettings(const Sm16716 &config)
        {
            typename ProtocolType::SettingsType settings{};
            settings.channelOrder = config.colorOrder;
            return settings;
        }
    };

    template <typename TColor>
    struct ProtocolConfigTraits<Sm168x<TColor>>
    {
        using ProtocolType = Sm168xProtocol<TColor>;

        static typename ProtocolType::SettingsType toSettings(const Sm168x<TColor> &config)
        {
            typename ProtocolType::SettingsType settings{};
            settings.channelOrder = config.colorOrder;
            settings.variant = config.variant;
            settings.gains = config.gains;
            return settings;
        }
    };

    template <typename TColor>
    struct ProtocolConfigTraits<Tlc5947<TColor>>
    {
        using ProtocolType = Tlc5947Protocol<TColor>;

        static typename ProtocolType::SettingsType toSettings(const Tlc5947<TColor> &config)
        {
            typename ProtocolType::SettingsType settings{};
            settings.latchPin = config.latchPin;
            settings.oePin = config.oePin;
            settings.channelOrder = config.colorOrder;
            settings.pixelStrategy = config.pixelStrategy;
            settings.tailFillStrategy = config.tailFillStrategy;
            return settings;
        }
    };

    template <>
    struct ProtocolConfigTraits<Tm1814>
    {
        using ProtocolType = Tm1814Protocol;

        static typename ProtocolType::SettingsType toSettings(const Tm1814 &config)
        {
            typename ProtocolType::SettingsType settings{};
            settings.channelOrder = config.colorOrder;
            settings.current = config.current;
            return settings;
        }
    };

    template <>
    struct ProtocolConfigTraits<Tm1914>
    {
        using ProtocolType = Tm1914Protocol;

        static typename ProtocolType::SettingsType toSettings(const Tm1914 &config)
        {
            typename ProtocolType::SettingsType settings{};
            settings.channelOrder = config.colorOrder;
            settings.mode = config.mode;
            return settings;
        }
    };

    template <>
    struct ProtocolConfigTraits<Ws2801>
    {
        using ProtocolType = Ws2801Protocol;

        static typename ProtocolType::SettingsType toSettings(const Ws2801 &config)
        {
            typename ProtocolType::SettingsType settings{};
            settings.channelOrder = config.colorOrder;
            return settings;
        }
    };

    template <typename TConfig>
    struct TransportConfigTraits;

    template <typename TTransport>
    struct TransportConfigTraits<TransportConfig<TTransport>>
    {
        using TransportType = TTransport;

        static typename TTransport::TransportSettingsType toSettings(TransportConfig<TTransport> config)
        {
            return std::move(config.settings);
        }
    };

    template <>
    struct TransportConfigTraits<Debug>
    {
        using TransportType = DebugOneWireTransport;

        static typename TransportType::TransportSettingsType toSettings(Debug &&config)
        {
            typename TransportType::TransportSettingsType settings{};
            settings.output = std::move(config.output);
            settings.invert = config.invert;
            return settings;
        }
    };

    template <typename TProtocolConfig, typename = void>
    struct FactoryProtocolConfigImpl : std::false_type
    {
    };

    template <typename TProtocolConfig>
    struct FactoryProtocolConfigImpl<TProtocolConfig,
                                     std::void_t<typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType,
                                                 typename remove_cvref_t<TProtocolConfig>::ColorType,
                                                 decltype(ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::toSettings(
                                                     std::declval<remove_cvref_t<TProtocolConfig> &&>()))>>
        : std::bool_constant<std::is_same<typename remove_cvref_t<TProtocolConfig>::ColorType,
                                          typename ProtocolConfigTraits<remove_cvref_t<TProtocolConfig>>::ProtocolType::ColorType>::value>
    {
    };

    template <typename TProtocolConfig>
    static constexpr bool FactoryProtocolConfig = FactoryProtocolConfigImpl<TProtocolConfig>::value;

    template <typename TTransportConfig, typename = void>
    struct FactoryTransportConfigImpl : std::false_type
    {
    };

    template <typename TTransportConfig>
    struct FactoryTransportConfigImpl<TTransportConfig,
                                      std::void_t<typename TransportConfigTraits<remove_cvref_t<TTransportConfig>>::TransportType,
                                                  decltype(TransportConfigTraits<remove_cvref_t<TTransportConfig>>::toSettings(
                                                      std::declval<remove_cvref_t<TTransportConfig> &&>()))>> : std::true_type
    {
    };

    template <typename TTransportConfig>
    static constexpr bool FactoryTransportConfig = FactoryTransportConfigImpl<TTransportConfig>::value;

    template <typename TShaderFactory, typename TColor, typename = void>
    struct FactoryShaderForColorImpl : std::false_type
    {
    };

    template <typename TShaderFactory, typename TColor>
    struct FactoryShaderForColorImpl<TShaderFactory,
                                     TColor,
                                     std::void_t<decltype(std::declval<const remove_cvref_t<TShaderFactory> &>().template make<TColor>())>> : std::true_type
    {
    };

    template <typename TShaderFactory, typename TColor>
    static constexpr bool FactoryShaderForColor = FactoryShaderForColorImpl<TShaderFactory, TColor>::value;

    template <typename TShader, typename TColor, typename = void>
    struct ShaderInstanceForColorImpl : std::false_type
    {
    };

    template <typename TShader, typename TColor>
    struct ShaderInstanceForColorImpl<TShader,
                                      TColor,
                                      std::enable_if_t<std::is_base_of<IShader<TColor>, remove_cvref_t<TShader>>::value &&
                                                       std::is_copy_constructible<remove_cvref_t<TShader>>::value &&
                                                       std::is_copy_assignable<remove_cvref_t<TShader>>::value>> : std::true_type
    {
    };

    template <typename TShader, typename TColor>
    static constexpr bool ShaderInstanceForColor = ShaderInstanceForColorImpl<TShader, TColor>::value;

    template <typename TShaderOrFactory, typename TColor, bool IsFactory = FactoryShaderForColor<TShaderOrFactory, TColor>>
    struct ShaderTypeForColorImpl;

    template <typename TShaderOrFactory, typename TColor>
    struct ShaderTypeForColorImpl<TShaderOrFactory, TColor, true>
    {
        using Type = decltype(std::declval<const remove_cvref_t<TShaderOrFactory> &>().template make<TColor>());
    };

    template <typename TShaderOrFactory, typename TColor>
    struct ShaderTypeForColorImpl<TShaderOrFactory, TColor, false>
    {
        using Type = remove_cvref_t<TShaderOrFactory>;
    };

    template <typename TShaderOrFactory, typename TColor>
    using ShaderTypeForColor = typename ShaderTypeForColorImpl<TShaderOrFactory, TColor>::Type;

} // namespace npb::factory
