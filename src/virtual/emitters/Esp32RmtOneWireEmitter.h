#pragma once

#if defined(ARDUINO_ARCH_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32C6) && !defined(CONFIG_IDF_TARGET_ESP32H2)

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>
#include <memory>
#include <vector>
#include <algorithm>
#include <utility>

#include <Arduino.h>
#include "driver/rmt.h"
#include "soc/rmt_struct.h"

#include "IEmitPixels.h"
#include "ColorOrderTransform.h"
#include "OneWireTiming.h"
#include "../shaders/IShader.h"
#include "../ResourceHandle.h"
#include "../colors/Color.h"

namespace npb
{

    /// Construction settings for Esp32RmtOneWireEmitter.
    struct Esp32RmtOneWireEmitterSettings
    {
        uint8_t pin;
        rmt_channel_t channel = RMT_CHANNEL_0;
        OneWireTiming timing = timing::Ws2812x;
        bool invert = false;
        ColorOrderTransformConfig colorConfig;
    };

    /// One-wire NRZ emitter for ESP32 using the RMT peripheral.
    ///
    /// Each instance uses one RMT channel. The RMT translator callback
    /// converts pixel bytes to RMT items on the fly, avoiding a large
    /// pre-encoded buffer.
    ///
    /// Signal inversion swaps the RMT item polarity and idle level.
    class Esp32RmtOneWireEmitter : public IEmitPixels
    {
    public:
        Esp32RmtOneWireEmitter(uint16_t pixelCount,
                               ResourceHandle<IShader> shader,
                               Esp32RmtOneWireEmitterSettings settings)
            : _settings{settings}
            , _shader{std::move(shader)}
            , _transform{settings.colorConfig}
            , _pixelCount{pixelCount}
            , _sizeData{_transform.bytesNeeded(pixelCount)}
            , _scratchColors(pixelCount)
            , _dataEditing(static_cast<uint8_t *>(malloc(_sizeData)))
            , _dataSending(static_cast<uint8_t *>(malloc(_sizeData)))
        {
            if (_dataEditing)
            {
                std::memset(_dataEditing, 0, _sizeData);
            }

            // Pre-compute RMT items from timing
            _computeRmtItems();
        }

        ~Esp32RmtOneWireEmitter()
        {
            if (_initialised)
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(
                    rmt_wait_tx_done(_settings.channel, 10000 / portTICK_PERIOD_MS));
                ESP_ERROR_CHECK(rmt_driver_uninstall(_settings.channel));

                gpio_matrix_out(_settings.pin, SIG_GPIO_OUT_IDX, false, false);
                pinMode(_settings.pin, INPUT);
            }

            free(_dataEditing);
            free(_dataSending);
        }

        Esp32RmtOneWireEmitter(const Esp32RmtOneWireEmitter &) = delete;
        Esp32RmtOneWireEmitter &operator=(const Esp32RmtOneWireEmitter &) = delete;
        Esp32RmtOneWireEmitter(Esp32RmtOneWireEmitter &&) = delete;
        Esp32RmtOneWireEmitter &operator=(Esp32RmtOneWireEmitter &&) = delete;

        void initialize() override
        {
            if (_initialised)
            {
                return;
            }

            rmt_config_t config = RMT_DEFAULT_CONFIG_TX(
                static_cast<gpio_num_t>(_settings.pin),
                _settings.channel);
            config.clk_div = RmtClockDivider;
            config.tx_config.idle_level = _settings.invert
                                              ? RMT_IDLE_LEVEL_HIGH
                                              : RMT_IDLE_LEVEL_LOW;
            config.tx_config.idle_output_en = true;

            ESP_ERROR_CHECK(rmt_config(&config));

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0)
            constexpr uint32_t intFlags = ESP_INTR_FLAG_LOWMED;
#else
            constexpr uint32_t intFlags =
                ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL1;
#endif
            ESP_ERROR_CHECK(rmt_driver_install(_settings.channel, 0, intFlags));
            ESP_ERROR_CHECK(rmt_translator_init(_settings.channel, translateCb));

