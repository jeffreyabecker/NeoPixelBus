#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include <cstdint>
#include <cstddef>

#include <Arduino.h>
#if __has_include(<SPI.h>)
#include <SPI.h>
#endif

#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

#include "transports/ITransport.h"
#include "detail/RpDmaManager.h"

namespace npb
{

#ifndef NEOPIXELBUS_SPI_CLOCK_DEFAULT_HZ
#define NEOPIXELBUS_SPI_CLOCK_DEFAULT_HZ 10000000UL
#endif

    static constexpr uint32_t SpiClockDefaultHz = NEOPIXELBUS_SPI_CLOCK_DEFAULT_HZ;

    struct RpSpiTransportSettings
    {
        bool invert = false;
        uint32_t clockRateHz = SpiClockDefaultHz;
        BitOrder bitOrder = MSBFIRST;
        uint8_t dataMode = SPI_MODE0;
        int8_t clockPin = -1;
        int8_t dataPin = -1;
    };

    class RpSpiTransport : public ITransport
    {
    public:
        using TransportSettingsType = RpSpiTransportSettings;
        using TransportCategory = TransportTag;

        explicit RpSpiTransport(RpSpiTransportSettings config)
            : _config{config}
        {
        }

        ~RpSpiTransport()
        {
            if (!_initialised)
            {
                return;
            }

            while (!isReadyToUpdate())
            {
                yield();
            }

            _dmaLease.release();

            if (_config.dataPin >= 0)
            {
                pinMode(_config.dataPin, INPUT);
            }
            if (_config.clockPin >= 0)
            {
                pinMode(_config.clockPin, INPUT);
            }
        }

        void begin() override
        {
            if (_initialised)
            {
                return;
            }

            if (_config.dataPin < 0 || _config.clockRateHz == 0)
            {
                return;
            }

            _spi = resolveSpi();
            if (_spi == nullptr)
            {
                return;
            }

            spi_init(_spi, _config.clockRateHz);
            spi_set_format(
                _spi,
                8,
                (_config.dataMode == SPI_MODE2 || _config.dataMode == SPI_MODE3) ? SPI_CPOL_1 : SPI_CPOL_0,
                (_config.dataMode == SPI_MODE1 || _config.dataMode == SPI_MODE3) ? SPI_CPHA_1 : SPI_CPHA_0,
                (_config.bitOrder == LSBFIRST) ? SPI_LSB_FIRST : SPI_MSB_FIRST);

            if (_config.dataPin >= 0)
            {
                gpio_set_function(static_cast<uint>(_config.dataPin), GPIO_FUNC_SPI);
            }
            if (_config.clockPin >= 0)
            {
                gpio_set_function(static_cast<uint>(_config.clockPin), GPIO_FUNC_SPI);
            }

            if (_config.invert)
            {
                if (_config.dataPin >= 0)
                {
                    gpio_set_outover(static_cast<uint>(_config.dataPin), GPIO_OVERRIDE_INVERT);
                }
                if (_config.clockPin >= 0)
                {
                    gpio_set_outover(static_cast<uint>(_config.clockPin), GPIO_OVERRIDE_INVERT);
                }
            }

            _dmaLease = _dmaManager.requestChannel();
            if (!_dmaLease.isValid())
            {
                if (_config.dataPin >= 0)
                {
                    pinMode(_config.dataPin, INPUT);
                }
                if (_config.clockPin >= 0)
                {
                    pinMode(_config.clockPin, INPUT);
                }
                return;
            }

            _initialised = true;
        }

        void transmitBytes(span<const uint8_t> data) override
        {
            if (!_initialised)
            {
                begin();
            }

            if (!_initialised || data.empty() || _spi == nullptr)
            {
                return;
            }

            _dmaLease.startTransferWithSize(
                data,
                static_cast<volatile void *>(&(spi_get_hw(_spi)->dr)),
                spi_get_dreq(_spi, true),
                DMA_SIZE_8,
                false,
                true,
                false);
        }

        bool isReadyToUpdate() const override
        {
            if (!_initialised)
            {
                return true;
            }

            if (_dmaManager.isSending())
            {
                return false;
            }

            if (_dmaManager.hasDmaCompleted())
            {
                if (_spi != nullptr && spi_is_busy(_spi))
                {
                    return false;
                }

                const_cast<RpDmaManager &>(_dmaManager).setIdle();
                return true;
            }

            return true;
        }

    private:
        RpSpiTransportSettings _config;
        RpDmaManager _dmaManager;
        RpDmaManager::ChannelLease _dmaLease;
        spi_inst_t *_spi{nullptr};
        bool _initialised{false};

        spi_inst_t *resolveSpi() const
        {
            const int dataSpi = pinToSpiIndex(_config.dataPin, false);
            const int clockSpi = (_config.clockPin >= 0) ? pinToSpiIndex(_config.clockPin, true) : -1;

            if (_config.clockPin >= 0)
            {
                if (dataSpi < 0 || clockSpi < 0)
                {
                    return nullptr;
                }
                if (dataSpi != clockSpi)
                {
                    return nullptr;
                }
                return (dataSpi == 0) ? spi0 : spi1;
            }

            if (dataSpi < 0)
            {
                return nullptr;
            }
            return (dataSpi == 0) ? spi0 : spi1;
        }

        static int pinToSpiIndex(int pin, bool isClockPin)
        {
            if (pin < 0)
            {
                return -1;
            }

            const uint gpio = static_cast<uint>(pin);

            if (isClockPin)
            {
                switch (gpio)
                {
                case 2:
                case 6:
                case 18:
                case 22:
                    return 0;
                case 10:
                case 14:
                case 26:
                    return 1;
                default:
                    return -1;
                }
            }

            switch (gpio)
            {
            case 3:
            case 7:
            case 19:
            case 23:
                return 0;
            case 11:
            case 15:
            case 27:
                return 1;
            default:
                return -1;
            }
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_RP2040
