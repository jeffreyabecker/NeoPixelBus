#pragma once

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_IDF_TARGET_ESP32S3)

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>

#include <Arduino.h>
#include "esp_heap_caps.h"
#include "esp_rom_gpio.h"
#include "driver/periph_ctrl.h"
#include "hal/gpio_hal.h"
#include "soc/lcd_cam_struct.h"
#include "soc/lcd_cam_reg.h"
#include "esp_private/gdma.h"

#include "../ISelfClockingTransport.h"
#include "../SelfClockingTransportConfig.h"

namespace npb
{

    struct Esp32LcdParallelSelfClockingTransportConfig : SelfClockingTransportConfig
    {
    };

    class Esp32LcdParallelContext
    {
    public:
        static constexpr size_t MaxChannels = 8;
        static constexpr size_t DmaBitsPerPixelBit = 3;
        static constexpr size_t DmaBytesPerPixelByte = 8 * DmaBitsPerPixelBit;

        uint8_t registerChannel(size_t channelDataSize)
        {
            uint8_t id = 0;
            while (id < MaxChannels && (_registeredMask & (1u << id)))
            {
                ++id;
            }
            _registeredMask |= (1u << id);

            if (channelDataSize > _maxDataSize)
            {
                _maxDataSize = channelDataSize;
            }
            return id;
        }

        void unregisterChannel(uint8_t muxId, uint8_t pin)
        {
            _registeredMask &= ~(1u << muxId);

            gpio_matrix_out(pin, SIG_GPIO_OUT_IDX, false, false);
            pinMode(pin, INPUT);

            if (_registeredMask == 0 && _initialised)
            {
                waitForDone();
                teardown();
            }
        }

