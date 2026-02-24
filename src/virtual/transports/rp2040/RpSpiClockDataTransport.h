#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include <cstdint>
#include <cstddef>
#include <span>

#include <Arduino.h>
#include "hardware/dma.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

#include "../ITransport.h"
#include "RpDmaStateTracker.h"

namespace npb
{

#ifndef NEOPIXELBUS_RP_SPI_CLOCK_DEFAULT_HZ
#define NEOPIXELBUS_RP_SPI_CLOCK_DEFAULT_HZ 10000000UL
#endif
    static constexpr uint32_t RpSpiClockDefaultHz = NEOPIXELBUS_RP_SPI_CLOCK_DEFAULT_HZ;

#ifndef NEOPIXELBUS_RP_SPI_DEFAULT_BUS
#define NEOPIXELBUS_RP_SPI_DEFAULT_BUS 0
#endif
    static constexpr uint8_t RpSpiDefaultBus = NEOPIXELBUS_RP_SPI_DEFAULT_BUS;

#if defined(PIN_SPI0_SCK)
    static constexpr int8_t RpSpiDefaultSckPin = PIN_SPI0_SCK;
#elif defined(SCK)
    static constexpr int8_t RpSpiDefaultSckPin = SCK;
#else
    static constexpr int8_t RpSpiDefaultSckPin = -1;
#endif

#if defined(PIN_SPI0_MOSI)
    static constexpr int8_t RpSpiDefaultMosiPin = PIN_SPI0_MOSI;
#elif defined(MOSI)
    static constexpr int8_t RpSpiDefaultMosiPin = MOSI;
#else
    static constexpr int8_t RpSpiDefaultMosiPin = -1;
#endif

    struct RpSpiTransportConfig
    {
        uint8_t spiBus = RpSpiDefaultBus;
        int8_t clockPin = RpSpiDefaultSckPin;
        int8_t dataPin = RpSpiDefaultMosiPin;
        bool invert = false;
        uint32_t clockDataBitRateHz = RpSpiClockDefaultHz;
    };

    class RpSpiTransport : public ITransport
    {
    public:
        using TransportConfigType = RpSpiTransportConfig;
        using TransportCategory = ClockDataTransportTag;
        static constexpr uint IrqIndex = 1;
        static constexpr uint32_t SpiTxFifoBytes = 8;

        explicit RpSpiTransport(RpSpiTransportConfig config)
            : _config{config}
            , _spi{resolveSpi(config.spiBus)}
        {
        }

        explicit RpSpiTransport(uint32_t clockHz = RpSpiClockDefaultHz)
            : RpSpiTransport(RpSpiTransportConfig{.clockDataBitRateHz = clockHz})
        {
        }

        ~RpSpiTransport() override
        {
            if (_dmaChannel >= 0)
            {
                while (!isReadyToUpdate())
                {
                    yield();
                }

                dma_irqn_set_channel_enabled(IrqIndex, _dmaChannel, false);
                _dmaState.unregisterChannel(_dmaChannel);

                dma_channel_unclaim(_dmaChannel);
                _dmaChannel = -1;
            }

            if (_config.invert && _config.dataPin >= 0)
            {
                gpio_set_outover(_config.dataPin, GPIO_OVERRIDE_NORMAL);
            }
        }

        void begin() override
        {
            if (_initialised)
            {
                return;
            }

            if (_spi == nullptr)
            {
                return;
            }

            spi_init(_spi, _config.clockDataBitRateHz);

            if (_config.clockPin >= 0)
            {
                gpio_set_function(_config.clockPin, GPIO_FUNC_SPI);
            }

            if (_config.dataPin >= 0)
            {
                gpio_set_function(_config.dataPin, GPIO_FUNC_SPI);
                if (_config.invert)
                {
                    gpio_set_outover(_config.dataPin, GPIO_OVERRIDE_INVERT);
                }
            }

            _dmaChannel = dma_claim_unused_channel(true);
            _dmaState.registerChannel(_dmaChannel);
            dma_irqn_set_channel_enabled(IrqIndex, _dmaChannel, true);
            _dmaConfig = dma_channel_get_default_config(_dmaChannel);
            channel_config_set_transfer_data_size(&_dmaConfig, DMA_SIZE_8);
            channel_config_set_read_increment(&_dmaConfig, true);
            channel_config_set_write_increment(&_dmaConfig, false);
            channel_config_set_dreq(&_dmaConfig, spi_get_dreq(_spi, true));

            if (_config.clockDataBitRateHz > 0)
            {
                _fifoDrainUs = static_cast<uint32_t>(
                    (static_cast<uint64_t>(SpiTxFifoBytes) * 8ULL * 1000000ULL + _config.clockDataBitRateHz - 1ULL) /
                    _config.clockDataBitRateHz);
            }
            else
            {
                _fifoDrainUs = 0;
            }

            _initialised = true;
        }

        void beginTransaction() override
        {
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            if (!_initialised)
            {
                begin();
            }

            if (!_initialised || data.empty())
            {
                return;
            }

            while (!isReadyToUpdate())
            {
                yield();
            }

            _dmaState.setSending();

            dma_channel_configure(
                _dmaChannel,
                &_dmaConfig,
                &spi_get_hw(_spi)->dr,
                data.data(),
                data.size(),
                true);
        }

        void endTransaction() override
        {
        }

        bool isReadyToUpdate() const override
        {
            if (!_initialised)
            {
                return true;
            }

            if (!_dmaState.isReadyToSend(_fifoDrainUs))
            {
                return false;
            }

            return !spi_is_busy(_spi);
        }

    private:
        RpSpiTransportConfig _config;
        spi_inst_t *_spi{nullptr};
        bool _initialised{false};

        int _dmaChannel{-1};
        dma_channel_config _dmaConfig{};
        uint32_t _fifoDrainUs{0};

        RpDmaStateTracker<IrqIndex> _dmaState;

        static spi_inst_t *resolveSpi(uint8_t bus)
        {
            switch (bus)
            {
            case 0:
                return spi0;
            case 1:
                return spi1;
            default:
                return spi0;
            }
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_RP2040
