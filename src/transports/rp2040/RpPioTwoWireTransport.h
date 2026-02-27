#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include <cstdint>
#include <cstddef>

#include <Arduino.h>
#if __has_include(<SPI.h>)
#include <SPI.h>
#endif
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"

#include "transports/ITransport.h"
#include "detail/RpDmaManager.h"
#include "detail/RpPioManager.h"

namespace npb
{

#ifndef NEOPIXELBUS_SPI_CLOCK_DEFAULT_HZ
#define NEOPIXELBUS_SPI_CLOCK_DEFAULT_HZ 10000000UL
#endif

    struct RpPioTwoWireTransportSettings
    {
        bool invert = false;
        uint32_t clockRateHz = NEOPIXELBUS_SPI_CLOCK_DEFAULT_HZ;
        BitOrder bitOrder = MSBFIRST;
        uint8_t dataMode = SPI_MODE0;
        int8_t clockPin = -1;
        int8_t dataPin = -1;
        uint8_t pioIndex = 1;
    };

    class RpPioTwoWireTransport : public ITransport
    {
    public:
        using TransportSettingsType = RpPioTwoWireTransportSettings;
        using TransportCategory = TransportTag;

        explicit RpPioTwoWireTransport(RpPioTwoWireTransportSettings config)
            : _config{config}
        {
        }

        ~RpPioTwoWireTransport()
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

            if (_config.clockPin >= 0)
            {
                pinMode(_config.clockPin, INPUT);
            }
            if (_config.dataPin >= 0)
            {
                pinMode(_config.dataPin, INPUT);
            }
        }

        void begin() override
        {
            if (_initialised)
            {
                return;
            }

            if (_config.dataPin < 0)
            {
                return;
            }

            const bool clockMapped = _config.clockPin >= 0;

            if (clockMapped && _config.dataPin != (_config.clockPin + 1))
            {
                return;
            }

            if (_config.clockRateHz == 0)
            {
                return;
            }

            if (_config.dataMode != SPI_MODE0)
            {
                return;
            }

            const uint dataPin = static_cast<uint>(_config.dataPin);
            const uint clockPin = clockMapped
                                      ? static_cast<uint>(_config.clockPin)
                                      : static_cast<uint>(_config.dataPin + 1);

            _smLease = RpPioManager::requestStateMachine(&Apa102MiniProgram::Program, static_cast<int8_t>(_config.pioIndex));
            if (!_smLease.isValid())
            {
                return;
            }

            _smLease.gpioInit(dataPin);

            if (clockMapped)
            {
                _smLease.gpioInit(clockPin);
                _smLease.setConsecutivePindirs(clockPin, 2, true);
            }
            else
            {
                _smLease.setConsecutivePindirs(dataPin, 1, true);
            }

            const bool shiftRight = (_config.bitOrder == LSBFIRST);

            _smLease.smConfig()
                .setWrap(_smLease.programOffset() + Apa102MiniProgram::WrapTarget,
                         _smLease.programOffset() + Apa102MiniProgram::Wrap)
                .setSideset(1, false, false)
                .setOutPins(dataPin, 1)
                .setSidesetPins(clockPin)
                .setOutShift(shiftRight, true, 32)
                .setFifoJoin(PIO_FIFO_JOIN_TX)
                .setClockDivisor(2u * _config.clockRateHz);

            _smLease.init();
            _smLease.setEnabled(true);

            if (_config.invert)
            {
                gpio_set_outover(dataPin, GPIO_OVERRIDE_INVERT);
                if (clockMapped)
                {
                    gpio_set_outover(clockPin, GPIO_OVERRIDE_INVERT);
                }
            }

            _dmaLease = _dmaManager.requestChannel();
            if (!_dmaLease.isValid())
            {
                _smLease.clearFifos();
                _smLease.setEnabled(false);
                _smLease.release();
                pinMode(dataPin, INPUT);
                if (clockMapped)
                {
                    pinMode(clockPin, INPUT);
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
                const_cast<RpDmaManager &>(_dmaManager).setIdle();
                return true;
            }

            return true;
        }

    private:
        struct Apa102MiniProgram
        {
            static constexpr uint WrapTarget = 0;
            static constexpr uint Wrap = 1;

            static inline const uint16_t Instructions[] =
                {
                    0x6001,
                    0xb042,
                };

            static inline const pio_program Program =
                {
                    .instructions = Instructions,
                    .length = 2,
                    .origin = -1,
                    .pio_version = 0,
                    .used_gpio_ranges = 0,
                };
        };

        RpPioTwoWireTransportSettings _config;
        RpPioManager::StateMachineLease _smLease;
        RpDmaManager _dmaManager;
        RpDmaManager::ChannelLease _dmaLease;
        bool _initialised{false};
    };

} // namespace npb

#endif // ARDUINO_ARCH_RP2040