        void initialize(uint16_t bitSendTimeNs,
                        uint8_t pin, uint8_t muxId, bool invert)
        {
            if (!_initialised)
            {
                allocateBuffers();
                initPeripheral(bitSendTimeNs);
                initGdma();
                _initialised = true;
            }

            uint8_t sigIdx = LCD_DATA_OUT0_IDX + muxId;
            esp_rom_gpio_connect_out_signal(pin, sigIdx, invert, false);
            gpio_hal_iomux_func_sel(GPIO_PIN_MUX_REG[pin], PIN_FUNC_GPIO);
            gpio_set_drive_capability(static_cast<gpio_num_t>(pin),
                                      static_cast<gpio_drive_cap_t>(3));
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

        void startWrite()
        {
            _updatedMask = 0;

            gdma_reset(_dmaChannel);

            LCD_CAM.lcd_user.lcd_dout = 1;
            LCD_CAM.lcd_user.lcd_update = 1;
            LCD_CAM.lcd_misc.lcd_afifo_reset = 1;

            gdma_start(_dmaChannel, reinterpret_cast<intptr_t>(&_dmaDescriptors[0]));

            esp_rom_delay_us(1);
            LCD_CAM.lcd_user.lcd_start = 1;
        }

        bool isWriteDone() const
        {
            return !LCD_CAM.lcd_user.lcd_start;
        }

    private:
        struct DmaDescriptor
        {
            struct
            {
                uint32_t size     : 12;
                uint32_t length   : 12;
                uint32_t rsvd_24  : 4;
                uint32_t err_eof  : 1;
                uint32_t rsvd_29  : 1;
                uint32_t suc_eof  : 1;
                uint32_t owner    : 1;
            } dw0;
            void *buffer;
            DmaDescriptor *next;
        };

        uint8_t *_dmaBuffer{nullptr};
        size_t _dmaBufferSize{0};
        size_t _maxDataSize{0};
        uint8_t _registeredMask{0};
        uint8_t _updatedMask{0};
        bool _initialised{false};

        gdma_channel_handle_t _dmaChannel{nullptr};
        DmaDescriptor *_dmaDescriptors{nullptr};
        size_t _dmaDescriptorCount{0};

        static size_t roundUp4(size_t value)
        {
            return (value + 3) & ~static_cast<size_t>(3);
        }

        void allocateBuffers()
        {
            _dmaBufferSize = roundUp4(DmaBytesPerPixelByte * (_maxDataSize + 1));
            _dmaBuffer = static_cast<uint8_t *>(
                heap_caps_malloc(_dmaBufferSize, MALLOC_CAP_DMA));
            if (_dmaBuffer)
            {
                std::memset(_dmaBuffer, 0, _dmaBufferSize);
            }

            static constexpr size_t MaxDescLen = 4092;
            _dmaDescriptorCount =
                (_dmaBufferSize + MaxDescLen - 1) / MaxDescLen + 1;

            _dmaDescriptors = static_cast<DmaDescriptor *>(
                heap_caps_calloc(_dmaDescriptorCount, sizeof(DmaDescriptor), MALLOC_CAP_DMA));

            size_t remaining = _dmaBufferSize;
            uint8_t *pBuf = _dmaBuffer;

            for (size_t i = 0; i < _dmaDescriptorCount - 1; ++i)
            {
                size_t blockLen = (remaining > MaxDescLen) ? MaxDescLen : remaining;
                _dmaDescriptors[i].dw0.size = static_cast<uint32_t>(blockLen);
                _dmaDescriptors[i].dw0.length = static_cast<uint32_t>(blockLen);
                _dmaDescriptors[i].dw0.suc_eof = 0;
                _dmaDescriptors[i].dw0.owner = 1;
                _dmaDescriptors[i].buffer = pBuf;
                _dmaDescriptors[i].next = &_dmaDescriptors[i + 1];
                pBuf += blockLen;
                remaining -= blockLen;
            }

            auto &sentinel = _dmaDescriptors[_dmaDescriptorCount - 1];
            sentinel.dw0.size = 0;
            sentinel.dw0.length = 0;
            sentinel.dw0.suc_eof = 1;
            sentinel.dw0.owner = 1;
            sentinel.buffer = nullptr;
            sentinel.next = nullptr;
        }

        void initPeripheral(uint16_t bitSendTimeNs)
        {
            periph_module_enable(PERIPH_LCD_CAM_MODULE);
            periph_module_reset(PERIPH_LCD_CAM_MODULE);

            LCD_CAM.lcd_user.lcd_reset = 1;
            LCD_CAM.lcd_clock.lcd_clk_sel = 2;

            double divider = static_cast<double>(bitSendTimeNs) / 1000.0 * 240.0;
            if (divider < 2.0)
            {
                divider = 2.0;
            }
            uint32_t intDiv = static_cast<uint32_t>(divider);
            double frac = divider - intDiv;

            LCD_CAM.lcd_clock.lcd_clkm_div_num = intDiv;

            if (frac < 0.01)
            {
                LCD_CAM.lcd_clock.lcd_clkm_div_a = 0;
                LCD_CAM.lcd_clock.lcd_clkm_div_b = 0;
            }
            else
            {
                static constexpr uint32_t FracDenominator = 63;
                LCD_CAM.lcd_clock.lcd_clkm_div_a = FracDenominator;
                LCD_CAM.lcd_clock.lcd_clkm_div_b =
                    static_cast<uint32_t>(frac * FracDenominator + 0.5);
            }

            LCD_CAM.lcd_clock.lcd_ck_out_edge = 0;
            LCD_CAM.lcd_clock.lcd_ck_idle_edge = 0;
            LCD_CAM.lcd_clock.lcd_clk_equ_sysclk = 1;

            LCD_CAM.lcd_ctrl.lcd_rgb_mode_en = 0;
            LCD_CAM.lcd_rgb_yuv.lcd_conv_bypass = 0;
            LCD_CAM.lcd_misc.lcd_next_frame_en = 0;
            LCD_CAM.lcd_data_dout_mode.val = 0;
            LCD_CAM.lcd_user.lcd_always_out_en = 1;
            LCD_CAM.lcd_user.lcd_8bits_order = 0;
            LCD_CAM.lcd_user.lcd_bit_order = 0;
            LCD_CAM.lcd_user.lcd_2byte_en = 0;
            LCD_CAM.lcd_user.lcd_dummy = 1;
            LCD_CAM.lcd_user.lcd_dummy_cyclelen = 0;
            LCD_CAM.lcd_user.lcd_cmd = 0;
        }

        void initGdma()
        {
            gdma_channel_alloc_config_t allocCfg = {};
            allocCfg.direction = GDMA_CHANNEL_DIRECTION_TX;
            allocCfg.flags.reserve_sibling = 0;
            gdma_new_channel(&allocCfg, &_dmaChannel);
            gdma_connect(_dmaChannel, GDMA_MAKE_TRIGGER(GDMA_TRIG_PERIPH_LCD, 0));

            gdma_strategy_config_t stratCfg = {};
            stratCfg.auto_update_desc = false;
            stratCfg.owner_check = false;
            gdma_apply_strategy(_dmaChannel, &stratCfg);

            gdma_tx_event_callbacks_t callbacks = {};
            callbacks.on_trans_eof = dmaEofCallback;
            gdma_register_tx_event_callbacks(_dmaChannel, &callbacks, nullptr);
        }

        void waitForDone()
        {
            while (LCD_CAM.lcd_user.lcd_start)
            {
                yield();
            }
        }

        void teardown()
        {
            if (_dmaChannel)
            {
                gdma_disconnect(_dmaChannel);
                gdma_del_channel(_dmaChannel);
                _dmaChannel = nullptr;
            }

            periph_module_disable(PERIPH_LCD_CAM_MODULE);

            if (_dmaBuffer)
            {
                heap_caps_free(_dmaBuffer);
                _dmaBuffer = nullptr;
            }
            if (_dmaDescriptors)
            {
                heap_caps_free(_dmaDescriptors);
                _dmaDescriptors = nullptr;
            }
            _initialised = false;
        }

        static IRAM_ATTR bool dmaEofCallback(gdma_channel_handle_t,
                                             gdma_event_data_t *,
                                             void *)
        {
            LCD_CAM.lcd_user.lcd_start = 0;
            return true;
        }
    };

