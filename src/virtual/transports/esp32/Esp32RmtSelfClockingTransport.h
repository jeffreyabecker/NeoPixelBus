#pragma once

#if defined(ARDUINO_ARCH_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32C6) && !defined(CONFIG_IDF_TARGET_ESP32H2)

#include <cstdint>
#include <cstddef>
#include <span>

#include <Arduino.h>
#include "driver/rmt.h"
#include "soc/rmt_struct.h"

#include "../ITransport.h"
#include "../OneWireTiming.h"

namespace npb
{

    struct Esp32RmtOneWireTransportConfig
    {
        rmt_channel_t channel = RMT_CHANNEL_0;
        OneWireTiming timing = timing::Ws2812x;
        uint8_t pin = 0;
        bool invert = false;
    };

    class Esp32RmtOneWireTransport : public ITransport
    {
    public:
        using TransportCategory = SelfClockingTransportTag;
        explicit Esp32RmtOneWireTransport(Esp32RmtOneWireTransportConfig config)
            : _config{config}
        {
            _computeRmtItems();
        }

        ~Esp32RmtOneWireTransport()
        {
            if (!_initialised)
            {
                return;
            }

            ESP_ERROR_CHECK_WITHOUT_ABORT(rmt_wait_tx_done(_config.channel, 10000 / portTICK_PERIOD_MS));
            ESP_ERROR_CHECK(rmt_driver_uninstall(_config.channel));

            gpio_matrix_out(_config.pin, SIG_GPIO_OUT_IDX, false, false);
            pinMode(_config.pin, INPUT);
        }

        void begin() override
        {
            if (_initialised)
            {
                return;
            }

            rmt_config_t config = RMT_DEFAULT_CONFIG_TX(
                static_cast<gpio_num_t>(_config.pin),
                _config.channel);
            config.clk_div = RmtClockDivider;
            config.tx_config.idle_level = _config.invert ? RMT_IDLE_LEVEL_HIGH : RMT_IDLE_LEVEL_LOW;
            config.tx_config.idle_output_en = true;

            ESP_ERROR_CHECK(rmt_config(&config));

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 3, 0)
            constexpr uint32_t intFlags = ESP_INTR_FLAG_LOWMED;
#else
            constexpr uint32_t intFlags = ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL1;
#endif
            ESP_ERROR_CHECK(rmt_driver_install(_config.channel, 0, intFlags));
            ESP_ERROR_CHECK(rmt_translator_init(_config.channel, translateCb));

            _initialised = true;
        }

        void transmitBytes(std::span<const uint8_t> data) override
        {
            if (!_initialised)
            {
                begin();
            }

            s_activeItems = &_rmtItems;
            ESP_ERROR_CHECK(rmt_write_sample(
                _config.channel,
                const_cast<uint8_t *>(data.data()),
                data.size(),
                false));
        }

        bool isReadyToUpdate() const override
        {
            if (!_initialised)
            {
                return true;
            }

            return rmt_wait_tx_done(_config.channel, 0) == ESP_OK;
        }

    private:
        static constexpr uint8_t RmtClockDivider = 2;
        static constexpr float NsPerTick = 25.0f;

        struct RmtItems
        {
            uint32_t bit0;
            uint32_t bit1;
            uint32_t resetDuration;
        };

        Esp32RmtOneWireTransportConfig _config;
        RmtItems _rmtItems{};
        bool _initialised{false};

        static inline const RmtItems *s_activeItems = nullptr;

        static uint32_t fromNs(uint32_t ns)
        {
            return static_cast<uint32_t>(static_cast<float>(ns) / NsPerTick + 0.5f);
        }

        uint32_t makeItem32(uint32_t highNs, uint32_t lowNs) const
        {
            uint32_t dur0 = fromNs(highNs);
            uint32_t dur1 = fromNs(lowNs);

            if (_config.invert)
            {
                return (dur1 << 16) | (1u << 31) | dur0;
            }

            return (dur1 << 16) | (1u << 15) | dur0;
        }

        void _computeRmtItems()
        {
            _rmtItems.bit0 = makeItem32(_config.timing.t0hNs, _config.timing.t0lNs);
            _rmtItems.bit1 = makeItem32(_config.timing.t1hNs, _config.timing.t1lNs);
            _rmtItems.resetDuration = fromNs(_config.timing.resetUs * 1000);
        }

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
                    dest[itemsDone].val = (byte & (1 << bit)) ? items->bit1 : items->bit0;
                    ++itemsDone;
                }

                ++srcDone;
            }

            if (srcDone >= src_size && itemsDone > 0)
            {
                dest[itemsDone - 1].duration1 = static_cast<uint16_t>(items->resetDuration);
            }

            *translated_size = srcDone;
            *item_num = itemsDone;
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_ESP32
