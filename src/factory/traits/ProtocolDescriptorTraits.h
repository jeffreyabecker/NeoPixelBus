#pragma once

#include <cstddef>
#include <string>
#include <type_traits>

#include "factory/descriptors/ProtocolDescriptors.h"

namespace lw
{
namespace factory
{

    template <typename TSettingsType>
    struct ProtocolDescriptorTraitDefaults
    {
        using SettingsType = TSettingsType;

        template <typename TTransportSettings>
        static void mutateTransportSettings(uint16_t,
                                            const SettingsType &,
                                            TTransportSettings &)
        {
        }

    private:
        enum class ChannelPrefix
        {
            RGB,
            GRB,
            BGR
        };

        template <typename TColor,
                  typename = void>
        struct ColorChannelCount
        {
            static constexpr size_t value = 3;
        };

        template <typename TColor>
        struct ColorChannelCount<TColor,
                                 std::void_t<decltype(TColor::ChannelCount)>>
        {
            static constexpr size_t value = static_cast<size_t>(TColor::ChannelCount);
        };

        static ChannelPrefix detectPrefix(const char *channelOrder,
                                          const char *fallback)
        {
            const char *value = (channelOrder != nullptr) ? channelOrder : fallback;
            if (value == nullptr)
            {
                return ChannelPrefix::RGB;
            }

            if (std::char_traits<char>::length(value) >= 3)
            {
                if (value[0] == 'G' && value[1] == 'R' && value[2] == 'B')
                {
                    return ChannelPrefix::GRB;
                }

                if (value[0] == 'B' && value[1] == 'G' && value[2] == 'R')
                {
                    return ChannelPrefix::BGR;
                }
            }

            return ChannelPrefix::RGB;
        }

        static const char *rgbForPrefix(ChannelPrefix prefix)
        {
            switch (prefix)
            {
            case ChannelPrefix::GRB:
                return lw::ChannelOrder::GRB::value;
            case ChannelPrefix::BGR:
                return lw::ChannelOrder::BGR::value;
            default:
                return lw::ChannelOrder::RGB::value;
            }
        }

        static const char *rgbwForPrefix(ChannelPrefix prefix)
        {
            switch (prefix)
            {
            case ChannelPrefix::GRB:
                return lw::ChannelOrder::GRBW::value;
            case ChannelPrefix::BGR:
                return lw::ChannelOrder::BGRW::value;
            default:
                return lw::ChannelOrder::RGBW::value;
            }
        }

        static const char *rgbcwForPrefix(ChannelPrefix prefix)
        {
            switch (prefix)
            {
            case ChannelPrefix::GRB:
                return lw::ChannelOrder::GRBCW::value;
            case ChannelPrefix::BGR:
                return lw::ChannelOrder::BGRCW::value;
            default:
                return lw::ChannelOrder::RGBCW::value;
            }
        }

    public:

        static SettingsType defaultSettings()
        {
            return SettingsType{};
        }

        static SettingsType fromConfig(SettingsType settings)
        {
            return settings;
        }

        template <typename TColor>
        static const char *normalizeChannelOrder(const char *providedChannelOrder,
                                                 const char *defaultChannelOrder)
        {
            const char *channelOrder = (providedChannelOrder != nullptr) ? providedChannelOrder : defaultChannelOrder;
            if (channelOrder == nullptr)
            {
                return defaultChannelOrder;
            }

            constexpr size_t channelCount = ColorChannelCount<TColor>::value;
            const size_t suppliedLength = std::char_traits<char>::length(channelOrder);

            if (suppliedLength == channelCount)
            {
                return channelOrder;
            }

            const ChannelPrefix prefix = detectPrefix(channelOrder, defaultChannelOrder);

            if (channelCount <= 3)
            {
                return rgbForPrefix(prefix);
            }

            if (channelCount == 4)
            {
                return rgbwForPrefix(prefix);
            }

            return rgbcwForPrefix(prefix);
        }
    };

    template <typename TProtocolDesc,
              typename = void>
    struct ProtocolDescriptorTraits;

    template <typename TProtocolDesc>
    struct ProtocolDescriptorTraits<TProtocolDesc,
                                    std::void_t<typename TProtocolDesc::SettingsType,
                                                typename TProtocolDesc::ColorType>>
        : ProtocolDescriptorTraitDefaults<typename TProtocolDesc::SettingsType>
    {
        using ProtocolType = TProtocolDesc;
        using SettingsType = typename ProtocolType::SettingsType;
        using ColorType = typename ProtocolType::ColorType;
        using Base = ProtocolDescriptorTraitDefaults<SettingsType>;
        using Base::defaultSettings;
        using Base::fromConfig;
        using Base::mutateTransportSettings;

        static SettingsType normalize(SettingsType settings)
        {
            return settings;
        }
    };

} // namespace factory
} // namespace lw
