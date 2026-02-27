#pragma once

#if defined(ARDUINO_ARCH_ESP32) && (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 1))

#include <cstdint>
#include <cstddef>
#include <cstring>

#include <Arduino.h>
#include "driver/spi_master.h"
#include "esp_heap_caps.h"

#include "transports/ITransport.h"

namespace npb
{

#ifndef NEOPIXELBUS_ESP32_DMA_SPI_CLOCK_DEFAULT_HZ
#define NEOPIXELBUS_ESP32_DMA_SPI_CLOCK_DEFAULT_HZ 10000000UL
#endif
        static constexpr uint32_t Esp32DmaSpiClockDefaultHz = NEOPIXELBUS_ESP32_DMA_SPI_CLOCK_DEFAULT_HZ;

#ifndef NEOPIXELBUS_ESP32_DMA_SPI_DEFAULT_HOST
#define NEOPIXELBUS_ESP32_DMA_SPI_DEFAULT_HOST SPI2_HOST
#endif
        static constexpr spi_host_device_t Esp32DmaSpiDefaultHost =
            static_cast<spi_host_device_t>(NEOPIXELBUS_ESP32_DMA_SPI_DEFAULT_HOST);

#if defined(SCK)
        static constexpr int8_t Esp32DmaSpiDefaultSckPin = SCK;
#else
        static constexpr int8_t Esp32DmaSpiDefaultSckPin = -1;
#endif

#if defined(MOSI)
        static constexpr int8_t Esp32DmaSpiDefaultDataPin = MOSI;
#else
        static constexpr int8_t Esp32DmaSpiDefaultDataPin = -1;
#endif

        struct Esp32DmaSpiTransportSettings
        {
                bool invert = false;
                uint32_t clockRateHz = Esp32DmaSpiClockDefaultHz;
                BitOrder bitOrder = MSBFIRST;
                uint8_t dataMode = SPI_MODE0;
                int clockPin = Esp32DmaSpiDefaultSckPin;
                int dataPin = Esp32DmaSpiDefaultDataPin;
                spi_host_device_t spiHost = Esp32DmaSpiDefaultHost;
                int8_t ssPin = -1;
        };

        class Esp32DmaSpiTransport : public ITransport
        {
        public:
                using TransportSettingsType = Esp32DmaSpiTransportSettings;
                using TransportCategory = TransportTag;
                explicit Esp32DmaSpiTransport(Esp32DmaSpiTransportSettings config)
                    : _config{config}
                {
                }

                explicit Esp32DmaSpiTransport(uint32_t clockHz = Esp32DmaSpiClockDefaultHz)
                    : _config{.clockRateHz = clockHz}
                {
                }

                explicit Esp32DmaSpiTransport(uint8_t spiBus,
                                              uint32_t clockHz = Esp32DmaSpiClockDefaultHz)
                    : _config{.spiHost = static_cast<spi_host_device_t>(spiBus), .clockRateHz = clockHz}
                {
                }

                ~Esp32DmaSpiTransport() override
                {
                        deinitSpi();
                        if (_dmaTxBuffer)
                        {
                                heap_caps_free(_dmaTxBuffer);
                                _dmaTxBuffer = nullptr;
                        }
                }

                void begin() override
                {
                }

                void beginTransaction() override
                {
                }

                void transmitBytes(span<const uint8_t> data) override
                {
                        if (data.empty())
                        {
                                return;
                        }

                        ensureReadyForWrite(data.size());
                        if (!_spiHandle || !_dmaTxBuffer)
                        {
                                return;
                        }

                        if (!_config.invert)
                        {
                                std::memcpy(_dmaTxBuffer, data.data(), data.size());
                        }
                        else
                        {
                                for (size_t i = 0; i < data.size(); ++i)
                                {
                                        _dmaTxBuffer[i] = static_cast<uint8_t>(~data[i]);
                                }
                        }

                        _spiTransaction = {0};
                        _spiTransaction.length = static_cast<size_t>(data.size()) * 8;
                        _spiTransaction.tx_buffer = _dmaTxBuffer;

                        esp_err_t ret = spi_device_queue_trans(_spiHandle, &_spiTransaction, 0);
                        ESP_ERROR_CHECK(ret);
                        _pendingTransaction = true;
                }

