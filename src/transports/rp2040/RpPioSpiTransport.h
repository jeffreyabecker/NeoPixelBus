#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include <cstdint>
#include <cstddef>

#include <Arduino.h>
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"

#include "transports/ITransport.h"
#include "RpDmaStateTracker.h"

namespace npb
{

#ifndef NEOPIXELBUS_RP_PIO_CLOCK_DATA_DEFAULT_HZ
#define NEOPIXELBUS_RP_PIO_CLOCK_DATA_DEFAULT_HZ 10000000UL
#endif
    static constexpr uint32_t RpPioClockDataDefaultHz = NEOPIXELBUS_RP_PIO_CLOCK_DATA_DEFAULT_HZ;

    namespace rpPioSpi
    {


   

        struct InvertSetting
        {
            bool invert = false;
        };

        struct ClockDataBitRateSetting
        {
            uint32_t clockRateHz = RpPioClockDataDefaultHz;
        };

        struct RequiredSettings
        {
            uint8_t clockPin;
            uint8_t dataPin;
            uint8_t pioIndex;
        };
        template<typename T>
        struct IsSpiTransportSettings : std::false_type {};

        template<>
        struct IsSpiTransportSettings<RpPioSpiTransportSettings> : std::true_type {};

        template<typename T>
        static constexpr bool IsSpiTransportSettingsV = IsSpiTransportSettings<T>::value;

    }



    struct RpPioSpiTransportSettings : public rpPioSpi::RequiredSettings,

                                     public rpPioSpi::InvertSetting,
                                     public rpPioSpi::ClockDataBitRateSetting
    {

    };
    struct RpPioSpiTransportSettingsFactory{
        template<typename TTransportSettings, typename = std::enable_if_t<rpPioSpi::IsSpiTransportSettingsV<TTransportSettings>>>
        static RpPioSpiTransportSettings create(TTransportSettings config){
            RpPioSpiTransportSettings settings{};
            settings.clockPin = config.clockPin;
            settings.dataPin = config.dataPin;
            // Assert that dataPin = clockPin + 1 (PIO requirement for consecutive pins)
            static_assert(std::is_same_v<TTransportSettings, RpPioSpiTransportSettings> || 
                          config.dataPin == config.clockPin + 1, 
                          "dataPin must equal clockPin + 1 for PIO SPI transport");
            settings.pioIndex = config.pioIndex;
            if constexpr (std::is_base_of_v<rpPioSpi::InvertSetting, TTransportSettings>)
            {
                settings.invert = config.invert;
            }
            else
            {
                settings.invert = false;
            }
            if constexpr (std::is_base_of_v<rpPioSpi::ClockDataBitRateSetting, TTransportSettings>)
            {
                settings.clockRateHz = config.clockRateHz;
            }
            else
            {
                settings.clockRateHz = RpPioClockDataDefaultHz;
            }
            return settings;
        }

        template <typename TTransportSettings, typename = std::enable_if_t<!rpPioSpi::IsSpiTransportSettingsV<TTransportSettings>>>
        static RpPioSpiTransportSettings create(OneWireTiming timing, TTransportSettings config){
            RpPioSpiTransportSettings settings = create(config);
            settings.clockRateHz = static_cast<uint32_t>(timing.bitRateHz()) * 8U * 2U; // Default to 8 bits per data bit, and a bit pattern that uses 2 PIO instructions per data bit, so multiply the bit rate by 16 to get the default clock rate.
            return settings;
        }
    };

    class RpPioSpiTransport : public ITransport
    {
    public:
        using TransportSettingsType = RpPioSpiTransportSettings;
        using TransportCategory = TransportTag;
        static constexpr uint IrqIndex = 1;
        static constexpr uint8_t BitCycles = 2;

        explicit RpPioSpiTransport(RpPioSpiTransportSettings config)
            : _config{config}
            , _pio{resolvePio(config.pioIndex)}
            , _mergedFifoCount{static_cast<uint8_t>((_pio->dbg_cfginfo & PIO_DBG_CFGINFO_FIFO_DEPTH_BITS) * 2)}
        {
        }

        ~RpPioSpiTransport() override
        {
            if (!_initialised)
            {
                return;
            }

            while (!isReadyToUpdate())
            {
                yield();
            }

            pio_sm_clear_fifos(_pio, _sm);
            pio_sm_set_enabled(_pio, _sm, false);

            dma_irqn_set_channel_enabled(IrqIndex, _dmaChannel, false);
            _dmaState.unregisterChannel(_dmaChannel);

            dma_channel_unclaim(_dmaChannel);
            pio_sm_unclaim(_pio, _sm);

            if (_config.invert)
            {
                gpio_set_outover(_config.dataPin, GPIO_OVERRIDE_NORMAL);
            }

            pinMode(_config.dataPin, INPUT);
            if (_config.clockPin >= 0)
            {
                pinMode(_config.clockPin, INPUT);
            }
        }

