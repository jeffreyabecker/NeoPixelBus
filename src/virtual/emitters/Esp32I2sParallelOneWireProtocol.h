#pragma once

// ESP32 I2S parallel one-wire emitter.
// Supported on: ESP32 (original), ESP32-S2.
// NOT supported on: ESP32-C3, ESP32-S3 (use LCD-CAM parallel instead).
//
// Drives up to 8 strips in parallel using the I2S peripheral in LCD/parallel
// mode.  All strips sharing a bus are encoded into a single DMA buffer where
// each bit position maps to a different GPIO pin.
//
// All instances on the same bus MUST call update() every frame because the
// shared DMA buffer is cleared and re-encoded each cycle.

#if defined(ARDUINO_ARCH_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32S3) && !defined(CONFIG_IDF_TARGET_ESP32C3)

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>
#include <memory>
#include <vector>
#include <algorithm>
#include <atomic>

#include <Arduino.h>
#include "esp_heap_caps.h"

extern "C"
{
    #include "../../original/internal/methods/platform/esp32/Esp32_i2s.h"
}

#include "IProtocol.h"
#include "ColorOrderTransform.h"
#include "OneWireTiming.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"
#include "../colors/Color.h"

namespace npb
{

    /// Construction settings for Esp32I2sParallelOneWireProtocol.
    struct Esp32I2sParallelOneWireProtocolSettings
    {
        uint8_t pin;
        uint8_t busNumber = 1;             // 0 or 1 (bus 1 only on original ESP32)
        OneWireTiming timing = timing::Ws2812x;
        bool invert = false;
        ColorOrderTransformConfig colorConfig;
    };

    /// Shared context for all parallel channels on one I2S bus.
    /// Owns the DMA buffer and tracks which mux channels have updated.
    class Esp32I2sParallelContext
    {
    public:
        static constexpr size_t MaxChannels = 8;
        static constexpr size_t DmaBitsPerPixelBit = 3; // 3-step cadence

        /// Register a new channel.  Returns the assigned mux ID (0–7).
        /// Must be called before initialize().
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
                        true,            // parallel mode
                        1,               // bytes per sample (8-bit parallel)
                        DmaBitsPerPixelBit,
                        bitSendTimeNs,
                        I2S_CHAN_RIGHT_TO_LEFT,
                        I2S_FIFO_16BIT_SINGLE,
                        dmaBlockCount,
                        _dmaBuffer,
                        _dmaBufferSize);

                _initialised = true;
            }

            // Route this channel's pin
            i2sSetPins(busNumber, pin, muxId, 1, invert);
        }

        /// Clear the DMA buffer (first channel to update does this).
        void clearIfNeeded()
        {
            if (_updatedMask == 0 && _dmaBuffer)
            {
                std::memset(_dmaBuffer, 0, _dmaBufferSize);
            }
        }

        /// Encode one channel's data into the shared DMA buffer.
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
                    // 3-step cadence: bit 1 → 110, bit 0 → 100
                    bool isOne = (value & 0x80) != 0;
                    value <<= 1;

                    pDma[0] |= muxBit;                              // always HIGH first step
                    if (isOne)
                    {
                        pDma[1] |= muxBit;                          // HIGH second step for 1-bit
                    }
                    // third step stays LOW (already cleared)

                    pDma += DmaBitsPerPixelBit;
                }
            }

            _updatedMask |= (1u << muxId);
        }

        /// Returns true when all registered channels have updated.
        bool allChannelsUpdated() const
        {
            return (_updatedMask & _registeredMask) == _registeredMask;
        }

        /// Start the DMA write (called when all channels have updated).
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

        static size_t roundUp4(size_t v)
        {
            return (v + 3) & ~static_cast<size_t>(3);
        }
    };

    /// One-wire NRZ emitter for ESP32 I2S parallel output.
    ///
    /// Each instance represents one strip on one mux channel.  All instances
    /// sharing the same bus must call update() every frame (alwaysUpdate
    /// returns true) because the shared DMA buffer is cleared and
    /// re-encoded each cycle.
    class Esp32I2sParallelOneWireProtocol : public IProtocol
    {
    public:
        Esp32I2sParallelOneWireProtocol(uint16_t pixelCount,
                                       ResourceHandle<IShader> shader,
                                       Esp32I2sParallelOneWireProtocolSettings settings)
            : _settings{settings}
            , _shader{std::move(shader)}
            , _transform{settings.colorConfig}
            , _pixelCount{pixelCount}
            , _sizeData{_transform.bytesNeeded(pixelCount)}
            , _scratchColors(pixelCount)
        {
            _data = static_cast<uint8_t *>(malloc(_sizeData));
            if (_data)
            {
                std::memset(_data, 0, _sizeData);
            }

            auto &ctx = context(_settings.busNumber);
            _muxId = ctx.registerChannel(_sizeData);
        }

        ~Esp32I2sParallelOneWireProtocol()
        {
            auto &ctx = context(_settings.busNumber);
            ctx.unregisterChannel(_muxId, _settings.pin, _settings.busNumber);
            free(_data);
        }

        Esp32I2sParallelOneWireProtocol(const Esp32I2sParallelOneWireProtocol &) = delete;
        Esp32I2sParallelOneWireProtocol &operator=(const Esp32I2sParallelOneWireProtocol &) = delete;
        Esp32I2sParallelOneWireProtocol(Esp32I2sParallelOneWireProtocol &&) = delete;
        Esp32I2sParallelOneWireProtocol &operator=(Esp32I2sParallelOneWireProtocol &&) = delete;

        void initialize() override
        {
            if (_initialised)
            {
                return;
            }

            auto &ctx = context(_settings.busNumber);
            ctx.initialize(
                _settings.busNumber,
                static_cast<uint16_t>(_settings.timing.bitPeriodNs()),
                _settings.pin,
                _muxId,
                _settings.invert);

            _initialised = true;
        }

        void update(std::span<const Color> colors) override
        {
            // Wait for prior send
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

            // Transform
            _transform.apply(
                std::span<uint8_t>{_data, _sizeData}, source);

            auto &ctx = context(_settings.busNumber);
            ctx.clearIfNeeded();
            ctx.encodeChannel(_data, _sizeData, _muxId);

            // Start DMA only when all channels have updated
            if (ctx.allChannelsUpdated())
            {
                ctx.startWrite(_settings.busNumber);
            }
        }

        bool isReadyToUpdate() const override
        {
            return context(_settings.busNumber).isWriteDone(_settings.busNumber);
        }

        bool alwaysUpdate() const override
        {
            return true; // all mux channels must update every frame
        }

    private:
        Esp32I2sParallelOneWireProtocolSettings _settings;
        ResourceHandle<IShader> _shader;
        ColorOrderTransform _transform;
        uint16_t _pixelCount;
        size_t _sizeData;

        std::vector<Color> _scratchColors;
        uint8_t *_data{nullptr};
        uint8_t _muxId{0};
        bool _initialised{false};

        // Shared contexts — one per I2S bus
        static Esp32I2sParallelContext &context(uint8_t busNumber)
        {
            static Esp32I2sParallelContext s_ctx[2];
            return s_ctx[busNumber & 1];
        }
    };

} // namespace npb

#endif // ESP32 && !S3 && !C3