                void endTransaction() override
                {
                }

                bool isReadyToUpdate() const override
                {
                        if (!_spiHandle || !_pendingTransaction)
                        {
                                return true;
                        }

                        spi_transaction_t result;
                        spi_transaction_t *resultPtr = &result;
                        esp_err_t ret = spi_device_get_trans_result(_spiHandle, &resultPtr, 0);
                        if (ret == ESP_OK)
                        {
                                _pendingTransaction = false;
                                return true;
                        }

                        return ret == ESP_ERR_TIMEOUT ? false : true;
                }

        private:
                Esp32DmaSpiTransportSettings _config;
                mutable bool _pendingTransaction{false};
                bool _initialised{false};
                size_t _maxTransferSize{0};
                uint8_t *_dmaTxBuffer{nullptr};
                size_t _dmaTxBufferSize{0};
                spi_device_handle_t _spiHandle{nullptr};
                spi_transaction_t _spiTransaction{0};

                static size_t roundUp4(size_t value)
                {
                        return (value + 3) & ~static_cast<size_t>(3);
                }

                void ensureReadyForWrite(size_t transferBytes)
                {
                        while (!isReadyToUpdate())
                        {
                                yield();
                        }

                        ensureInitialised(transferBytes);
                        ensureTxBuffer(transferBytes);
                }

                void ensureInitialised(size_t transferBytes)
                {
                        if (_initialised && transferBytes <= _maxTransferSize)
                        {
                                return;
                        }

                        deinitSpi();

                        _maxTransferSize = roundUp4(transferBytes);

                        spi_bus_config_t buscfg = {0};
                        buscfg.sclk_io_num = _config.clockPin;
                        buscfg.data0_io_num = _config.dataPin;
                        buscfg.data1_io_num = -1;
                        buscfg.data2_io_num = -1;
                        buscfg.data3_io_num = -1;
                        buscfg.data4_io_num = -1;
                        buscfg.data5_io_num = -1;
                        buscfg.data6_io_num = -1;
                        buscfg.data7_io_num = -1;
                        buscfg.max_transfer_sz = _maxTransferSize;

                        esp_err_t ret = spi_bus_initialize(_config.spiHost, &buscfg, SPI_DMA_CH_AUTO);
                        ESP_ERROR_CHECK(ret);

                        spi_device_interface_config_t devcfg = {0};
                        devcfg.clock_speed_hz = _config.clockRateHz;
                        devcfg.mode = 0;
                        devcfg.spics_io_num = _config.ssPin;
                        devcfg.queue_size = 1;

                        ret = spi_bus_add_device(_config.spiHost, &devcfg, &_spiHandle);
                        ESP_ERROR_CHECK(ret);

                        _initialised = true;
                        _pendingTransaction = false;
                }

                void ensureTxBuffer(size_t transferBytes)
                {
                        size_t required = roundUp4(transferBytes);
                        if (_dmaTxBuffer && required <= _dmaTxBufferSize)
                        {
                                return;
                        }

                        if (_dmaTxBuffer)
                        {
                                heap_caps_free(_dmaTxBuffer);
                                _dmaTxBuffer = nullptr;
                        }

                        _dmaTxBuffer = static_cast<uint8_t *>(heap_caps_malloc(required, MALLOC_CAP_DMA));
                        _dmaTxBufferSize = _dmaTxBuffer ? required : 0;
                }

                void deinitSpi()
                {
                        if (!_initialised)
                        {
                                return;
                        }

                        while (!isReadyToUpdate())
                        {
                                yield();
                        }

                        if (_spiHandle)
                        {
                                esp_err_t ret = spi_bus_remove_device(_spiHandle);
                                ESP_ERROR_CHECK(ret);
                                _spiHandle = nullptr;
                        }

                        esp_err_t ret = spi_bus_free(_config.spiHost);
                        ESP_ERROR_CHECK(ret);

                        _initialised = false;
                        _pendingTransaction = false;
                }
        };

} // namespace npb

#endif // ARDUINO_ARCH_ESP32 && ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 1)


