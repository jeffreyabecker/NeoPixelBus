#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include <cstdint>
#include <cstddef>

#include <Arduino.h>
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"

#include "transports/ITransport.h"
#include "detail/RpDmaManager.h"
#include "detail/RpPioManager.h"
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
            : _config{config}, _holdoffUs{config.timing.resetUs + RpDmaManager::computeFifoCacheEmptyDeltaUs(config.timing.bitPeriodNs())}
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

            if (_smLease.isValid())
            {
                _smLease.clearFifos();
                _smLease.setEnabled(false);
            }

            _dmaLease.release();
            _smLease.release();

            pinMode(_config.pin, INPUT);
        }

        void begin() override
        {
            if (_initialised)
            {
                return;
            }

            EncodedClockDataBitPattern bitPattern = _config.timing.bitPattern();
            uint8_t bitCycles = (bitPattern == EncodedClockDataBitPattern::FourStep) ? Cadence4Step::BitCycles : Cadence3Step::BitCycles;
            const uint fifoWordBits = 8;

            const pio_program *program = (bitPattern == EncodedClockDataBitPattern::FourStep)
                                             ? &Cadence4Step::Program
                                             : &Cadence3Step::Program;

            _smLease = RpPioManager::requestStateMachine(program, static_cast<int8_t>(_config.pioIndex));
            if (!_smLease.isValid())
            {
                return;
            }

            _smLease.smConfig()
                .setWrap(_smLease.programOffset() + 0, _smLease.programOffset() + 3)
                .setSideset(1, false, false)
                .setSidesetPins(_config.pin)
                .setOutShift(false, true, fifoWordBits)
                .setFifoJoin(PIO_FIFO_JOIN_TX)
                .setClockDivisor(_config.timing.bitRateHz() * bitCycles);

            _smLease.gpioInit(_config.pin);
            _smLease.setConsecutivePindirs(_config.pin, 1, true);
            _smLease.init();
            _smLease.setEnabled(true);

            if (_config.invert)
            {
                gpio_set_outover(_config.pin, GPIO_OVERRIDE_INVERT);
            }

            _dmaLease = _dmaManager.requestChannel();
            if (!_dmaLease.isValid())
            {
                _smLease.clearFifos();
                _smLease.setEnabled(false);
                _smLease.release();
                pinMode(_config.pin, INPUT);
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

            if (!_initialised || data.empty())
            {
                return;
            }
            _dmaLease.startTransfer(
                data,
                _smLease.txFifoWriteAddress(),
                _smLease.dreq(true));
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
                if (_dmaManager.elapsedSinceDmaCompleteUs() >= _holdoffUs)
                {
                    const_cast<RpDmaManager &>(_dmaManager).setIdle();
                    return true;
                }
                return false;
            }

            return true;
        }

    private:
        struct Cadence3Step
        {
            static constexpr uint8_t BitCycles = 3;

            static inline const uint16_t Instructions[] =
                {
                    0x6021,
                    0x1023,
                    0x1000,
                    0xa042,
                };

            static inline const pio_program Program =
                {
                    .instructions = Instructions,
                    .length = 4,
                    .origin = -1,
                    .pio_version = 0,
                    .used_gpio_ranges = 0,
                };
        };

        struct Cadence4Step
        {
            static constexpr uint8_t BitCycles = 4;

            static inline const uint16_t Instructions[] =
                {
                    0x6021,
                    0x1023,
                    0x1100,
                    0xa142,
                };

            static inline const pio_program Program =
                {
                    .instructions = Instructions,
                    .length = 4,
                    .origin = -1,
                    .pio_version = 0,
                    .used_gpio_ranges = 0,
                };
        };

        RpPioOneWireTransportSettings _config;
        RpPioManager::StateMachineLease _smLease;
        RpDmaManager _dmaManager;
        RpDmaManager::ChannelLease _dmaLease;
        const uint32_t _holdoffUs;
        bool _initialised{false};
    };

} // namespace npb

#endif // ARDUINO_ARCH_RP2040
