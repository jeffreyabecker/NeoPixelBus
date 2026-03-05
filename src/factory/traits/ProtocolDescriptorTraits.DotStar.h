#pragma once

#include <type_traits>

#include "protocols/DotStarProtocol.h"
#include "factory/descriptors/ProtocolDescriptors.h"
#include "factory/traits/ProtocolDescriptorTraits.h"

namespace lw
{
namespace factory
{

    struct DotStarOptions
    {
        const char *channelOrder = nullptr;
    };

    using DotStarDescriptorDefault = descriptors::DotStar<>;

    template <typename TDefaultChannelOrder = typename DotStarDescriptorDefault::DefaultChannelOrder>
    struct DotStarOptionsT : DotStarOptions
    {
        DotStarOptionsT()
            : DotStarOptions{}
        {
            channelOrder = TDefaultChannelOrder::value;
        }
    };

    template <typename TInterfaceColor,
              typename TDefaultChannelOrder,
              typename TStripColor>
    struct ProtocolDescriptorTraits<descriptors::DotStar<TInterfaceColor, TDefaultChannelOrder, TStripColor>, void>
        : ProtocolDescriptorTraitDefaults<lw::Apa102ProtocolSettings>
    {
        using DescriptorType = descriptors::DotStar<TInterfaceColor, TDefaultChannelOrder, TStripColor>;
        using EffectiveInterfaceColor = std::conditional_t<lw::ColorAtLeastAsLarge<typename DescriptorType::InterfaceColorType,
                                                                                    typename DescriptorType::StripColorType>,
                                                           typename DescriptorType::InterfaceColorType,
                                                           typename DescriptorType::StripColorType>;
        using ProtocolType = lw::Apa102Protocol<EffectiveInterfaceColor,
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

        static SettingsType fromConfig(const DotStarOptions &config)
        {
            SettingsType settings = Base::defaultSettings();
            settings.channelOrder = config.channelOrder;
            return settings;
        }
    };

} // namespace factory
} // namespace lw
