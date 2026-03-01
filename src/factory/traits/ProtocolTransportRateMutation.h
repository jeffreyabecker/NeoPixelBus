#pragma once

#include <cstdint>
#include <type_traits>

namespace lw
{
namespace factory
{

    template <typename TTransportSettings, typename = void>
    struct TransportSettingsHasClockRateHz : std::false_type
    {
    };

    template <typename TTransportSettings>
    struct TransportSettingsHasClockRateHz<TTransportSettings,
                                           std::void_t<decltype(std::declval<TTransportSettings &>().clockRateHz)>>
        : std::true_type
    {
    };

    template <typename TTransportSettings, typename = void>
    struct TransportSettingsHasBaudRate : std::false_type
    {
    };

    template <typename TTransportSettings>
    struct TransportSettingsHasBaudRate<TTransportSettings,
                                        std::void_t<decltype(std::declval<TTransportSettings &>().baudRate)>>
        : std::true_type
    {
    };

    template <typename TTransportSettings>
    void applyEncodedOneWireRateIfUnset(uint32_t encodedRateHz,
                                         TTransportSettings &transportSettings)
    {
        if constexpr (TransportSettingsHasClockRateHz<TTransportSettings>::value)
        {
            if (transportSettings.clockRateHz == 0)
            {
                transportSettings.clockRateHz = encodedRateHz;
            }
        }

        if constexpr (TransportSettingsHasBaudRate<TTransportSettings>::value)
        {
            if (transportSettings.baudRate == 0)
            {
                transportSettings.baudRate = encodedRateHz;
            }
        }
    }

} // namespace factory
} // namespace lw
