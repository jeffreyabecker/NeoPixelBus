#pragma once

// ESP32-S3 LCD-CAM parallel one-wire emitter.
// Supported ONLY on: ESP32-S3 (CONFIG_IDF_TARGET_ESP32S3).
//
// Uses the LCD-CAM peripheral + GDMA to drive up to 8 strips in parallel.
// All instances share a single static context, single DMA buffer, and must
// call update() every frame (alwaysUpdate() returns true).

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_IDF_TARGET_ESP32S3)

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>
#include <memory>
#include <vector>
#include <algorithm>

#include <Arduino.h>
#include "esp_heap_caps.h"
#include "esp_rom_gpio.h"
#include "driver/periph_ctrl.h"
#include "hal/gpio_hal.h"
#include "soc/lcd_cam_struct.h"
#include "soc/lcd_cam_reg.h"
#include "esp_private/gdma.h"

#include "IProtocol.h"
#include "ColorOrderTransform.h"
#include "OneWireTiming.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"
#include "../colors/Color.h"

namespace npb
{

    /// Construction settings for Esp32LcdParallelOneWireProtocol.
    struct Esp32LcdParallelOneWireProtocolSettings
    {
        uint8_t pin;
        OneWireTiming timing = timing::Ws2812x;
        bool invert = false;
        ColorOrderTransformConfig colorConfig;
    };

    /// Shared context for all channels on the LCD-CAM bus.
    /// Owns the DMA buffer and GDMA channel.
    class Esp32LcdParallelContext
    {
    public:
        static constexpr size_t MaxChannels = 8;
        static constexpr size_t DmaBitsPerPixelBit = 3; // 3-step cadence
        static constexpr size_t DmaBytesPerPixelByte = 8 * DmaBitsPerPixelBit; // 24

        // ---- Channel management -----------------------------------------

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

            // Disconnect pin
            gpio_matrix_out(pin, SIG_GPIO_OUT_IDX, false, false);
            pinMode(pin, INPUT);

            if (_registeredMask == 0 && _initialised)
            {
                waitForDone();
                teardown();
            }
        }

        // ---- Initialisation --------------------------------------------

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

