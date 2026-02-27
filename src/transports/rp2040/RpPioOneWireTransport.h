#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include <cstdint>
#include <cstddef>

#include <Arduino.h>
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"

#include "transports/ITransport.h"
#include "RpDmaStateTracker.h"
#include "RpPioMonoProgram.h"
#include "transports/OneWireTiming.h"

namespace npb
{

    struct RpPioOneWireTransportSettings
    {
        uint8_t pin = 0;
        uint8_t pioIndex = 1;
        bool invert = false;
        OneWireTiming timing = timing::Ws2812x;
    };

    class RpPioOneWireTransport : public ITransport
    {
    public:
        using TransportSettingsType = RpPioOneWireTransportSettings;
        using TransportCategory = OneWireTransportTag;
        explicit RpPioOneWireTransport(RpPioOneWireTransportSettings config)
            : _config{config}
            , _pio{resolvePio(config.pioIndex)}
            , _fifoCacheEmptyDelta{computeFifoCacheEmptyDeltaUs(config.timing.bitPeriodNs())}
        {
        }

        ~RpPioOneWireTransport()
        {
            if (!_initialised)
            {
                return;
            }

            while (!isReadyToUpdate())
            {
                yield();
            }

            pio_sm_clear_fifos(_pio, _sm);
            pio_sm_set_enabled(_pio, _sm, false);

            _dmaState.releaseChannel(static_cast<uint>(_dmaChannel));
            pio_sm_unclaim(_pio, _sm);

            pinMode(_config.pin, INPUT);
        }

        void begin() override
        {
            if (_initialised)
            {
                return;
            }

            EncodedClockDataBitPattern bitPattern = _config.timing.bitPattern();
            uint8_t bitCycles = (bitPattern == EncodedClockDataBitPattern::FourStep) ? RpPioCadence4Step::BitCycles : RpPioCadence3Step::BitCycles;
            const uint fifoWordBits = 8;

            uint offset = (bitPattern == EncodedClockDataBitPattern::FourStep) ? RpPioMonoProgram::load4Step(_pio) : RpPioMonoProgram::load3Step(_pio);

            _sm = pio_claim_unused_sm(_pio, true);

            RpPioMonoProgram::initSm(
                _pio,
                _sm,
                offset,
                _config.pin,
                _config.timing.bitRateHz(),
                bitCycles,
                fifoWordBits);

            if (_config.invert)
            {
                gpio_set_outover(_config.pin, GPIO_OVERRIDE_INVERT);
            }

            _dmaChannel = static_cast<int>(_dmaState.claimChannel());

            _initialised = true;
        }

        void transmitBytes(span<const uint8_t> data) override
        {
            if (!_initialised)
            {
                begin();
            }

            if (!_initialised || data.empty())
            {
                return;
            }
            _dmaState.setSending();

            const dma_channel_transfer_size dmaTransferSize =
                (data.size() % 4 == 0) ? DMA_SIZE_32 :
                ((data.size() % 2 == 0) ? DMA_SIZE_16 : DMA_SIZE_8);
            const uint transferCount = static_cast<uint>(
                data.size() / (static_cast<size_t>(dmaTransferSize) + 1U));

            dma_channel_config cfg = dma_channel_get_default_config(_dmaChannel);
            channel_config_set_transfer_data_size(&cfg, dmaTransferSize);
            channel_config_set_read_increment(&cfg, true);
            channel_config_set_write_increment(&cfg, false);
            channel_config_set_bswap(&cfg, true);
            channel_config_set_dreq(&cfg, pio_get_dreq(_pio, _sm, true));

            dma_channel_configure(
                _dmaChannel,
                &cfg,
                &(_pio->txf[_sm]),
                nullptr,
                transferCount,
                false);


            dma_channel_set_read_addr(_dmaChannel, data.data(), true);
            
        }

        bool isReadyToUpdate() const override
        {
            if (!_initialised)
            {
                return true;
            }

            if (_dmaState.isSending())
            {
                return false;
            }

            if (_dmaState.hasDmaCompleted())
            {
                const uint32_t holdoffUs = _config.timing.resetUs + _fifoCacheEmptyDelta;
                if (_dmaState.elapsedSinceDmaCompleteUs() >= holdoffUs)
                {
                    const_cast<RpDmaStateTracker<IrqIndex> &>(_dmaState).setIdle();
                    return true;
                }
                return false;
            }

            return true;
        }

    private:
        static constexpr uint IrqIndex = 1;

        RpPioOneWireTransportSettings _config;
        PIO _pio;

        int _sm{-1};
        int _dmaChannel{-1};
        const uint32_t _fifoCacheEmptyDelta;
        bool _initialised{false};

        RpDmaStateTracker<IrqIndex> _dmaState;

        static PIO resolvePio(uint8_t index)
        {
            switch (index)
            {
            case 0:
                return pio0;
#if NUM_PIOS >= 2
            case 1:
                return pio1;
#endif
#if NUM_PIOS >= 3
            case 2:
                return pio2;
#endif
            default:
                return pio0;
            }
        }

        static uint32_t computeFifoCacheEmptyDeltaUs(uint32_t bitPeriodNs)
        {
            constexpr uint8_t mergedFifoCount = 32*2; // always 32 per state machine, lets double that to be safe;
            const uint64_t fifoDrainNs = static_cast<uint64_t>(bitPeriodNs) *
                                         static_cast<uint64_t>(8U) *
                                         static_cast<uint64_t>(mergedFifoCount + 1U);
            return static_cast<uint32_t>((fifoDrainNs + 999ULL) / 1000ULL);
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_RP2040


