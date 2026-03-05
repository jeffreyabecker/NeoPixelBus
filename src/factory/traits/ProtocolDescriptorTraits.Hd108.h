#pragma once

#include <type_traits>

#include "protocols/DotStarProtocol.h"
#include "factory/descriptors/ProtocolDescriptors.h"
#include "factory/traits/ProtocolDescriptorTraits.h"
#include "factory/traits/ProtocolDescriptorTraits.DotStar.h"

namespace lw
{
namespace factory
{

    struct Hd108Options
    {
        const char *channelOrder = nullptr;
    };

    template <typename TDefaultChannelOrder = typename descriptors::HD108::DefaultChannelOrder>
    struct Hd108OptionsT : Hd108Options
    {
        Hd108OptionsT()
            : Hd108Options{}
        {
            channelOrder = TDefaultChannelOrder::value;
        }
    };

    template <typename TInterfaceColor,
              typename TDefaultChannelOrder,
              typename TStripColor>
    struct ProtocolDescriptorTraits<descriptors::Hd108<TInterfaceColor, TDefaultChannelOrder, TStripColor>, void>
        : ProtocolDescriptorTraitDefaults<lw::Hd108ProtocolSettings>
    {
        using DescriptorType = descriptors::Hd108<TInterfaceColor, TDefaultChannelOrder, TStripColor>;
        using EffectiveInterfaceColor = std::conditional_t<lw::ColorAtLeastAsLarge<typename DescriptorType::InterfaceColorType,
                                                                                    typename DescriptorType::StripColorType>,
                                                           typename DescriptorType::InterfaceColorType,
                                                           typename DescriptorType::StripColorType>;
        using ProtocolType = lw::Hd108Protocol<EffectiveInterfaceColor,
                                               typename DescriptorType::StripColorType>;
        using SettingsType = typename ProtocolType::SettingsType;
        using ColorType = typename ProtocolType::ColorType;
        using Base = ProtocolDescriptorTraitDefaults<SettingsType>;
        using Base::defaultSettings;
        using Base::fromConfig;

        static SettingsType normalize(SettingsType settings)
        {
            return SettingsType::template normalizeForColor<ColorType>(settings,
                                                                       DescriptorType::DefaultChannelOrder::value);
        }

        static SettingsType fromConfig(const Hd108Options &config)
        {
            SettingsType settings = Base::defaultSettings();
            settings.channelOrder = config.channelOrder;
            return settings;
        }

        static SettingsType fromConfig(const DotStarOptions &config)
        {
            SettingsType settings = Base::defaultSettings();
            settings.channelOrder = config.channelOrder;
            return settings;
        }
    };

} // namespace factory
} // namespace lw
