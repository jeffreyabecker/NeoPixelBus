#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include <cstdint>
#include <cstddef>

#include <Arduino.h>
#include "hardware/dma.h"
#include "hardware/irq.h"

namespace npb
{

    /// DMA completion tracking states.
    enum class RpDmaState : uint8_t
    {
        Sending,       // DMA transfer in progress
        DmaCompleted,  // DMA finished, FIFO may still be draining
        Idle           // FIFO empty, reset time elapsed — ready for next send
    };

    /// Tracks DMA completion and enforces the chip's reset/latch timing.
    ///
    /// One instance per protocol.  All instances sharing the same IRQ index
    /// cooperate through a shared static IRQ handler that iterates
    /// a table of registered DMA channels.
    ///
    /// Template parameter V_IRQ_INDEX selects DMA_IRQ_0 (0) or DMA_IRQ_1 (1).
    template <uint V_IRQ_INDEX = 1>
    class RpPioDmaState
    {
    public:
        RpPioDmaState() = default;

        /// Mark the channel as actively sending.  Call just before
        /// starting (or restarting) a DMA transfer.
        void setSending()
        {
            _state = RpDmaState::Sending;
        }

        /// Called from the shared ISR when the DMA channel finishes.
        void dmaFinished()
        {
            _endTime = micros();
            _state = RpDmaState::DmaCompleted;
        }

        /// Returns true when the reset interval (plus FIFO-drain fudge)
        /// has elapsed after the last DMA transfer completed.
        bool isReadyToSend(uint32_t resetTimeUs) const
        {
            switch (_state)
            {
            case RpDmaState::Sending:
                return false;

            case RpDmaState::DmaCompleted:
            {
                uint32_t delta = micros() - _endTime;
                if (delta >= resetTimeUs)
                {
                    // Transition to Idle (safe: ISR only writes DmaCompleted)
                    const_cast<volatile RpDmaState &>(_state) = RpDmaState::Idle;
                    return true;
                }
                return false;
            }

            default: // Idle
                return true;
            }
        }

        /// Register this instance for IRQ callbacks on the given DMA channel.
        void registerChannel(uint dmaChannel)
        {
            if (s_table[dmaChannel] != nullptr)
            {
                return; // already registered
            }

            s_table[dmaChannel] = this;

            int32_t prev = static_cast<int32_t>(s_refCount);
            s_refCount = prev + 1;
            if (prev == 0)
            {
                constexpr uint irqNum = V_IRQ_INDEX ? DMA_IRQ_1 : DMA_IRQ_0;
                irq_add_shared_handler(irqNum, dmaIrqHandler,
                                       PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
                irq_set_enabled(irqNum, true);
            }
        }

        /// Unregister this instance from the given DMA channel.
        void unregisterChannel(uint dmaChannel)
        {
            if (s_table[dmaChannel] != this)
            {
                return;
            }

            s_table[dmaChannel] = nullptr;

            int32_t prev = static_cast<int32_t>(s_refCount);
            s_refCount = prev - 1;
            if (prev == 1)
            {
                constexpr uint irqNum = V_IRQ_INDEX ? DMA_IRQ_1 : DMA_IRQ_0;
                irq_set_enabled(irqNum, false);
                irq_remove_handler(irqNum, dmaIrqHandler);
            }
        }

    private:
        volatile uint32_t _endTime{0};
        volatile RpDmaState _state{RpDmaState::Idle};

        // ---- shared statics (one set per IRQ index) ----

        static RpPioDmaState *s_table[NUM_DMA_CHANNELS];
        static volatile int32_t s_refCount;

        /// Shared IRQ handler — iterates all registered channels.
        static void dmaIrqHandler()
        {
            for (uint ch = 0; ch < NUM_DMA_CHANNELS; ++ch)
            {
                RpPioDmaState *obj = s_table[ch];
                if (obj != nullptr && dma_irqn_get_channel_status(V_IRQ_INDEX, ch))
                {
                    dma_irqn_acknowledge_channel(V_IRQ_INDEX, ch);
                    obj->dmaFinished();
                }
            }
        }
    };

    // ---- static member definitions ----

    template <uint V_IRQ_INDEX>
    RpPioDmaState<V_IRQ_INDEX> *RpPioDmaState<V_IRQ_INDEX>::s_table[NUM_DMA_CHANNELS] = {};

    template <uint V_IRQ_INDEX>
    volatile int32_t RpPioDmaState<V_IRQ_INDEX>::s_refCount = 0;

} // namespace npb

#endif // ARDUINO_ARCH_RP2040
