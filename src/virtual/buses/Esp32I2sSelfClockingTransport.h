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
    #include "../../original/internal/methods/platform/esp32/Esp32_i2s.h"
}

#include "ISelfClockingTransport.h"
#include "SelfClockingTransportConfig.h"

namespace npb
{

    struct Esp32I2sSelfClockingTransportConfig : SelfClockingTransportConfig
    {
        uint8_t busNumber = 0;
    };

    class Esp32I2sSelfClockingTransport : public ISelfClockingTransport
    {
    public:
        static constexpr size_t DmaBitsPerPixelBit = 3;

        explicit Esp32I2sSelfClockingTransport(Esp32I2sSelfClockingTransportConfig config)
            : _config{config}
        {
        }

        ~Esp32I2sSelfClockingTransport() override
        {
            if (_initialised)
            {
                while (!isReadyToUpdate())
                {
                    yield();
                }
                i2sDeinit(_config.busNumber);
                gpio_matrix_out(_config.pin, SIG_GPIO_OUT_IDX, false, false);
                pinMode(_config.pin, INPUT);
            }

            if (_i2sBuffer)
            {
                heap_caps_free(_i2sBuffer);
            }
        }

        void begin() override
        {
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            ensureInitialised(data.size());
            if (!_i2sBuffer)
            {
                return;
            }

            encode3Step(_i2sBuffer, data.data(), data.size());
            i2sWrite(_config.busNumber);
        }

        bool isReadyToUpdate() const override
        {
            if (!_initialised)
            {
                return true;
            }
            return i2sWriteDone(_config.busNumber);
        }

    private:
        Esp32I2sSelfClockingTransportConfig _config;
        uint8_t *_i2sBuffer{nullptr};
        size_t _i2sBufferSize{0};
        size_t _frameBytes{0};
        bool _initialised{false};

        static size_t roundUp4(size_t value)
        {
            return (value + 3) & ~static_cast<size_t>(3);
        }

        size_t resetByteCount() const
        {
            float byteSendTimeUs =
                static_cast<float>(_config.timing.bitPeriodNs())
                * DmaBitsPerPixelBit * 8.0f / 1000.0f;
            size_t resetBytes = static_cast<size_t>(
                static_cast<float>(_config.timing.resetUs) / byteSendTimeUs + 1.0f);
            return roundUp4(resetBytes);
        }

        void ensureInitialised(size_t frameBytes)
        {
            if (_initialised && _frameBytes == frameBytes)
            {
                return;
            }

            if (_initialised)
            {
                while (!isReadyToUpdate())
                {
                    yield();
                }
                i2sDeinit(_config.busNumber);
                _initialised = false;
            }

            if (_i2sBuffer)
            {
                heap_caps_free(_i2sBuffer);
                _i2sBuffer = nullptr;
            }

            _frameBytes = frameBytes;
            size_t dmaPixelSize = DmaBitsPerPixelBit * frameBytes;
            _i2sBufferSize = roundUp4(dmaPixelSize) + resetByteCount();
            _i2sBuffer = static_cast<uint8_t *>(
                heap_caps_malloc(_i2sBufferSize, MALLOC_CAP_DMA));
            if (_i2sBuffer)
            {
                std::memset(_i2sBuffer, 0, _i2sBufferSize);
            }

            size_t dmaBlockCount =
                (_i2sBufferSize + I2S_DMA_MAX_DATA_LEN - 1) / I2S_DMA_MAX_DATA_LEN;

            uint16_t bitSendTimeNs =
                static_cast<uint16_t>(_config.timing.bitPeriodNs());

            i2sInit(_config.busNumber,
                    false,
                    2,
                    DmaBitsPerPixelBit,
                    bitSendTimeNs,
                    I2S_CHAN_STEREO,
                    I2S_FIFO_16BIT_DUAL,
                    dmaBlockCount,
                    _i2sBuffer,
                    _i2sBufferSize);

            i2sSetPins(_config.busNumber,
                       _config.pin,
                       -1,
                       -1,
                       _config.invert);

            _initialised = true;
        }

        static void encode3Step(uint8_t *dmaBuffer,
                                const uint8_t *data,
                                size_t sizeData)
        {
            static constexpr uint16_t OneBit  = 0b00000110;
            static constexpr uint16_t ZeroBit = 0b00000100;
            static constexpr uint8_t SrcBitMask = 0x80;
            static constexpr size_t BitsInSample = 16;

            uint16_t *pDma = reinterpret_cast<uint16_t *>(dmaBuffer);
            uint16_t dmaValue = 0;
            uint8_t destBitsLeft = BitsInSample;

            const uint8_t *pEnd = data + sizeData;
            for (const uint8_t *pSrc = data; pSrc < pEnd; ++pSrc)
            {
                uint8_t value = *pSrc;
                for (uint8_t bitSrc = 0; bitSrc < 8; ++bitSrc)
                {
                    uint16_t bit = (value & SrcBitMask) ? OneBit : ZeroBit;
                    value <<= 1;

                    if (destBitsLeft > 3)
                    {
                        destBitsLeft -= 3;
                        dmaValue |= bit << destBitsLeft;
                    }
                    else
                    {
                        uint8_t bitSplit = 3 - destBitsLeft;
                        dmaValue |= bit >> bitSplit;
                        *(pDma++) = dmaValue;
                        dmaValue = 0;
                        destBitsLeft = BitsInSample - bitSplit;
                        if (bitSplit > 0)
                        {
                            dmaValue = bit << destBitsLeft;
                        }
                    }
                }
            }

            if (destBitsLeft < BitsInSample)
            {
                *pDma = dmaValue;
            }
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_ESP32 && !ESP32S3 && !ESP32C3
