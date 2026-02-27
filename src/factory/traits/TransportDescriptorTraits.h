#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

#include "transports/OneWireTiming.h"

namespace npb
{
namespace factory
{

    inline uint32_t oneWireEncodedDataRateHz(const OneWireTiming &timing)
    {
        const uint32_t bitRateHz = static_cast<uint32_t>(timing.bitRateHz());
        const uint32_t encodedBitsPerDataBit = static_cast<uint32_t>(timing.bitPattern());
        return bitRateHz * encodedBitsPerDataBit;
    }

    // Contract: the timing context pointer is transient call-site state.
    // Trait implementations must copy any needed timing values into SettingsType and never store the pointer.

    template <typename TSettingsType>
    struct TransportDescriptorTraitDefaults
    {
        using SettingsType = TSettingsType;

        static SettingsType defaultSettings(uint16_t)
        {
            return SettingsType{};
        }

        static SettingsType fromConfig(SettingsType settings,
                                       uint16_t)
        {
            return settings;
        }
    };

    template <typename TTransportDesc,
              typename = void>
    struct TransportDescriptorTraits;

    template <typename TTransportDesc>
    struct TransportDescriptorTraits<TTransportDesc,
                                     std::void_t<typename TTransportDesc::TransportSettingsType>>
        : TransportDescriptorTraitDefaults<typename TTransportDesc::TransportSettingsType>
    {
        using TransportType = TTransportDesc;
        using SettingsType = typename TransportType::TransportSettingsType;
        using Base = TransportDescriptorTraitDefaults<SettingsType>;
        using Base::defaultSettings;
        using Base::fromConfig;

        static SettingsType normalize(SettingsType settings,
                                      uint16_t,
                                      const OneWireTiming * = nullptr)
        {
            return settings;
        }
    };

} // namespace factory
} // namespace npb
