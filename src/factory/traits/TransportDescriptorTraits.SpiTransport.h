#pragma once

#if defined(NPB_FACTORY_ENABLE_SPI_DESCRIPTOR_TRAITS)

#include "factory/descriptors/TransportDescriptors.h"
#include "factory/traits/TransportDescriptorTraits.h"
#include "transports/SpiTransport.h"

namespace npb
{
namespace factory
{

#if defined(NPB_HAS_SPI_TRANSPORT)

    struct NeoSpiOptions
    {
        SPIClass *spi = nullptr;
        uint32_t clockRateHz = SpiClockDefaultHz;
        BitOrder bitOrder = MSBFIRST;
        uint8_t dataMode = SPI_MODE0;
        bool invert = false;
    };

    template <>
    struct TransportDescriptorTraits<descriptors::NeoSpi, void>
    {
        using TransportType = npb::SpiTransport;
        using SettingsType = SpiTransportSettings;

        static SettingsType defaultSettings()
        {
            return SettingsType{};
        }

        static SettingsType normalize(SettingsType settings)
        {
            if (settings.clockRateHz == 0)
            {
                settings.clockRateHz = SpiClockDefaultHz;
            }

            return settings;
        }

        static SettingsType fromConfig(SettingsType settings)
        {
            return settings;
        }

        static SettingsType fromConfig(const NeoSpiOptions &config)
        {
            SettingsType settings{};
            settings.spi = config.spi;
            settings.clockRateHz = config.clockRateHz;
            settings.bitOrder = config.bitOrder;
            settings.dataMode = config.dataMode;
            settings.invert = config.invert;
            return settings;
        }
    };

#endif

} // namespace factory
} // namespace npb

#endif
