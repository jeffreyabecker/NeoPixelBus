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

#include "IClockDataTransport.h"

namespace npb
{

    struct Esp32I2sClockDataTransportConfig
    {
        uint8_t pin = 0;
        bool invert = false;
        uint8_t busNumber = 0;
        int8_t clockPin = -1;
        uint32_t clockDataBitRateHz = 0;
    };

    class Esp32I2sClockDataTransport : public IClockDataTransport
    {
    public:
        static constexpr size_t DmaBitsPerClockDataBit = 1;

        explicit Esp32I2sClockDataTransport(Esp32I2sClockDataTransportConfig config)
            : _config{config}
        {
        }

        ~Esp32I2sClockDataTransport() override
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
                if (_config.clockPin >= 0)
                {
                    gpio_matrix_out(_config.clockPin, SIG_GPIO_OUT_IDX, false, false);
                    pinMode(_config.clockPin, INPUT);
                }
            }

            if (_i2sBuffer)
            {
                heap_caps_free(_i2sBuffer);
            }
        }

        void begin() override
        {
        }

        void beginTransaction() override
        {
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            ensureInitialised(data.size());
            if (!_i2sBuffer)
            {
                return;
            }

            std::memset(_i2sBuffer, 0, _i2sBufferSize);
            std::memcpy(_i2sBuffer, data.data(), data.size());
            i2sWrite(_config.busNumber);
        }

        void endTransaction() override
        {
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
        Esp32I2sClockDataTransportConfig _config;
        uint8_t *_i2sBuffer{nullptr};
        size_t _i2sBufferSize{0};
        size_t _frameBytes{0};
        bool _initialised{false};
        static constexpr size_t TailSilenceBytes = 16;

        static size_t roundUp4(size_t value)
        {
            return (value + 3) & ~static_cast<size_t>(3);
        }

        static uint16_t bitSendTimeNsFromRate(uint32_t rateHz)
        {
            if (rateHz == 0)
            {
                return 400;
            }

            const uint32_t ns = 1000000000UL / rateHz;
            return static_cast<uint16_t>((ns == 0) ? 1 : ns);
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
            _i2sBufferSize = roundUp4(frameBytes) + TailSilenceBytes;
            _i2sBuffer = static_cast<uint8_t *>(
                heap_caps_malloc(_i2sBufferSize, MALLOC_CAP_DMA));
            if (_i2sBuffer)
            {
                std::memset(_i2sBuffer, 0, _i2sBufferSize);
            }

            size_t dmaBlockCount =
                (_i2sBufferSize + I2S_DMA_MAX_DATA_LEN - 1) / I2S_DMA_MAX_DATA_LEN;

            uint16_t bitSendTimeNs = bitSendTimeNsFromRate(_config.clockDataBitRateHz);

            i2sInit(_config.busNumber,
                    false,
                    2,
                    DmaBitsPerClockDataBit,
                    bitSendTimeNs,
                    I2S_CHAN_STEREO,
                    I2S_FIFO_16BIT_DUAL,
                    dmaBlockCount,
                    _i2sBuffer,
                    _i2sBufferSize);

            if (_config.clockPin >= 0)
            {
                i2sSetClockDataBus(_config.busNumber,
                                   _config.clockPin,
                                   false,
                                   _config.pin,
                                   _config.invert);
            }
            else
            {
                i2sSetPins(_config.busNumber,
                           _config.pin,
                           -1,
                           -1,
                           _config.invert);
            }

            _initialised = true;
        }

    };

} // namespace npb

#endif // ARDUINO_ARCH_ESP32 && !ESP32S3 && !ESP32C3
