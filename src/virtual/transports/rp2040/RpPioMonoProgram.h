#pragma once

#ifdef ARDUINO_ARCH_RP2040

#include <cstdint>
#include <cassert>

#include "hardware/pio.h"
#include "hardware/clocks.h"

namespace npb
{

    /// 3-step PIO cadence (33/33/33 duty split).
    /// Good for chips where T0H ≈ T0L (e.g. WS2812x, SK6812, TM1814).
    ///
    /// PIO program (pioasm source):
    /// ```
    ///   .program rgbic_mono
    ///   .side_set 1
    ///   .wrap_target
    ///   bitloop:
    ///       out x, 1       side 0 [TL1 - 1]
    ///       jmp !x do_zero side 1 [TH0 - 1]
    ///   do_one:
    ///       jmp bitloop    side 1 [TH1 - 1]
    ///   do_zero:
    ///       nop            side 0 [TH1 - 1]
    ///   .wrap
    /// ```
    struct RpPioCadence3Step
    {
        static constexpr uint8_t BitCycles = 3; // TH0(1) + TH1(1) + TL1(1)

        static constexpr uint8_t WrapTarget = 0;
        static constexpr uint8_t Wrap = 3;

        static inline const uint16_t Instructions[] =
        {
            0x6021,  // out x, 1       side 0
            0x1023,  // jmp !x, 3      side 1
            0x1000,  // jmp 0          side 1
            0xa042,  // nop            side 0
        };

        static inline const pio_program Program =
        {
            .instructions = Instructions,
            .length = 4,
            .origin = -1,
            .pio_version = 0,
            .used_gpio_ranges = 0,
        };
    };

    /// 4-step PIO cadence (25/50/25 duty split).
    /// Good for chips with asymmetric pulse widths (e.g. WS2811, APA106).
    ///
    /// Same program structure but TH1 has a [1] delay, doubling
    /// the middle phase to 50%.
    struct RpPioCadence4Step
    {
        static constexpr uint8_t BitCycles = 4; // TH0(1) + TH1(2) + TL1(1)

        static constexpr uint8_t WrapTarget = 0;
        static constexpr uint8_t Wrap = 3;

        static inline const uint16_t Instructions[] =
        {
            0x6021,  // out x, 1       side 0
            0x1023,  // jmp !x, 3      side 1
            0x1100,  // jmp 0          side 1 [1]
            0xa142,  // nop            side 0 [1]
        };

        static inline const pio_program Program =
        {
            .instructions = Instructions,
            .length = 4,
            .origin = -1,
            .pio_version = 0,
            .used_gpio_ranges = 0,
        };
    };

    /// Manages lazy-loading of a PIO cadence program into PIO instruction
    /// memory.  One program is loaded at most once per PIO block; subsequent
    /// calls to `load()` return the cached offset.
    class RpPioMonoProgram
    {
    public:
        /// Load the given cadence program into the specified PIO block
        /// (if not already loaded) and return the instruction offset.
        static uint load3Step(PIO pio)
        {
            return loadProgram(pio, &RpPioCadence3Step::Program);
        }

        static uint load4Step(PIO pio)
        {
            return loadProgram(pio, &RpPioCadence4Step::Program);
        }

        /// Initialize a state machine for one-wire output.
        ///
        /// @param pio          PIO instance (pio0 / pio1 / pio2)
        /// @param sm           State machine index
        /// @param offset       Program offset (from load3Step / load4Step)
        /// @param pin          GPIO pin number
        /// @param bitRateHz    Desired bit rate
        /// @param bitCycles    Number of PIO cycles per bit (3 or 4)
        /// @param shiftBits    FIFO word width (8, 16, or 32)
        static void initSm(PIO pio, uint sm, uint offset,
                            uint pin, float bitRateHz,
                            uint8_t bitCycles, uint shiftBits)
        {
            float div = static_cast<float>(clock_get_hz(clk_sys))
                        / (bitRateHz * bitCycles);

            pio_sm_config c = pio_get_default_sm_config();
            sm_config_set_wrap(&c, offset + 0, offset + 3);
            sm_config_set_sideset(&c, 1, false, false);
            sm_config_set_sideset_pins(&c, pin);
            sm_config_set_out_shift(&c, false, true, shiftBits); // MSB first, auto-pull
            sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
            sm_config_set_clkdiv(&c, div);

            pio_gpio_init(pio, pin);
            pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);
            pio_sm_init(pio, sm, offset, &c);
            pio_sm_set_enabled(pio, sm, true);
        }

    private:
        static constexpr uint NotLoaded = static_cast<uint>(-1);

        // Indexed by PIO block number.  Separate tables for each cadence.
        static inline uint s_offset3Step[NUM_PIOS] =
        {
#if NUM_PIOS == 2
            NotLoaded, NotLoaded
#elif NUM_PIOS == 3
            NotLoaded, NotLoaded, NotLoaded
#endif
        };

        static inline uint s_offset4Step[NUM_PIOS] =
        {
#if NUM_PIOS == 2
            NotLoaded, NotLoaded
#elif NUM_PIOS == 3
            NotLoaded, NotLoaded, NotLoaded
#endif
        };

        static uint pioIndex(PIO pio)
        {
#if NUM_PIOS == 2
            return (pio == pio0) ? 0 : 1;
#elif NUM_PIOS == 3
            return (pio == pio0) ? 0 : (pio == pio1 ? 1 : 2);
#endif
        }

        static uint loadProgram(PIO pio, const pio_program *prog)
        {
            uint *table = (prog == &RpPioCadence3Step::Program)
                              ? s_offset3Step
                              : s_offset4Step;
            uint idx = pioIndex(pio);

            if (table[idx] == NotLoaded)
            {
                assert(pio_can_add_program(pio, prog));
                table[idx] = pio_add_program(pio, prog);
            }
            return table[idx];
        }
    };

} // namespace npb

#endif // ARDUINO_ARCH_RP2040
