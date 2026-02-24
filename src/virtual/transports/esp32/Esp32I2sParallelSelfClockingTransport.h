#pragma once

#if defined(ARDUINO_ARCH_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32S3) && !defined(CONFIG_IDF_TARGET_ESP32C3)

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>

#include <Arduino.h>
#include "esp_heap_caps.h"

extern "C"
{
    #include "../../../original/internal/methods/platform/esp32/Esp32_i2s.h"
}

#include "../ISelfClockingTransport.h"
#include "../SelfClockingTransportConfig.h"

namespace npb
{

    struct Esp32I2sParallelSelfClockingTransportConfig : SelfClockingTransportConfig
    {
        uint8_t busNumber = 1;
    };

    class Esp32I2sParallelContext
    {
    public:
        static constexpr size_t MaxChannels = 8;
        static constexpr size_t DmaBitsPerPixelBit = 3;

        uint8_t registerChannel(size_t channelDataSize)
        {
            uint8_t id = _nextMuxId++;
            if (channelDataSize > _maxDataSize)
            {
                _maxDataSize = channelDataSize;
            }
            _registeredMask |= (1u << id);
            return id;
        }

        void unregisterChannel(uint8_t muxId, uint8_t pin, uint8_t busNumber)
        {
            _registeredMask &= ~(1u << muxId);

            gpio_matrix_out(pin, SIG_GPIO_OUT_IDX, false, false);
            pinMode(pin, INPUT);

            if (_registeredMask == 0 && _initialised)
            {
                while (!i2sWriteDone(busNumber))
                {
                    yield();
                }
                i2sDeinit(busNumber);
                if (_dmaBuffer)
                {
                    heap_caps_free(_dmaBuffer);
                    _dmaBuffer = nullptr;
                }
                _initialised = false;
            }
        }

        void initialize(uint8_t busNumber, uint16_t bitSendTimeNs,
                        uint8_t pin, uint8_t muxId, bool invert)
        {
            if (!_initialised)
            {
                _dmaBufferSize = roundUp4(
                    MaxChannels * DmaBitsPerPixelBit * _maxDataSize);
                _dmaBuffer = static_cast<uint8_t *>(
                    heap_caps_malloc(_dmaBufferSize, MALLOC_CAP_DMA));
                if (_dmaBuffer)
                {
                    std::memset(_dmaBuffer, 0, _dmaBufferSize);
                }

                size_t dmaBlockCount =
                    (_dmaBufferSize + I2S_DMA_MAX_DATA_LEN - 1) / I2S_DMA_MAX_DATA_LEN;

                i2sInit(busNumber,
                        true,
                        1,
                        DmaBitsPerPixelBit,
                        bitSendTimeNs,
                        I2S_CHAN_RIGHT_TO_LEFT,
                        I2S_FIFO_16BIT_SINGLE,
                        dmaBlockCount,
                        _dmaBuffer,
                        _dmaBufferSize);

                _initialised = true;
            }

            i2sSetPins(busNumber, pin, muxId, 1, invert);
        }

        void clearIfNeeded()
        {
            if (_updatedMask == 0 && _dmaBuffer)
            {
                std::memset(_dmaBuffer, 0, _dmaBufferSize);
            }
        }

        void encodeChannel(const uint8_t *data, size_t sizeData, uint8_t muxId)
        {
            if (!_dmaBuffer)
            {
                return;
            }

            uint8_t muxBit = (1u << muxId);
            uint8_t *pDma = _dmaBuffer;

            for (size_t i = 0; i < sizeData; ++i)
            {
                uint8_t value = data[i];
                for (uint8_t bit = 0; bit < 8; ++bit)
                {
                    bool isOne = (value & 0x80) != 0;
                    value <<= 1;

                    pDma[0] |= muxBit;
                    if (isOne)
                    {
                        pDma[1] |= muxBit;
                    }

                    pDma += DmaBitsPerPixelBit;
                }
            }

            _updatedMask |= (1u << muxId);
        }

        bool allChannelsUpdated() const
        {
            return (_updatedMask & _registeredMask) == _registeredMask;
        }

        void startWrite(uint8_t busNumber)
        {
            _updatedMask = 0;
            i2sWrite(busNumber);
        }

        bool isWriteDone(uint8_t busNumber) const
        {
            return i2sWriteDone(busNumber);
        }

    private:
        uint8_t *_dmaBuffer{nullptr};
        size_t _dmaBufferSize{0};
        size_t _maxDataSize{0};
        uint8_t _nextMuxId{0};
        uint8_t _registeredMask{0};
        uint8_t _updatedMask{0};
        bool _initialised{false};

        static size_t roundUp4(size_t value)
        {
            return (value + 3) & ~static_cast<size_t>(3);
        }
    };

    class Esp32I2sParallelSelfClockingTransport : public ISelfClockingTransport
    {
    public:
        explicit Esp32I2sParallelSelfClockingTransport(Esp32I2sParallelSelfClockingTransportConfig config)
            : _config{config}
        {
        }

        ~Esp32I2sParallelSelfClockingTransport() override
        {
            if (_registered)
            {
                context(_config.busNumber).unregisterChannel(_muxId, _config.pin, _config.busNumber);
            }
        }

        void begin() override
        {
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            ensureChannelReady(data.size());

            auto &ctx = context(_config.busNumber);
            ctx.clearIfNeeded();
            ctx.encodeChannel(data.data(), data.size(), _muxId);

            if (ctx.allChannelsUpdated())
            {
                ctx.startWrite(_config.busNumber);
            }
        }

        bool isReadyToUpdate() const override
        {
            return context(_config.busNumber).isWriteDone(_config.busNumber);
        }

    private:
        Esp32I2sParallelSelfClockingTransportConfig _config;
        uint8_t _muxId{0};
        bool _registered{false};

        void ensureChannelReady(size_t frameBytes)
        {
            auto &ctx = context(_config.busNumber);

            if (!_registered)
            {
                _muxId = ctx.registerChannel(frameBytes);
                _registered = true;

                ctx.initialize(
                    _config.busNumber,
                    static_cast<uint16_t>(_config.timing.bitPeriodNs()),
                    _config.pin,
                    _muxId,
                    _config.invert);
            }
        }

        static Esp32I2sParallelContext &context(uint8_t busNumber)
        {
            static Esp32I2sParallelContext SharedContext[2];
            return SharedContext[busNumber & 1];
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_ESP32 && !ESP32S3 && !ESP32C3
