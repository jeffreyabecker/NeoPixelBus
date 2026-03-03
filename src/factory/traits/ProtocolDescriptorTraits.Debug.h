#pragma once

#include <utility>
#include <type_traits>
#include <cstdint>

#include <Arduino.h>

#include "protocols/DebugProtocol.h"
#include "factory/descriptors/ProtocolDescriptors.h"
#include "factory/traits/ProtocolDescriptorTraits.h"
#include "factory/traits/ProtocolDescriptorTraits.None.h"

namespace lw
{
namespace factory
{

    template <typename TWrappedProtocolConfig,
              bool THasWrapped = !std::is_same<typename std::remove_cv<typename std::remove_reference<TWrappedProtocolConfig>::type>::type,
                                               NoneOptions>::value>
    struct DebugWrappedConfigField
    {
        TWrappedProtocolConfig wrapped{};
    };

    template <typename TWrappedProtocolConfig>
    struct DebugWrappedConfigField<TWrappedProtocolConfig, false>
    {
    };

    template <typename TWrappedProtocolConfig>
    struct DebugOptions : DebugWrappedConfigField<TWrappedProtocolConfig>
    {
        Print *output = nullptr;
        int8_t serialPortNumber = 0;
        bool invert = false;
    };

    template <typename TWrappedProtocolDesc>
    struct ProtocolDescriptorTraits<descriptors::Debug<TWrappedProtocolDesc>, void>
    {
        using WrappedTraits = ProtocolDescriptorTraits<TWrappedProtocolDesc>;
        using WrappedProtocolType = typename WrappedTraits::ProtocolType;
        using ProtocolType = lw::DebugProtocol<WrappedProtocolType>;
        using SettingsType = typename ProtocolType::SettingsType;
        using ColorType = typename ProtocolType::ColorType;

        static SettingsType defaultSettings()
        {
            SettingsType settings{};
            settings.wrapped = WrappedTraits::defaultSettings();
            return settings;
        }

        static SettingsType fromConfig(SettingsType settings)
        {
            return normalize(std::move(settings));
        }

        template <typename TWrappedProtocolConfig>
        static SettingsType fromConfig(const DebugOptions<TWrappedProtocolConfig> &config)
        {
            SettingsType settings = defaultSettings();
            constexpr bool hasWrappedConfig = !std::is_same<typename std::remove_cv<typename std::remove_reference<TWrappedProtocolConfig>::type>::type,
                                                            NoneOptions>::value;
            if constexpr (hasWrappedConfig)
            {
                settings.wrapped = WrappedTraits::fromConfig(config.wrapped);
            }
            settings.output = config.output;

            if (settings.output == nullptr)
            {
                if (config.serialPortNumber > 0)
                {
#if defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_ESP32)
                    if (config.serialPortNumber == 1)
                    {
                        settings.output = &Serial1;
                    }
                    else if (config.serialPortNumber == 2)
                    {
                        settings.output = &Serial2;
                    }
#endif

#if defined(ARDUINO_ARCH_ESP8266)
                    if (config.serialPortNumber == 1)
                    {
                        settings.output = &Serial1;
                    }
#endif
                }

                if (settings.output == nullptr)
                {
                    settings.output = &Serial;
                }
            }

            settings.invert = config.invert;
            return normalize(std::move(settings));
        }

        static SettingsType normalize(SettingsType settings)
        {
            settings.wrapped = WrappedTraits::normalize(std::move(settings.wrapped));
            return settings;
        }

        template <typename TTransportSettings>
        static void mutateTransportSettings(uint16_t pixelCount,
                                            const SettingsType &settings,
                                            TTransportSettings &transportSettings)
        {
            WrappedTraits::mutateTransportSettings(pixelCount,
                                                   settings.wrapped,
                                                   transportSettings);
        }
    };

} // namespace factory
} // namespace lw