    class Esp32LcdParallelSelfClockingTransport : public ISelfClockingTransport
    {
    public:
        explicit Esp32LcdParallelSelfClockingTransport(Esp32LcdParallelSelfClockingTransportConfig config)
            : _config{config}
        {
        }

        ~Esp32LcdParallelSelfClockingTransport() override
        {
            if (_registered)
            {
                SharedContext.unregisterChannel(_muxId, _config.pin);
            }
        }

        void begin() override
        {
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            ensureChannelReady(data.size());

            SharedContext.clearIfNeeded();
            SharedContext.encodeChannel(data.data(), data.size(), _muxId);

            if (SharedContext.allChannelsUpdated())
            {
                SharedContext.startWrite();
            }
        }

        bool isReadyToUpdate() const override
        {
            return SharedContext.isWriteDone();
        }

    private:
        Esp32LcdParallelSelfClockingTransportConfig _config;
        uint8_t _muxId{0};
        bool _registered{false};

        void ensureChannelReady(size_t frameBytes)
        {
            if (!_registered)
            {
                _muxId = SharedContext.registerChannel(frameBytes);
                _registered = true;

                SharedContext.initialize(
                    static_cast<uint16_t>(_config.timing.bitPeriodNs()),
                    _config.pin,
                    _muxId,
                    _config.invert);
            }
        }

        static Esp32LcdParallelContext SharedContext;
    };

    inline Esp32LcdParallelContext Esp32LcdParallelSelfClockingTransport::SharedContext{};

} // namespace npb

#endif // ARDUINO_ARCH_ESP32 && CONFIG_IDF_TARGET_ESP32S3