            _initialised = true;
        }

        void update(std::span<const Color> colors) override
        {
            // Wait for prior send
            while (!isReadyToUpdate())
            {
                yield();
            }

            // Apply shader
            std::span<const Color> source = colors;
            if (nullptr != _shader)
            {
                std::copy(colors.begin(), colors.end(), _scratchColors.begin());
                _shader->apply(_scratchColors);
                source = _scratchColors;
            }

            // Transform
            _transform.apply(
                std::span<uint8_t>{_dataEditing, _sizeData}, source);

            // Send via RMT translator
            // The translator accesses the active items through the static pointer
            s_activeItems = &_rmtItems;
            ESP_ERROR_CHECK(
                rmt_write_sample(_settings.channel, _dataEditing,
                                 _sizeData, false));

            // Maintain buffer consistency
            std::memcpy(_dataSending, _dataEditing, _sizeData);
            std::swap(_dataSending, _dataEditing);
        }

        bool isReadyToUpdate() const override
        {
            return rmt_wait_tx_done(_settings.channel, 0) == ESP_OK;
        }

        bool alwaysUpdate() const override
        {
            return false;
        }

    private:
        static constexpr uint8_t RmtClockDivider = 2;
        // 80 MHz APB / 2 = 40 MHz → 25 ns per tick
        static constexpr float NsPerTick = 25.0f;

        struct RmtItems
        {
            uint32_t bit0;
            uint32_t bit1;
            uint32_t resetDuration; // in ticks
        };

        Esp32RmtOneWireEmitterSettings _settings;
        ResourceHandle<IShader> _shader;
        ColorOrderTransform _transform;
        uint16_t _pixelCount;
        size_t _sizeData;

        std::vector<Color> _scratchColors;

        uint8_t *_dataEditing;
        uint8_t *_dataSending;

        RmtItems _rmtItems{};
        bool _initialised{false};

        // Static pointer to active items for the translator callback.
        // This is set just before each rmt_write_sample call.
        static inline const RmtItems *s_activeItems = nullptr;

        static uint32_t fromNs(uint32_t ns)
        {
            return static_cast<uint32_t>(static_cast<float>(ns) / NsPerTick + 0.5f);
        }

        /// Pack an RMT item as a uint32_t.
        /// Normal polarity: bit15 = 1 (HIGH first half), bit31 = 0 (LOW second half)
        /// Inverted polarity: bit15 = 0 (LOW first half), bit31 = 1 (HIGH second half)
        uint32_t makeItem32(uint32_t highNs, uint32_t lowNs) const
        {
            uint32_t dur0 = fromNs(highNs);
            uint32_t dur1 = fromNs(lowNs);

            if (_settings.invert)
            {
                // Inverted: first phase LOW (level0=0), second phase HIGH (level1=1)
                return (dur1 << 16) | (1u << 31) | dur0;
            }
            else
            {
                // Normal: first phase HIGH (level0=1), second phase LOW (level1=0)
                return (dur1 << 16) | (1u << 15) | dur0;
            }
        }

        void _computeRmtItems()
        {
            _rmtItems.bit0 = makeItem32(_settings.timing.t0hNs, _settings.timing.t0lNs);
            _rmtItems.bit1 = makeItem32(_settings.timing.t1hNs, _settings.timing.t1lNs);
            _rmtItems.resetDuration = fromNs(_settings.timing.resetUs * 1000);
        }

        /// Static IRAM_ATTR translator callback for rmt_write_sample.
        /// Converts source bytes to rmt_item32_t on the fly, MSB first.
        /// On the last byte, extends the final item's duration1 to embed
        /// the reset pulse.
        static void IRAM_ATTR translateCb(
            const void *src,
            rmt_item32_t *dest,
            size_t src_size,
            size_t wanted_num,
            size_t *translated_size,
            size_t *item_num)
        {
            const RmtItems *items = s_activeItems;
            if (items == nullptr || src_size == 0 || wanted_num == 0)
            {
                *translated_size = 0;
                *item_num = 0;
                return;
            }

            const uint8_t *pSrc = static_cast<const uint8_t *>(src);
            size_t srcDone = 0;
            size_t itemsDone = 0;

            while (srcDone < src_size && itemsDone + 8 <= wanted_num)
            {
                uint8_t byte = pSrc[srcDone];

                for (int bit = 7; bit >= 0; --bit)
                {
                    dest[itemsDone].val = (byte & (1 << bit))
                                              ? items->bit1
                                              : items->bit0;
                    ++itemsDone;
                }

                ++srcDone;
            }

            // On the last byte of the entire source, embed reset
            if (srcDone >= src_size && itemsDone > 0)
            {
                dest[itemsDone - 1].duration1 =
                    static_cast<uint16_t>(items->resetDuration);
            }

            *translated_size = srcDone;
            *item_num = itemsDone;
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_ESP32
