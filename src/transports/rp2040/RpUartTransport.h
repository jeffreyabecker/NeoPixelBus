#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include <cstdint>
#include <cstddef>

#include <Arduino.h>

#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"

#include "transports/ITransport.h"
#include "detail/RpDmaManager.h"

namespace npb
{

#ifndef NEOPIXELBUS_UART_BAUD_DEFAULT
#define NEOPIXELBUS_UART_BAUD_DEFAULT 3200000UL
#endif

    static constexpr uint32_t UartBaudDefault = NEOPIXELBUS_UART_BAUD_DEFAULT;

    struct RpUartTransportSettings
    {
        bool invert = false;
        uint32_t baudRate = UartBaudDefault;
        uint8_t uartIndex = 0;
        int8_t txPin = -1;
    };

    class RpUartTransport : public ITransport
    {
    public:
        using TransportSettingsType = RpUartTransportSettings;
        using TransportCategory = TransportTag;

        explicit RpUartTransport(RpUartTransportSettings config)
            : _config{config},
              _holdoffUs{RpDmaManager::computeFifoCacheEmptyDeltaUs(computeBitPeriodNs(config.baudRate))}
        {
        }

        ~RpUartTransport()
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

            if (_uart != nullptr)
            {
                uart_deinit(_uart);
            }

            if (_config.txPin >= 0)
            {
                pinMode(_config.txPin, INPUT);
            }
        }

        void begin() override
        {
            if (_initialised)
            {
                return;
            }

            if (_config.txPin < 0 || _config.baudRate == 0)
            {
                return;
            }

            _uart = resolveUart();
            if (_uart == nullptr)
            {
                return;
            }

            uart_init(_uart, _config.baudRate);
            uart_set_format(_uart, 8, 1, UART_PARITY_NONE);
            uart_set_hw_flow(_uart, false, false);
            uart_set_fifo_enabled(_uart, true);

            gpio_set_function(static_cast<uint>(_config.txPin), GPIO_FUNC_UART);

            if (_config.invert)
            {
                gpio_set_outover(static_cast<uint>(_config.txPin), GPIO_OVERRIDE_INVERT);
            }

            _dmaLease = _dmaManager.requestChannel();
            if (!_dmaLease.isValid())
            {
                uart_deinit(_uart);
                _uart = nullptr;
                pinMode(_config.txPin, INPUT);
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

            if (!_initialised || data.empty() || _uart == nullptr)
            {
                return;
            }

            _dmaLease.startTransferWithSize(
                data,
                static_cast<volatile void *>(&(uart_get_hw(_uart)->dr)),
                uart_get_dreq(_uart, true),
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
                if (_dmaManager.elapsedSinceDmaCompleteUs() < _holdoffUs)
                {
                    return false;
                }

                const_cast<RpDmaManager &>(_dmaManager).setIdle();
                return true;
            }

            return true;
        }

    private:
        RpUartTransportSettings _config;
        RpDmaManager _dmaManager;
        RpDmaManager::ChannelLease _dmaLease;
        uart_inst_t *_uart{nullptr};
        const uint32_t _holdoffUs;
        bool _initialised{false};

        static uint32_t computeBitPeriodNs(uint32_t bitRateHz)
        {
            if (bitRateHz == 0)
            {
                return 0;
            }

            return static_cast<uint32_t>((1000000000ULL + static_cast<uint64_t>(bitRateHz) - 1ULL) /
                                         static_cast<uint64_t>(bitRateHz));
        }

        uart_inst_t *resolveUart() const
        {
            if (_config.uartIndex > 1)
            {
                return nullptr;
            }

            return (_config.uartIndex == 0) ? uart0 : uart1;
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_RP2040
