#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>
#include <memory>
#include <vector>
#include <algorithm>
#include <utility>

#include <Arduino.h>
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"

#include "IProtocol.h"
#include "ColorOrderTransform.h"
#include "OneWireTiming.h"
#include "RpPioDmaState.h"
#include "RpPioMonoProgram.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"
#include "../colors/Color.h"

namespace npb
{

    /// Construction settings for RpPioOneWireProtocol.
    struct RpPioOneWireProtocolSettings
    {
        uint8_t pin;
        uint8_t pioIndex = 1;              // 0 = PIO0, 1 = PIO1 (2 on RP2350)
        OneWireTiming timing = timing::Ws2812x;
        bool invert = false;               // compensate for external inverting hardware
        ColorOrderTransformConfig colorConfig;
    };

    /// One-wire NRZ emitter for RP2040/RP2350 using PIO + DMA.
    ///
    /// Each instance drives a single strip on one pin.  Internally it claims
    /// the next available state machine on the selected PIO block.  Up to 4
    /// strips can share one PIO block (one emitter per SM).
    ///
    /// The user is responsible for choosing the PIO block and not exceeding
    /// the available state machines.
    ///
    /// DMA channels are claimed cooperatively via `dma_claim_unused_channel()`
    /// so multiple PIO-based emitter types can coexist.
    class RpPioOneWireProtocol : public IProtocol
    {
    public:
        RpPioOneWireProtocol(uint16_t pixelCount,
                            ResourceHandle<IShader> shader,
                            RpPioOneWireProtocolSettings settings)
            : _settings{settings}
            , _shader{std::move(shader)}
            , _transform{settings.colorConfig}
            , _pixelCount{pixelCount}
            , _sizeData{_transform.bytesNeeded(pixelCount)}
            , _scratchColors(pixelCount)
            , _dataEditing(static_cast<uint8_t *>(malloc(_sizeData)))
            , _dataSending(static_cast<uint8_t *>(malloc(_sizeData)))
            , _pio{resolvePio(settings.pioIndex)}
            , _mergedFifoCount{static_cast<uint8_t>(
                  (_pio->dbg_cfginfo & PIO_DBG_CFGINFO_FIFO_DEPTH_BITS) * 2)}
        {
            // Zero the editing buffer (sending buffer is overwritten before use)
            if (_dataEditing)
            {
                std::memset(_dataEditing, 0, _sizeData);
            }
        }

        ~RpPioOneWireProtocol()
        {
            if (_initialised)
            {
                // Wait for any in-flight DMA to complete
                while (!_dmaState.isReadyToSend(
                    _settings.timing.resetUs + _fifoCacheEmptyDelta))
                {
                    yield();
                }

                pio_sm_clear_fifos(_pio, _sm);
                pio_sm_set_enabled(_pio, _sm, false);

                dma_irqn_set_channel_enabled(IrqIndex, _dmaChannel, false);
                _dmaState.unregisterChannel(_dmaChannel);

                dma_channel_unclaim(_dmaChannel);
                pio_sm_unclaim(_pio, _sm);

                pinMode(_settings.pin, INPUT);
            }

            free(_dataEditing);
            free(_dataSending);
        }

        // Non-copyable, non-movable (owns hardware resources)
        RpPioOneWireProtocol(const RpPioOneWireProtocol &) = delete;
        RpPioOneWireProtocol &operator=(const RpPioOneWireProtocol &) = delete;
        RpPioOneWireProtocol(RpPioOneWireProtocol &&) = delete;
        RpPioOneWireProtocol &operator=(RpPioOneWireProtocol &&) = delete;