        void begin() override
        {
            if (_initialised)
            {
                return;
            }

            if (_config.clockPin < 0 || _config.clockRateHz == 0)
            {
                return;
            }



            float bitLengthUs = 1'000'000.0f / static_cast<float>(_config.clockRateHz);
            _fifoCacheEmptyDelta = static_cast<uint32_t>(bitLengthUs * 8.0f * (_mergedFifoCount + 1));

            uint offset = loadProgram(_pio);
            _sm = pio_claim_unused_sm(_pio, true);
            initSm(_pio, _sm, offset, static_cast<uint>(_config.clockPin), _config.dataPin, static_cast<float>(_config.clockRateHz));

            if (_config.invert)
            {
                gpio_set_outover(_config.dataPin, GPIO_OVERRIDE_INVERT);
            }

            _dmaChannel = dma_claim_unused_channel(true);
            _dmaState.registerChannel(_dmaChannel);
            dma_irqn_set_channel_enabled(IrqIndex, _dmaChannel, true);

            _initialised = true;
        }

        void beginTransaction() override
        {
        }

        void transmitBytes(span<const uint8_t> data) override
        {
            if (!_initialised)
            {
                begin();
            }

            if (!_initialised || data.empty())
            {
                return;
            }

            while (!isReadyToUpdate())
            {
                yield();
            }
            dma_channel_transfer_size transferDataSize = data.size() % 4 == 0 ? DMA_SIZE_32 : (data.size() % 2 == 0 ? DMA_SIZE_16 : DMA_SIZE_8);
            uint transferCount = static_cast<uint>(data.size() / (1 << transferDataSize));

            dma_channel_config cfg = dma_channel_get_default_config(_dmaChannel);
            channel_config_set_transfer_data_size(&cfg, transferDataSize);
            channel_config_set_read_increment(&cfg, true);
            channel_config_set_write_increment(&cfg, false);
            channel_config_set_dreq(&cfg, pio_get_dreq(_pio, _sm, true));

            dma_channel_configure(
                _dmaChannel,
                &cfg,
                reinterpret_cast<io_rw_8 *>(&(_pio->txf[_sm])),
                data.data(),
                transferCount,
                false);            

            _dmaState.setSending();
            dma_channel_set_read_addr(_dmaChannel, data.data(), false);
            dma_channel_start(_dmaChannel);
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

            return _dmaState.isReadyToSend(_fifoCacheEmptyDelta);
        }

    private:
        static constexpr uint NotLoaded = static_cast<uint>(-1);
        //TODO: check this implementation its way different from the pio code in the example
        static inline const uint16_t ProgramInstructions[] = {
            static_cast<uint16_t>(pio_encode_out(pio_pins, 1) | pio_encode_sideset(1, 0)),
            static_cast<uint16_t>(pio_encode_nop() | pio_encode_sideset(1, 1)),
        };

        static inline const pio_program Program = {
            .instructions = ProgramInstructions,
            .length = 2,
            .origin = -1,
            .pio_version = 0,
            .used_gpio_ranges = 0,
        };

        static inline uint s_programOffset[NUM_PIOS] = {
#if NUM_PIOS == 2
            NotLoaded, NotLoaded
#elif NUM_PIOS == 3
            NotLoaded, NotLoaded, NotLoaded
#endif
        };

        RpPioSpiTransportSettings _config;
        PIO _pio;
        uint8_t _mergedFifoCount;

        int _sm{-1};
        int _dmaChannel{-1};

        uint32_t _fifoCacheEmptyDelta{0};
        bool _initialised{false};

        RpDmaStateTracker<IrqIndex> _dmaState;

        static PIO resolvePio(uint8_t index)
        {
            switch (index)
            {
            case 0:
                return pio0;
#if NUM_PIOS >= 2
            case 1:
                return pio1;
#endif
#if NUM_PIOS >= 3
            case 2:
                return pio2;
#endif
            default:
                return pio0;
            }
        }

        static uint pioIndex(PIO pio)
        {
#if NUM_PIOS == 2
            return (pio == pio0) ? 0 : 1;
#elif NUM_PIOS == 3
            return (pio == pio0) ? 0 : (pio == pio1 ? 1 : 2);
#endif
        }

        static uint loadProgram(PIO pio)
        {
            uint idx = pioIndex(pio);
            if (s_programOffset[idx] == NotLoaded)
            {
                s_programOffset[idx] = pio_add_program(pio, &Program);
            }
            return s_programOffset[idx];
        }

        static void initSm(PIO pio, uint sm, uint offset, uint clockPin, uint dataPin, float bitRateHz)
        {
            float div = static_cast<float>(clock_get_hz(clk_sys)) / (bitRateHz * BitCycles);

            pio_sm_config c = pio_get_default_sm_config();
            sm_config_set_wrap(&c, offset + 0, offset + 1);
            sm_config_set_sideset(&c, 1, false, false);
            sm_config_set_sideset_pins(&c, clockPin);
            sm_config_set_out_pins(&c, dataPin, 1);
            sm_config_set_out_shift(&c, false, true, 8);
            sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
            sm_config_set_clkdiv(&c, div);

            pio_gpio_init(pio, dataPin);
            pio_gpio_init(pio, clockPin);
            pio_sm_set_consecutive_pindirs(pio, sm, dataPin, 1, true);
            pio_sm_set_consecutive_pindirs(pio, sm, clockPin, 1, true);

            pio_sm_init(pio, sm, offset, &c);
            pio_sm_set_enabled(pio, sm, true);
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_RP2040


