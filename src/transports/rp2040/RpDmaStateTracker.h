#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include <cstdint>
#include <cstddef>

#include <Arduino.h>
#include "hardware/dma.h"
#include "hardware/irq.h"

namespace npb
{

    enum class RpDmaState : uint8_t
    {
        Sending,
        DmaCompleted,
        Idle
    };

    template <uint V_IRQ_INDEX = 1>
    class RpDmaStateTracker
    {
    public:
        RpDmaStateTracker() = default;

        void setSending()
        {
            _state = RpDmaState::Sending;
        }

        void dmaFinished()
        {
            _endTime = micros();
            _state = RpDmaState::DmaCompleted;
        }

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
                    const_cast<volatile RpDmaState &>(_state) = RpDmaState::Idle;
                    return true;
                }
                return false;
            }

            default:
                return true;
            }
        }

        void registerChannel(uint dmaChannel)
        {
            if (s_table[dmaChannel] != nullptr)
            {
                return;
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

        static RpDmaStateTracker *s_table[NUM_DMA_CHANNELS];
        static volatile int32_t s_refCount;

        static void dmaIrqHandler()
        {
            for (uint ch = 0; ch < NUM_DMA_CHANNELS; ++ch)
            {
                RpDmaStateTracker *obj = s_table[ch];
                if (obj != nullptr && dma_irqn_get_channel_status(V_IRQ_INDEX, ch))
                {
                    dma_irqn_acknowledge_channel(V_IRQ_INDEX, ch);
                    obj->dmaFinished();
                }
            }
        }
    };

    template <uint V_IRQ_INDEX>
    RpDmaStateTracker<V_IRQ_INDEX> *RpDmaStateTracker<V_IRQ_INDEX>::s_table[NUM_DMA_CHANNELS] = {};

    template <uint V_IRQ_INDEX>
    volatile int32_t RpDmaStateTracker<V_IRQ_INDEX>::s_refCount = 0;

} // namespace npb

#endif // ARDUINO_ARCH_RP2040

