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
              typename TCapabilityRequirement,
              typename TDefaultChannelOrder,
              typename TStripColor>
    struct ProtocolDescriptorTraits<descriptors::DotStar<TInterfaceColor, TCapabilityRequirement, TDefaultChannelOrder, TStripColor>, void>
        : ProtocolDescriptorTraitDefaults<lw::Apa102ProtocolSettings>
    {
        using DescriptorType = descriptors::DotStar<TInterfaceColor, TCapabilityRequirement, TDefaultChannelOrder, TStripColor>;
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


        static_assert(std::is_same<typename DescriptorType::CapabilityRequirement, typename ProtocolType::TransportCategory>::value,
                      "DotStar descriptor capability requirement must match Apa102Protocol transport category.");

        static SettingsType normalize(SettingsType settings)
        {
            settings.channelOrder = Base::template normalizeChannelOrder<ColorType>(
                settings.channelOrder,
                DescriptorType::DefaultChannelOrder::value);

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
