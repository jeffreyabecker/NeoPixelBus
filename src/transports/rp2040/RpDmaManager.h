#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include <cstdint>
#include <cstddef>

#include <Arduino.h>
#include "hardware/dma.h"
#include "hardware/irq.h"

#include "core/Compat.h"

namespace npb
{

    enum class RpDmaManagerState : uint8_t
    {
        Sending,
        DmaCompleted,
        Idle
    };

    template <uint V_IRQ_INDEX = 1>
    class RpDmaManager
    {
    public:
        class ChannelLease
        {
        public:
            ChannelLease() = default;

            ChannelLease(const ChannelLease &) = delete;
            ChannelLease &operator=(const ChannelLease &) = delete;

            ChannelLease(ChannelLease &&other) noexcept
                : _owner{other._owner}
                , _channel{other._channel}
            {
                other._owner = nullptr;
                other._channel = -1;
            }

            ChannelLease &operator=(ChannelLease &&other) noexcept
            {
                if (this == &other)
                {
                    return *this;
                }

                release();

                _owner = other._owner;
                _channel = other._channel;

                other._owner = nullptr;
                other._channel = -1;

                return *this;
            }

            ~ChannelLease()
            {
                release();
            }

            bool isValid() const
            {
                return _owner != nullptr && _channel >= 0;
            }

            uint channel() const
            {
                return static_cast<uint>(_channel);
            }

            void release()
            {
                if (!isValid())
                {
                    return;
                }

                _owner->releaseClaimedChannel(static_cast<uint>(_channel));
                _owner = nullptr;
                _channel = -1;
            }

            void startTransfer(
                span<uint8_t> data,
                volatile void *writeAddress,
                uint dreq,
                bool byteSwap = true,
                bool readIncrement = true,
                bool writeIncrement = false)
            {
                if (!isValid() || data.empty())
                {
                    return;
                }

                const dma_channel_transfer_size transferSize =
                    (data.size() % 4 == 0) ? DMA_SIZE_32 :
                    ((data.size() % 2 == 0) ? DMA_SIZE_16 : DMA_SIZE_8);

                size_t bytesPerTransfer = 1;
                switch (transferSize)
                {
                case DMA_SIZE_32:
                    bytesPerTransfer = 4;
                    break;
                case DMA_SIZE_16:
                    bytesPerTransfer = 2;
                    break;
                case DMA_SIZE_8:
                default:
                    bytesPerTransfer = 1;
                    break;
                }

                const uint transferCount = static_cast<uint>(data.size() / bytesPerTransfer);

                _owner->startTransferOnChannel(
                    static_cast<uint>(_channel),
                    data.data(),
                    writeAddress,
                    transferCount,
                    transferSize,
                    dreq,
                    byteSwap,
                    readIncrement,
                    writeIncrement);
            }

        private:
            friend class RpDmaManager<V_IRQ_INDEX>;

            ChannelLease(RpDmaManager *owner, uint channel)
                : _owner{owner}
                , _channel{static_cast<int>(channel)}
            {
            }

            RpDmaManager *_owner{nullptr};
            int _channel{-1};
        };

        RpDmaManager() = default;

        RpDmaManager(const RpDmaManager &) = delete;
        RpDmaManager &operator=(const RpDmaManager &) = delete;

        ChannelLease requestChannel()
        {
            if (_channel >= 0)
            {
                return ChannelLease{};
            }

            const uint channel = dma_claim_unused_channel(true);
            registerChannel(channel);
            dma_irqn_set_channel_enabled(V_IRQ_INDEX, channel, true);

            _channel = static_cast<int>(channel);
            _state = RpDmaManagerState::Idle;
            return ChannelLease{this, channel};
        }

        bool hasChannel() const
        {
            return _channel >= 0;
        }

        uint channel() const
        {
            return static_cast<uint>(_channel);
        }

        RpDmaManagerState state() const
        {
            return _state;
        }

        bool isSending() const
        {
            return _state == RpDmaManagerState::Sending;
        }

        bool hasDmaCompleted() const
        {
            return _state == RpDmaManagerState::DmaCompleted;
        }

        uint32_t elapsedSinceDmaCompleteUs() const
        {
            if (_state != RpDmaManagerState::DmaCompleted)
            {
                return 0;
            }

            return micros() - _endTime;
        }

        void setIdle()
        {
            _state = RpDmaManagerState::Idle;
        }

        void startTransferOnChannel(
            uint dmaChannel,
            const void *readAddress,
            volatile void *writeAddress,
            uint transferCount,
            dma_channel_transfer_size transferSize,
            uint dreq,
            bool byteSwap,
            bool readIncrement,
            bool writeIncrement)
        {
            if (!hasChannel() || channel() != dmaChannel || readAddress == nullptr || writeAddress == nullptr || transferCount == 0)
            {
                return;
            }

            // Intentionally rebuild channel configuration for every transfer.
            dma_channel_config cfg = dma_channel_get_default_config(dmaChannel);
            channel_config_set_transfer_data_size(&cfg, transferSize);
            channel_config_set_read_increment(&cfg, readIncrement);
            channel_config_set_write_increment(&cfg, writeIncrement);
            channel_config_set_bswap(&cfg, byteSwap);
            channel_config_set_dreq(&cfg, dreq);

            dma_channel_configure(
                dmaChannel,
                &cfg,
                writeAddress,
                nullptr,
                transferCount,
                false);

            _state = RpDmaManagerState::Sending;
            dma_channel_set_read_addr(dmaChannel, readAddress, true);
        }

    private:
        volatile uint32_t _endTime{0};
        volatile RpDmaManagerState _state{RpDmaManagerState::Idle};
        int _channel{-1};

        static RpDmaManager *s_table[NUM_DMA_CHANNELS];
        static volatile int32_t s_refCount;

        static void dmaIrqHandler()
        {
            for (uint ch = 0; ch < NUM_DMA_CHANNELS; ++ch)
            {
                RpDmaManager *obj = s_table[ch];
                if (obj != nullptr && dma_irqn_get_channel_status(V_IRQ_INDEX, ch))
                {
                    dma_irqn_acknowledge_channel(V_IRQ_INDEX, ch);
                    obj->onDmaFinished();
                }
            }
        }

        void onDmaFinished()
        {
            _endTime = micros();
            _state = RpDmaManagerState::DmaCompleted;
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
                irq_add_shared_handler(
                    irqNum,
                    dmaIrqHandler,
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

        void releaseClaimedChannel(uint dmaChannel)
        {
            dma_irqn_set_channel_enabled(V_IRQ_INDEX, dmaChannel, false);
            unregisterChannel(dmaChannel);
            dma_channel_unclaim(dmaChannel);

            _channel = -1;
            _state = RpDmaManagerState::Idle;
            _endTime = 0;
        }
    };

    template <uint V_IRQ_INDEX>
    RpDmaManager<V_IRQ_INDEX> *RpDmaManager<V_IRQ_INDEX>::s_table[NUM_DMA_CHANNELS] = {};

    template <uint V_IRQ_INDEX>
    volatile int32_t RpDmaManager<V_IRQ_INDEX>::s_refCount = 0;

} // namespace npb

#endif // ARDUINO_ARCH_RP2040
