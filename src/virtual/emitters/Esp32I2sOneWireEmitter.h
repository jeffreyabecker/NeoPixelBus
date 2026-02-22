#pragma once

// ESP32 I2S single-channel one-wire emitter.
// Supported on: ESP32 (original), ESP32-S2.
// NOT supported on: ESP32-C3, ESP32-S3 (different I2S peripheral).
//
// Uses the low-level C driver from Esp32_i2s.h/.c which manages I2S
// peripheral init, DMA descriptors, clock configuration, and continuous
// idle-loop output.

#if defined(ARDUINO_ARCH_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32S3) && !defined(CONFIG_IDF_TARGET_ESP32C3)

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>
#include <memory>
#include <vector>
#include <algorithm>

#include <Arduino.h>
#include "esp_heap_caps.h"

// The original C driver
extern "C"
{
    #include "../../original/internal/methods/platform/esp32/Esp32_i2s.h"
}

#include "IEmitPixels.h"
#include "ColorOrderTransform.h"
#include "OneWireTiming.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"
#include "../colors/Color.h"

namespace npb
{

    /// Construction settings for Esp32I2sOneWireEmitter.
    struct Esp32I2sOneWireEmitterSettings
    {
        uint8_t pin;
        uint8_t busNumber = 0;             // 0 or 1 (bus 1 only on original ESP32)
        OneWireTiming timing = timing::Ws2812x;
        bool invert = false;
        ColorOrderTransformConfig colorConfig;
    };

    /// One-wire NRZ emitter for ESP32 using the I2S peripheral (single channel).
    ///
    /// This emitter encodes pixel data into a DMA-capable I2S buffer using
    /// a 3-step cadence (3 I2S bits per NeoPixel bit).  The I2S peripheral
    /// continuously loops on silence; `update()` patches the DMA chain to
    /// include the encoded pixel data.
    ///
    /// Signal inversion is handled at the GPIO matrix level.
    class Esp32I2sOneWireEmitter : public IEmitPixels
    {
    public:
        Esp32I2sOneWireEmitter(uint16_t pixelCount,
                               ResourceHandle<IShader> shader,
                               Esp32I2sOneWireEmitterSettings settings)
            : _settings{settings}
            , _shader{std::move(shader)}
            , _transform{settings.colorConfig}
            , _pixelCount{pixelCount}
            , _sizeData{_transform.bytesNeeded(pixelCount)}
            , _scratchColors(pixelCount)
        {
            // CPU-side pixel data buffer
            _data = static_cast<uint8_t *>(malloc(_sizeData));
            if (_data)
            {
                std::memset(_data, 0, _sizeData);
            }

            // I2S DMA buffer: 3 bits per pixel bit = 3 bytes per pixel byte,
            // rounded up to 4-byte alignment, plus reset silence bytes.
            size_t dmaPixelSize = DmaBitsPerPixelBit * _sizeData;
            _i2sBufferSize = roundUp4(dmaPixelSize) + _resetByteCount();
            _i2sBuffer = static_cast<uint8_t *>(
                heap_caps_malloc(_i2sBufferSize, MALLOC_CAP_DMA));
            if (_i2sBuffer)
            {
                std::memset(_i2sBuffer, 0, _i2sBufferSize);
            }
        }

        ~Esp32I2sOneWireEmitter()
        {
            if (_initialised)
            {
                while (!isReadyToUpdate())
                {
                    yield();
                }
                i2sDeinit(_settings.busNumber);
                gpio_matrix_out(_settings.pin, SIG_GPIO_OUT_IDX, false, false);
                pinMode(_settings.pin, INPUT);
            }

            free(_data);
            if (_i2sBuffer)
            {
                heap_caps_free(_i2sBuffer);
            }
        }

        Esp32I2sOneWireEmitter(const Esp32I2sOneWireEmitter &) = delete;
        Esp32I2sOneWireEmitter &operator=(const Esp32I2sOneWireEmitter &) = delete;
        Esp32I2sOneWireEmitter(Esp32I2sOneWireEmitter &&) = delete;
        Esp32I2sOneWireEmitter &operator=(Esp32I2sOneWireEmitter &&) = delete;

        void initialize() override
        {
            if (_initialised)
            {
                return;
            }

            size_t dmaBlockCount =
                (_i2sBufferSize + I2S_DMA_MAX_DATA_LEN - 1) / I2S_DMA_MAX_DATA_LEN;

            uint16_t bitSendTimeNs =
                static_cast<uint16_t>(_settings.timing.bitPeriodNs());

            i2sInit(_settings.busNumber,
                    false,           // not parallel
                    2,               // bytes per sample
                    DmaBitsPerPixelBit,
                    bitSendTimeNs,
                    I2S_CHAN_STEREO,
                    I2S_FIFO_16BIT_DUAL,
                    dmaBlockCount,
                    _i2sBuffer,
                    _i2sBufferSize);

            i2sSetPins(_settings.busNumber,
                       _settings.pin,
                       -1,          // not parallel
                       -1,
                       _settings.invert);

            _initialised = true;
        }

        void update(std::span<const Color> colors) override
        {
            while (!isReadyToUpdate())
            {
                yield();
            }

            // Shade
            std::span<const Color> source = colors;
            if (nullptr != _shader)
            {
                std::copy(colors.begin(), colors.end(), _scratchColors.begin());
                _shader->apply(_scratchColors);
                source = _scratchColors;
            }

            // Transform colors → pixel bytes
            _transform.apply(
                std::span<uint8_t>{_data, _sizeData}, source);

            // Encode pixel bytes → I2S DMA buffer (3-step cadence)
            encode3Step(_i2sBuffer, _data, _sizeData);

            // Trigger DMA send
            i2sWrite(_settings.busNumber);
        }

        bool isReadyToUpdate() const override
        {
            return i2sWriteDone(_settings.busNumber);
        }

        bool alwaysUpdate() const override
        {
            return false;
        }

    private:
        static constexpr size_t DmaBitsPerPixelBit = 3;

        Esp32I2sOneWireEmitterSettings _settings;
        ResourceHandle<IShader> _shader;
        ColorOrderTransform _transform;
        uint16_t _pixelCount;
        size_t _sizeData;

        std::vector<Color> _scratchColors;

        uint8_t *_data{nullptr};
        uint8_t *_i2sBuffer{nullptr};
        size_t _i2sBufferSize{0};
        bool _initialised{false};

        static size_t roundUp4(size_t v)
        {
            return (v + 3) & ~static_cast<size_t>(3);
        }

        /// Compute the number of reset silence bytes needed for the latch gap.
        size_t _resetByteCount() const
        {
            // Each byte represents ByteSendTimeUs of silence.
            // ByteSendTimeUs ≈ bitPeriodNs * DmaBitsPerPixelBit * 8 / 1000
            float byteSendTimeUs =
                static_cast<float>(_settings.timing.bitPeriodNs())
                * DmaBitsPerPixelBit * 8.0f / 1000.0f;
            size_t resetBytes = static_cast<size_t>(
                static_cast<float>(_settings.timing.resetUs) / byteSendTimeUs + 1.0f);
            return roundUp4(resetBytes);
        }

        /// 3-step cadence encoder: 3 I2S bits per NeoPixel bit.
        /// Bit 1 → 0b110, Bit 0 → 0b100
        /// Packs 16-bit DMA samples MSB first with bit-spanning.
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

            // Flush remaining bits
            if (destBitsLeft < BitsInSample)
            {
                *pDma = dmaValue;
            }
        }
    };

} // namespace npb

#endif // ESP32 && !S3 && !C3
