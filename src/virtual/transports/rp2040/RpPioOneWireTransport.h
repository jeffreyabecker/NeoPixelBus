#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include <cstdint>
#include <cstddef>
#include <span>

#include <Arduino.h>
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"

#include "../ITransport.h"
#include "RpDmaStateTracker.h"
#include "RpPioMonoProgram.h"
#include "../OneWireTiming.h"

namespace npb
{

    struct RpPioOneWireTransportConfig
    {
        uint8_t pin = 0;
        uint8_t pioIndex = 1;
        size_t frameBytes = 0;
        bool invert = false;
        OneWireTiming timing = timing::Ws2812x;
    };

    class RpPioOneWireTransport : public ITransport
    {
    public:
        using TransportConfigType = RpPioOneWireTransportConfig;
        using TransportCategory = SelfClockingTransportTag;
        explicit RpPioOneWireTransport(RpPioOneWireTransportConfig config)
            : _config{config}
            , _pio{resolvePio(config.pioIndex)}
            , _mergedFifoCount{static_cast<uint8_t>((_pio->dbg_cfginfo & PIO_DBG_CFGINFO_FIFO_DEPTH_BITS) * 2)}
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

            dma_irqn_set_channel_enabled(IrqIndex, _dmaChannel, false);
            _dmaState.unregisterChannel(_dmaChannel);

            dma_channel_unclaim(_dmaChannel);
            pio_sm_unclaim(_pio, _sm);

            pinMode(_config.pin, INPUT);
        }

        void begin() override
        {
            if (_initialised)
            {
                return;
            }

            if (_config.frameBytes == 0)
            {
                return;
            }

            uint fifoWordBits = 8;
            if (_config.frameBytes % 4 == 0)
            {
                fifoWordBits = 32;
            }
            else if (_config.frameBytes % 2 == 0)
            {
                fifoWordBits = 16;
            }

            _dmaTransferCount = static_cast<uint>(_config.frameBytes / (fifoWordBits / 8));
            auto dmaTransferSize = static_cast<dma_channel_transfer_size>(fifoWordBits / 16);

            float bitLengthUs = 1'000'000.0f / _config.timing.bitRateHz();
            _fifoCacheEmptyDelta = static_cast<uint32_t>(bitLengthUs * fifoWordBits * (_mergedFifoCount + 1));

            bool fourStep = (2 * _config.timing.t1hNs) > (3 * _config.timing.t0hNs);
            uint8_t bitCycles = fourStep ? RpPioCadence4Step::BitCycles : RpPioCadence3Step::BitCycles;

            uint offset = fourStep ? RpPioMonoProgram::load4Step(_pio) : RpPioMonoProgram::load3Step(_pio);

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

            _dmaChannel = dma_claim_unused_channel(true);
            _dmaState.registerChannel(_dmaChannel);

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
                _dmaTransferCount,
                false);

            dma_irqn_set_channel_enabled(IrqIndex, _dmaChannel, true);

            _initialised = true;
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

            if (_config.frameBytes != 0 && data.size() != _config.frameBytes)
            {
                return;
            }

            _dmaState.setSending();
            dma_channel_set_read_addr(_dmaChannel, data.data(), false);
            dma_channel_start(_dmaChannel);
        }

        bool isReadyToUpdate() const override
        {
            if (!_initialised)
            {
                return true;
            }

            return _dmaState.isReadyToSend(_config.timing.resetUs + _fifoCacheEmptyDelta);
        }

    private:
        static constexpr uint IrqIndex = 1;

        RpPioOneWireTransportConfig _config;
        PIO _pio;
        uint8_t _mergedFifoCount;

        int _sm{-1};
        int _dmaChannel{-1};
        uint _dmaTransferCount{0};
        uint32_t _fifoCacheEmptyDelta{0};
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
    };

} // namespace npb

#endif // ARDUINO_ARCH_RP2040