            // Route this channel's pin to the LCD data output
            uint8_t sigIdx = LCD_DATA_OUT0_IDX + muxId;
            esp_rom_gpio_connect_out_signal(pin, sigIdx, invert, false);
            gpio_hal_iomux_func_sel(GPIO_PIN_MUX_REG[pin], PIN_FUNC_GPIO);
            gpio_set_drive_capability(static_cast<gpio_num_t>(pin),
                                      static_cast<gpio_drive_cap_t>(3));
        }

        // ---- Frame encoding / sending ----------------------------------

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

                    pDma[0] |= muxBit;             // step 0: always HIGH
                    if (isOne)
                    {
                        pDma[1] |= muxBit;         // step 1: HIGH for 1-bit
                    }
                    // step 2: stays LOW

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
        uint8_t *_dmaBuffer{nullptr};
        size_t _dmaBufferSize{0};
        size_t _maxDataSize{0};
        uint8_t _registeredMask{0};
        uint8_t _updatedMask{0};
        bool _initialised{false};

        gdma_channel_handle_t _dmaChannel{nullptr};

        // DMA descriptor chain (allocated dynamically)
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

        DmaDescriptor *_dmaDescriptors{nullptr};
        size_t _dmaDescriptorCount{0};

        // ------------------------------------------------------------------

        static size_t roundUp4(size_t v)
        {
            return (v + 3) & ~static_cast<size_t>(3);
        }

        void allocateBuffers()
        {
            // Leave one pixel-byte worth of silence at the end for reset latch
            _dmaBufferSize = roundUp4(DmaBytesPerPixelByte * (_maxDataSize + 1));
            _dmaBuffer = static_cast<uint8_t *>(
                heap_caps_malloc(_dmaBufferSize, MALLOC_CAP_DMA));
            if (_dmaBuffer)
            {
                std::memset(_dmaBuffer, 0, _dmaBufferSize);
            }

            // Build DMA descriptor chain
            static constexpr size_t MaxDescLen = 4092; // DMA_DESCRIPTOR_BUFFER_MAX_SIZE - 4
            _dmaDescriptorCount =
                (_dmaBufferSize + MaxDescLen - 1) / MaxDescLen + 1; // +1 sentinel

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

            // Sentinel EOF descriptor
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

            // Clock: PLL 240 MHz
            LCD_CAM.lcd_clock.lcd_clk_sel = 2;

            double divider = static_cast<double>(bitSendTimeNs) / 1000.0 * 240.0;
            if (divider < 2.0)
            {
                divider = 2.0;
            }
            uint32_t intDiv = static_cast<uint32_t>(divider);
            double frac = divider - intDiv;

            LCD_CAM.lcd_clock.lcd_clkm_div_num = intDiv;

            // Simple fractional approximation
            if (frac < 0.01)
            {
                LCD_CAM.lcd_clock.lcd_clkm_div_a = 0;
                LCD_CAM.lcd_clock.lcd_clkm_div_b = 0;
            }
            else
            {
                static constexpr uint32_t FracDenominator = 63; // max 6-bit
                LCD_CAM.lcd_clock.lcd_clkm_div_a = FracDenominator;
                LCD_CAM.lcd_clock.lcd_clkm_div_b =
                    static_cast<uint32_t>(frac * FracDenominator + 0.5);
            }

            LCD_CAM.lcd_clock.lcd_ck_out_edge = 0;
            LCD_CAM.lcd_clock.lcd_ck_idle_edge = 0;
            LCD_CAM.lcd_clock.lcd_clk_equ_sysclk = 1;

            // LCD config
            LCD_CAM.lcd_ctrl.lcd_rgb_mode_en = 0;   // i8080 mode
            LCD_CAM.lcd_rgb_yuv.lcd_conv_bypass = 0;
            LCD_CAM.lcd_misc.lcd_next_frame_en = 0;
            LCD_CAM.lcd_data_dout_mode.val = 0;      // no data delays
            LCD_CAM.lcd_user.lcd_always_out_en = 1;
            LCD_CAM.lcd_user.lcd_8bits_order = 0;
            LCD_CAM.lcd_user.lcd_bit_order = 0;
            LCD_CAM.lcd_user.lcd_2byte_en = 0;       // 8-bit bus
            LCD_CAM.lcd_user.lcd_dummy = 1;
            LCD_CAM.lcd_user.lcd_dummy_cyclelen = 0;  // 1 dummy cycle
            LCD_CAM.lcd_user.lcd_cmd = 0;             // no command phase
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

            // Register EOF callback to clear lcd_start
            gdma_tx_event_callbacks_t cbs = {};
            cbs.on_trans_eof = dmaEofCallback;
            gdma_register_tx_event_callbacks(_dmaChannel, &cbs, nullptr);
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

    /// One-wire NRZ emitter using ESP32-S3 LCD-CAM peripheral in parallel.
    ///
    /// All instances share a single DMA buffer and must update every frame.
    class Esp32LcdParallelOneWireProtocol : public IProtocol
    {
    public:
        Esp32LcdParallelOneWireProtocol(uint16_t pixelCount,
                                       ResourceHandle<IShader> shader,
                                       Esp32LcdParallelOneWireProtocolSettings settings)
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
            _muxId = s_context.registerChannel(_sizeData);
        }

        ~Esp32LcdParallelOneWireProtocol()
        {
            s_context.unregisterChannel(_muxId, _settings.pin);
            free(_data);
        }

        Esp32LcdParallelOneWireProtocol(const Esp32LcdParallelOneWireProtocol &) = delete;
        Esp32LcdParallelOneWireProtocol &operator=(const Esp32LcdParallelOneWireProtocol &) = delete;
        Esp32LcdParallelOneWireProtocol(Esp32LcdParallelOneWireProtocol &&) = delete;
        Esp32LcdParallelOneWireProtocol &operator=(Esp32LcdParallelOneWireProtocol &&) = delete;

        void initialize() override
        {
            if (_initialised)
            {
                return;
            }
            s_context.initialize(
                static_cast<uint16_t>(_settings.timing.bitPeriodNs()),
                _settings.pin,
                _muxId,
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

            // Transform
            _transform.apply(
                std::span<uint8_t>{_data, _sizeData}, source);

            s_context.clearIfNeeded();
            s_context.encodeChannel(_data, _sizeData, _muxId);

            if (s_context.allChannelsUpdated())
            {
                s_context.startWrite();
            }
        }

        bool isReadyToUpdate() const override
        {
            return s_context.isWriteDone();
        }

        bool alwaysUpdate() const override
        {
            return true;
        }

    private:
        Esp32LcdParallelOneWireProtocolSettings _settings;
        ResourceHandle<IShader> _shader;
        ColorOrderTransform _transform;
        uint16_t _pixelCount;
        size_t _sizeData;

        std::vector<Color> _scratchColors;
        uint8_t *_data{nullptr};
        uint8_t _muxId{0};
        bool _initialised{false};

        static Esp32LcdParallelContext s_context;
    };

    // Static context — shared across all instances
    inline Esp32LcdParallelContext Esp32LcdParallelOneWireProtocol::s_context{};

} // namespace npb

#endif // ESP32 && ESP32S3