        void initialize() override
        {
            if (_initialised)
            {
                return;
            }

            // ---- FIFO word width ----
            uint fifoWordBits = 8;
            if (_sizeData % 4 == 0)
            {
                fifoWordBits = 32;
            }
            else if (_sizeData % 2 == 0)
            {
                fifoWordBits = 16;
            }

            auto dmaTransferSize =
                static_cast<dma_channel_transfer_size>(fifoWordBits / 16);
            uint dmaTransferCount = _sizeData / (fifoWordBits / 8);

            // ---- FIFO-drain fudge factor ----
            float bitLengthUs = 1'000'000.0f / _settings.timing.bitRateHz();
            _fifoCacheEmptyDelta =
                static_cast<uint32_t>(bitLengthUs * fifoWordBits
                                      * (_mergedFifoCount + 1));

            // ---- Cadence selection ----
            // Heuristic: if T1H > 1.5 × T0H the protocol needs asymmetric
            // pulse widths → 4-step cadence.  Otherwise 3-step is sufficient.
            // Multiply instead of divide to stay in integer math:
            //   T1H > 1.5 * T0H  ⟺  2·T1H > 3·T0H
            bool fourStep = (2 * _settings.timing.t1hNs) > (3 * _settings.timing.t0hNs);
            uint8_t bitCycles = fourStep
                                    ? RpPioCadence4Step::BitCycles
                                    : RpPioCadence3Step::BitCycles;

            uint offset = fourStep
                              ? RpPioMonoProgram::load4Step(_pio)
                              : RpPioMonoProgram::load3Step(_pio);

            // ---- State machine ----
            _sm = pio_claim_unused_sm(_pio, true);

            RpPioMonoProgram::initSm(
                _pio, _sm, offset, _settings.pin,
                _settings.timing.bitRateHz(), bitCycles, fifoWordBits);

            // Apply GPIO inversion if requested
            if (_settings.invert)
            {
                gpio_set_outover(_settings.pin, GPIO_OVERRIDE_INVERT);
            }

            // ---- DMA channel ----
            _dmaChannel = dma_claim_unused_channel(true);
            _dmaState.registerChannel(_dmaChannel);

            dma_channel_config cfg = dma_channel_get_default_config(_dmaChannel);
            channel_config_set_transfer_data_size(&cfg, dmaTransferSize);
            channel_config_set_read_increment(&cfg, true);
            channel_config_set_write_increment(&cfg, false);
            channel_config_set_bswap(&cfg, true); // byte-stream → FIFO word endian fix
            channel_config_set_dreq(&cfg,
                                    pio_get_dreq(_pio, _sm, true));

            dma_channel_configure(
                _dmaChannel,
                &cfg,
                &(_pio->txf[_sm]),   // dest: PIO TX FIFO
                _dataSending,        // src: pixel data
                dmaTransferCount,
                false);              // don't start yet

            dma_irqn_set_channel_enabled(IrqIndex, _dmaChannel, true);

            _initialised = true;
        }

        void update(std::span<const Color> colors) override
        {
            // Wait for any prior transfer to complete
            while (!isReadyToUpdate())
            {
                yield();
            }

            // Apply shaders
            std::span<const Color> source = colors;
            if (nullptr != _shader)
            {
                std::copy(colors.begin(), colors.end(), _scratchColors.begin());
                _shader->apply(_scratchColors);
                source = _scratchColors;
            }

            // Transform colors → bytes into the editing buffer
            _transform.apply(
                std::span<uint8_t>{_dataEditing, _sizeData}, source);

            // Start DMA from the editing buffer
            _dmaState.setSending();
            dma_channel_set_read_addr(_dmaChannel, _dataEditing, false);
            dma_channel_start(_dmaChannel);

            // Copy editing → sending for buffer consistency, then swap
            std::memcpy(_dataSending, _dataEditing, _sizeData);
            std::swap(_dataSending, _dataEditing);
        }

        bool isReadyToUpdate() const override
        {
            return _dmaState.isReadyToSend(
                _settings.timing.resetUs + _fifoCacheEmptyDelta);
        }

        bool alwaysUpdate() const override
        {
            return false;
        }

    private:
        static constexpr uint IrqIndex = 1; // Use DMA_IRQ_1

        RpPioOneWireProtocolSettings _settings;
        ResourceHandle<IShader> _shader;
        ColorOrderTransform _transform;
        uint16_t _pixelCount;
        size_t _sizeData;

        std::vector<Color> _scratchColors;

        uint8_t *_dataEditing;           // user-facing buffer
        uint8_t *_dataSending;           // DMA source buffer

        PIO _pio;
        uint8_t _mergedFifoCount;
        int _sm{-1};
        int _dmaChannel{-1};
        uint32_t _fifoCacheEmptyDelta{0};
        bool _initialised{false};

        RpPioDmaState<IrqIndex> _dmaState;

        /// Resolve PIO block index to PIO instance pointer.
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
                return pio0; // fallback
            }
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_RP2040
