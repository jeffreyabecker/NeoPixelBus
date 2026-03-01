# LED Chip Datasheet Summary

Extracted from PDFs in `docs/internal/data-sheets/`.

> **Timing convention**: ranges shown are min–max from the datasheet.
> Chips marked **(inverted)** use inverted NRZ encoding (idle-high, data encoded in low-time).
> Chips marked **(e-RZ)** use extended return-to-zero encoding.

## One-Wire NRZ — 800 kHz Class

| Chip         | Bit Depth             | Channels   | Wires                     | Signal Timing (typical)                                                             | Data Rate     | Default Channel Order | Notes                                                         |
| ------------ | --------------------- | ---------- | ------------------------- | ----------------------------------------------------------------------------------  | ------------- | --------------------- | ------------------------------------------------------------- |
| WS2812B      | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 220–380 ns, T1H 750–1600 ns, T0L 750–1600 ns, T1L 220–420 ns, reset ≥280000 ns  | 800 kbps      | **GRB**               |                                                               |
| WS2812B-V5W  | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 220–380 ns, T1H 580–1000 ns, T0L 580–1000 ns, T1L 580–1000 ns, reset ≥280000 ns | 800 kbps      | **GRB**               | 5V I/O variant                                                |
| WS2812C      | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 220–380 ns, T1H 580–1000 ns, T0L 580–1000 ns, T1L 220–420 ns, reset ≥280000 ns  | 800 kbps      | **GRB**               |                                                               |
| WS2811       | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 220–380 ns, T1H 580–1000 ns, T0L 580–1000 ns, T1L 580–1000 ns                   | 800 kbps      | **RGB**               | External driver IC                                            |
| WS2811C      | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 220–380 ns, T1H 580–1000 ns, T0L 580–1000 ns, T1L 580–1000 ns                   | 800 kbps      | **RGB**               |                                                               |
| WS2813       | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 300–450 ns, T1H 750–1000 ns, reset ≥280000 ns                                   | 800 kbps      | **RGB**               | Backup data pin                                               |
| WS2813-RGBW  | 8                     | 4 (RGBW)   | 1-wire NRZ                | T0H 220–380 ns, T1H 580–1000 ns, reset ≥280000 ns                                   | 800 kbps      | **GRBW**              | 32-bit per pixel                                              |
| WS2813B-V5W  | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 220–380 ns, T1H 580–1000 ns, reset ≥280000 ns                                   | 800 kbps      | **GRB**               |                                                               |
| WS2814       | 8                     | 4 (RGBW)   | 1-wire NRZ                | T0H 220–380 ns, T1H 580–1000 ns, T0L 580–1000 ns, T1L 580–1000 ns                   | 800 kbps      | **RGBW**              | Dual data input                                               |
| WS2814A      | 8                     | 4 (RGBW)   | 1-wire NRZ                | T0H 220–380 ns, T1H 580–1000 ns                                                     | 800 kbps      | **WRGB**              | Note: W byte first!                                           |
| WS2814B      | 8                     | 4 (RGBW)   | 1-wire NRZ                | T0H 220–380 ns, T1H 580–1000 ns, T0L 580–1000 ns, T1L 580–1000 ns                   | 800 kbps      | **RGBW**              |                                                               |
| WS2814C      | 8                     | 4 (RGBW)   | 1-wire NRZ                | T0H 220–380 ns, T1H 580–1000 ns, T0L 580–1000 ns, T1L 580–1000 ns                   | 800 kbps      | **RGBW**              |                                                               |
| WS2815       | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 220–380 ns, T1H 580–1600 ns, T0L 580–1600 ns, T1L 220–420 ns, reset ≥280000 ns  | 800 kbps      | **GRB**               | 12V, backup data pin                                          |
| WS2815B      | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 220–380 ns, T1H 580–840 ns, T0L 900–5000 ns, T1L 600–5000 ns, reset ≥280000 ns  | 800 kbps      | **GRB**               |                                                               |
| WS2818       | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 220–480 ns, T1H 750–2000 ns, T0L 750–2000 ns, T1L 220–480 ns                    | 800 kbps      | **RGB**               | 12V/24V, dual data input                                      |
| WS2818B      | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 220–380 ns, T1H 580–1000 ns, T0L 580–1000 ns, T1L 580–1000 ns                   | 800 kbps      | **RGB**               |                                                               |
| WS2805       | 8                     | 5 (RGBWW)  | 1-wire NRZ                | T0H 220–380 ns, T1H 580–1000 ns, T0L 580–1000 ns, T1L 580–1000 ns                   | 800 kbps      | **RGBCw**             | 40-bit per pixel, 5-channel                                   |
| SK6812       | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 400 ns, T0L 850 ns, T1H 800 ns, T1L 450 ns, reset ≥80000 ns                     | 800 kbps      | **GRB**               |                                                               |
| SK6812-White | 8                     | 3 (WWW)    | 1-wire NRZ                | T0H 400 ns, T0L 850 ns, T1H 800 ns, T1L 450 ns, reset ≥80000 ns                     | 800 kbps      | **WWW**               | White-only variant                                            |
| SK6813       | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 220–380 ns, T1H 750–1600 ns, T0L 750–1600 ns, T1L 220–420 ns, reset ≥280000 ns  | 800 kbps      | **GRB**               | Backup data pin                                               |
| SK6813HV     | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 220–380 ns, T1H 750–1600 ns, T0L 750–1600 ns, T1L 220–420 ns, reset ≥280000 ns  | 800 kbps      | **GRB**               | High-voltage variant                                          |
| APA107       | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 220–380 ns, T1H 750–1600 ns, T0L 750–1600 ns, T1L 220–420 ns, reset ≥280000 ns  | 800 kbps      | **GRB**               |                                                               |
| HC2912C-2020 | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 220–380 ns, T1H 750–1600 ns, T0L 750–1600 ns, T1L 220–420 ns, reset ≥280000 ns  | 800 kbps      | **GRB**               | 2020 package                                                  |
| SM16703P     | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 200–400 ns, T1H 800–1000 ns                                                     | 800 kbps      | **RGB**               |                                                               |
| SM16703SP    | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 200–400 ns                                                                      | 800 kbps      | **RGB**               |                                                               |
| SM16704      | 8                     | 4 (RGBW)   | 1-wire NRZ                | T0H 200–400 ns, T1H 800–1000 ns                                                     | 800 kbps      | **RGB(W)**            | 32-bit; datasheet says "order of RGB"                         |
| SM16704PB    | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 200–400 ns, T1H 800–1000 ns                                                     | 800 kbps      | **RGB**               |                                                               |
| GS8208B      | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 220–380 ns, T1H 750–1600 ns, T0L 750–1600 ns, T1L 220–420 ns, reset ≥280000 ns  | 800 kbps      | **RGB**               | Explicitly states RGB order                                   |
| UCS1903      | 8                     | 3 (RGB)    | 1-wire NRZ                | 800 kbps class                                                                      | 800 kbps      | **RGB**               | OUTR/OUTG/OUTB                                                |
| UCS2903      | 8                     | 3 (RGB)    | 1-wire NRZ                | 800 kbps class                                                                      | 800 kbps      | **RGB**               | External R_EXT                                                |
| UCS2904      | 8                     | 4 (RGBW)   | 1-wire NRZ                | 800 kbps class                                                                      | 800 kbps      | **RGBW**              |                                                               |
| UCS5603      | 12                    | 3 (RGB)    | 1-wire NRZ                | 800 kbps class                                                                      | 800 kbps      | **RGB**               | 4096-level grayscale, bad-point resume                        |
| UCS7604      | 16                    | 4 (RGBW)   | 1-wire NRZ                | 800 kbps class                                                                      | 800 kbps      | **RGBW**              | 65536-level grayscale                                         |
| UCS8903      | 16                    | 3 (RGB)    | 1-wire NRZ                | 800 kbps class                                                                      | 800 kbps      | **RGB**               | 48-bit per pixel (16-bit/ch)                                  |
| UCS8904      | 16                    | 4 (RGBW)   | 1-wire NRZ                | 800 kbps class                                                                      | 800 kbps      | **RGBW**              | 64-bit per pixel (16-bit/ch)                                  |
| UCS8904B     | 16                    | 4 (RGBW)   | 1-wire NRZ                | 800 kbps class                                                                      | 800 kbps      | **RGBW**              | 64-bit per pixel (16-bit/ch)                                  |
| UCS9812      | 16+4                  | 12 (4×RGB) | 1-wire NRZ                | 1100 kbps class                                                                     | 1100 kbps     | **RGB**               | 240-bit frame; 16-bit gray + 4-bit current per ch             |
| TM1812       | 8                     | 12 (4×RGB) | 1-wire NRZ                | T0H 350–450 ns, T1H 700–1000 ns                                                     | 800 kbps      | **RGB**               | 96-bit per chip (4 LED groups)                                |

## One-Wire NRZ — 800 kHz Class, Inverted (Idle-High)

| Chip         | Bit Depth             | Channels   | Wires                     | Signal Timing (typical)                                                             | Data Rate     | Default Channel Order | Notes                                                         |
| ------------ | --------------------- | ---------- | ------------------------- | ----------------------------------------------------------------------------------  | ------------- | --------------------- | ------------------------------------------------------------- |
| TM1814       | 8                     | 4 (WRGB)   | 1-wire NRZ **(inverted)** | T0L 310–410 ns, T1L 650–1000 ns, reset ≥200000 ns                                   | 800 kHz       | **WRGB**              | Inverted NRZ; W byte first                                    |
| TM1903       | 8                     | 3 (RGB)    | 1-wire NRZ **(inverted)** | T0H 310–410 ns, T1H 650–1000 ns, reset ≥50000 ns                                    | 800 kHz       | **RGB**               | Same timing family as TM1814                                  |
| TM1914       | 8                     | 3 (RGB)    | 1-wire NRZ **(inverted)** | T0L 310–410 ns, T1L 650–1000 ns, reset ≥200000 ns                                   | 800 kHz       | **RGB**               | Dual-channel input (DI/FDI)                                   |
| TM1934       | 8                     | 3 (RGB)    | 1-wire NRZ **(inverted)** | T0H 310–410 ns, T1H 650–1000 ns, reset ≥200000 ns                                   | 800 kHz       | **RGB**               | Dual-channel input (DI/FDI)                                   |
| TM1829       | 8                     | 3 (RGB)    | 1-wire NRZ **(inverted)** | T0L 150–450 ns, T1L 600–1000 ns                                                     | 800 kHz       | **RGB**               | Constant current adjustable                                   |
| LB1908       | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 250–350 ns, T1H 700–900 ns, reset ≥200000 ns                                    | 800 kHz       | **RGB**               | 12V, dual data input                                          |

## One-Wire NRZ — 400 kHz Class

| Chip         | Bit Depth             | Channels   | Wires                     | Signal Timing (typical)                                                             | Data Rate     | Default Channel Order | Notes                                                         |
| ------------ | --------------------- | ---------- | ------------------------- | ----------------------------------------------------------------------------------  | ------------- | --------------------- | ------------------------------------------------------------- |
| TM1803       | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H ≈680 ns, T1H ≈1360 ns, T0L ≈1360 ns, T1L ≈680 ns                                | 400 kbps      | **RGB**               |                                                               |
| TM1804       | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 350–900 ns, T1H 700–2000 ns (has slow/fast modes)                               | 400 kbps      | **RGB**               | Dual-speed datasheet                                          |
| TM1809       | 8                     | 3 (RGB)    | 1-wire NRZ                | T0H 250–750 ns, T1H 530–1350 ns (variable, slow/fast)                               | 400 kbps      | **RGB**               | Dual-speed datasheet                                          |

## One-Wire e-RZ (Extended Return-to-Zero) — 800 kHz

| Chip         | Bit Depth             | Channels   | Wires                     | Signal Timing (typical)                                                             | Data Rate     | Default Channel Order | Notes                                                         |
| ------------ | --------------------- | ---------- | ------------------------- | ----------------------------------------------------------------------------------  | ------------- | --------------------- | ------------------------------------------------------------- |
| CS8812       | 8 (12-bit Γ)          | 3 (RGB)    | 1-wire **(e-RZ)**         | e-RZ encoding, reset ≥300000 ns                                                     | 800 kHz       | **RGB**               | 12V; built-in 12-bit gamma; dual SDI                          |
| GS8206       | 8 (12-bit Γ)          | 3 (RGB)    | 1-wire **(e-RZ)**         | e-RZ encoding, reset ≥300000 ns                                                     | 800 kHz       | **RGB**               | 5–24V; built-in 12-bit gamma; dual SDI                        |
| GS8208       | 8 (12-bit Γ)          | 3 (RGB)    | 1-wire **(e-RZ)**         | e-RZ encoding, reset ≥300000 ns                                                     | 800 kHz       | **RGB**               | 12V; built-in 12-bit gamma; dual SDI                          |

## One-Wire — Special / 6-Channel

| Chip         | Bit Depth             | Channels   | Wires                     | Signal Timing (typical)                                                             | Data Rate     | Default Channel Order | Notes                                                         |
| ------------ | --------------------- | ---------- | ------------------------- | ----------------------------------------------------------------------------------  | ------------- | --------------------- | ------------------------------------------------------------- |
| FW1906       | 8                     | 6 (2×RGB)  | 1-wire NRZ                | T0H 320–400 ns, T1H 640–800 ns, cycle ≈1250 ns                                      | 800 kHz       | **RGB** (×2)          | 24V; 6ch drives 2 LED groups                                  |







## Two-Wire SPI (Clock + Data)

| Chip         | Bit Depth             | Channels   | Wires                     | Signal Timing (typical)                                                             | Data Rate     | Default Channel Order | Notes                                                         |
| ------------ | --------------------- | ---------- | ------------------------- | ----------------------------------------------------------------------------------  | ------------- | --------------------- | ------------------------------------------------------------- |
| APA102       | 8 + 5-bit brightness  | 3 (RGB)    | 2-wire SPI                | CLK ≥ 30 ns high/low                                                                | Up to ~30 MHz | **RGB**               | 32-bit frame: 111+5 brightness + 8B + 8G + 8R (BGR on wire)   |
| APA102-1515  | 8 + 5-bit brightness  | 3 (RGB)    | 2-wire SPI                | CLK ≥ 30 ns high/low                                                                | Up to ~30 MHz | **RGB**               | 1515 package                                                  |
| APA102-2020  | 8 + 5-bit brightness  | 3 (RGB)    | 2-wire SPI                | CLK ≥ 30 ns high/low                                                                | Up to ~30 MHz | **RGB**               | 2020 package                                                  |
| HD107S       | 8 + 5-bit brightness  | 3 (RGB)    | 2-wire SPI                | CLK ≥ 30 ns                                                                         | Up to 30 MHz  | **RGB**               | APA102-compatible                                             |
| HD108        | 16 + 5-bit brightness | 3 (RGB)    | 2-wire SPI                | CLK ≥ 30 ns                                                                         | Up to 32 MHz  | **RGB**               | 65536-level grayscale per ch                                  |
| SM16716      | 8                     | 3 (RGB)    | 2-wire (DCLK+DIN)         | CLK up to 30 MHz                                                                    | Up to 30 MHz  | **RGB**               | 3ch open-drain constant current                               |
| LPD6803      | 5                     | 3 (RGB)    | 2-wire SPI                | CLK up to 24 MHz                                                                    | Up to 24 MHz  | **RGB**               | 15-bit color (5-bit/ch), reversed-gamma                       |
| WS2801       | 8                     | 3 (RGB)    | 2-wire SPI                | 8 ns typical delay                                                                  | Up to 25 MHz  | **RGB**               |                                                               |
| WS2801S      | 8                     | 3 (RGB)    | 2-wire SPI                | 8 ns typical delay                                                                  | Up to 25 MHz  | **RGB**               | Variant of WS2801                                             |

---

## Inverted NRZ Encoding Summary

Several Titan Micro chips use an **inverted NRZ** (idle-high) signal encoding.
Where standard NRZ chips (WS2812x, SK6812, etc.) idle low and encode data in the
high pulse duration, inverted-NRZ chips idle **high** and encode data in the
**low** pulse duration. The reset/latch signal is a sustained **high** level
instead of a sustained low.

### How Inverted NRZ Differs from Standard NRZ

```
Standard NRZ (WS2812x)             Inverted NRZ (TM1814)
Line idles LOW                     Line idles HIGH

  ┌── T0H ──┐                           ┌─────── idle ────────┐
  │         │                           │                     │
──┘         └── T0L ──  (0 bit)    ─────┘   ┌── T0L ──┐      │  (0 bit)
                                            │         │
                                       ─────┘         └──────

  ┌──── T1H ────┐                       ┌─────── idle ────────┐
  │             │                       │                     │
──┘             └ T1L ─  (1 bit)   ─────┘  ┌─── T1L ───┐     │  (1 bit)
                                           │           │
                                      ─────┘           └─────

Reset: hold LOW ≥ 280000 ns          Reset: hold HIGH ≥ 200000 ns
```

In the datasheet tables, the timing parameters are labeled differently:

| Standard NRZ          | Inverted NRZ         | Meaning                |
| --------------------- | -------------------- | ---------------------- |
| T0H (high time for 0) | T0L (low time for 0) | Short pulse = zero bit |
| T1H (high time for 1) | T1L (low time for 1) | Long pulse = one bit   |
| Reset: low ≥ N ns     | Reset: high ≥ N ns   | Latch frame data       |

The **data content** (bit encoding, channel order, frame structure) is the same as
standard NRZ — only the signal polarity is flipped. A "0" is still represented by
a short pulse and a "1" by a long pulse; the only difference is which voltage
level is active.

### Timing Values from Datasheets

| Chip       | T0L (0-bit low)      | T1L (1-bit low)       | Bit Cycle | Reset (high)         | Data Rate |
| ---------- | -------------------- | --------------------- | --------- | -------------------- | --------- |
| **TM1814** | 310–410 ns (typ 360) | 650–1000 ns (typ 720) | 1250 ns   | ≥ 200000 ns (max 20 ms) | 800 kHz   |
| **TM1914** | 310–410 ns (typ 360) | 650–1000 ns (typ 720) | 1250 ns   | ≥ 200000 ns             | 800 kHz   |
| **TM1934** | 310–410 ns (typ 360) | 650–1000 ns (typ 720) | 1250 ns   | ≥ 200000 ns             | 800 kHz   |
| **TM1829** | 150–450 ns (typ 300) | 600–1000 ns (typ 800) | ≥ 1200 ns | 140–500000 ns           | 800 kHz   |

TM1903 and LB1908 are **not** inverted — their datasheets specify T0H/T1H (high
level times), and their DO port idles low before forwarding. They share the same
Titan Micro family but are standard-polarity NRZ chips.

### `OneWireTiming` Struct Mapping

In NeoPixelBus, the `OneWireTiming` struct always uses the field names `t0hNs`,
`t0lNs`, `t1hNs`, `t1lNs` with **standard NRZ semantics** regardless of
inversion. The mapping for inverted chips is:

| Datasheet (inverted)  | `OneWireTiming` field | Description                                |
| --------------------- | --------------------- | ------------------------------------------ |
| T0L (short low pulse) | `t0hNs`               | Duration of the "active" phase for a 0-bit |
| T0 cycle − T0L        | `t0lNs`               | Duration of the "idle" phase for a 0-bit   |
| T1L (long low pulse)  | `t1hNs`               | Duration of the "active" phase for a 1-bit |
| T1 cycle − T1L        | `t1lNs`               | Duration of the "idle" phase for a 1-bit   |

Signal inversion itself (flipping the output pin polarity) is handled at the
**transport layer** level, not in the timing struct. The `invert` field in
transport settings controls whether the hardware output is inverted.

### Data Frame Structure

**TM1814** (4 channels — WRGB):

Each pixel frame is **32 bits**, transmitted MSB-first in the order
**W[7:0] R[7:0] G[7:0] B[7:0]** — the white channel is sent first.

A complete data stream requires two **constant-current command packets** (C1 + C2,
32 bits each) before the pixel data. C1 sets per-channel constant current (6-bit
per channel, 64 levels from 6.5 mA to 38 mA). C2 is the bitwise complement of C1
for validation.

```
Frame: C1(32b) C2(32b) D1(32b) D2(32b) ... Dn(32b) Reset(high ≥200000 ns)
```

**TM1914 / TM1934** (3 channels — RGB):

Each pixel frame is **24 bits**, transmitted MSB-first: **R[7:0] G[7:0] B[7:0]**.

These chips also require mode-setting command packets before pixel data:
- `0xFFFFFF_000000` → Normal mode (DIN/FDIN auto-switch)
- `0xFFFFFA_000005` → DIN-only mode
- `0xFFFFF5_00000A` → FDIN-only mode

```
Frame: C1(24b) C2(24b) D1(24b) D2(24b) ... Dn(24b) Reset(high ≥200000 ns)
```

**TM1829** (3 channels — RGB, with per-channel constant current):

Each pixel frame is **24 bits**, transmitted MSB-first: **R[7:0] G[7:0] B[7:0]**.

Constant-current commands are multiplexed in-band: if the high byte (bits 23–16)
is all 1s (`0xFF`), the packet is treated as a constant-current setting
(5-bit per channel, 32 levels from 10 mA to 41 mA) instead of a PWM command.
Otherwise, the 24 bits are interpreted as PWM duty-cycle data.

```
Frame: D1(24b) D2(24b) ... Dn(24b) Reset(high ≥500000 ns)
       (or constant-current packet if high byte = 0xFF)
```

### Dual-Channel Data Input

| Chip       | Inputs     | Redundancy Model                                                     |
| ---------- | ---------- | -------------------------------------------------------------------- |
| **TM1814** | DIN only   | Single input; DO always present                                      |
| **TM1914** | DIN + FDIN | Mode-switchable; dual DO (DO1 + DO2). Auto-failover after 300 ms     |
| **TM1934** | DI + FDI   | Mode-switchable; single DO. Auto-failover                            |
| **TM1829** | DIN only   | Single input                                                         |
| **TM1903** | DIN only   | Standard NRZ (not inverted)                                          |
| **LB1908** | DIN + FDIN | Dual input, standard NRZ (not inverted). FDIN discards first 24 bits |

TM1914 provides the richest redundancy: two independent data inputs (DIN, FDIN)
and two independent outputs (DO1, DO2), with automatic channel switching after
300 ms of no signal. TM1934 is similar but has a single DO output.

### Chip Comparison

| Chip   | Channels | Bits/Pixel | Constant Current                  | Channel Order | VDD Range             | I_OUT Range | Package             |
| ------ | -------- | ---------- | --------------------------------- | ------------- | --------------------- | ----------- | ------------------- |
| TM1814 | 4 (WRGB) | 32         | 64 levels (6.5–38 mA) per channel | **WRGB**      | 6–24 V (via series R) | 6.5–38 mA   | SOP8                |
| TM1914 | 3 (RGB)  | 24         | 18 mA fixed                       | **RGB**       | 6–24 V (via series R) | 18 mA       | MSOP10/SSOP10/ESOP8 |
| TM1934 | 3 (RGB)  | 24         | 15 mA fixed                       | **RGB**       | 6–24 V (via series R) | 15 mA       | SOP8/SOT23-8        |
| TM1829 | 3 (RGB)  | 24         | 32 levels (10–41 mA) per channel  | **RGB**       | 6–24 V (via series R) | 10–41 mA    | SOP8/DIP8           |

### Key Differences Between Inverted Chips

1. **TM1814 is the only 4-channel (WRGB)** inverted chip. Its data order is
   **W-R-G-B** (white first), and each frame carries 32 bits instead of 24.

2. **TM1814 and TM1829 have per-channel adjustable constant current.** TM1814
   offers 64-level adjustment (6.5–38 mA); TM1829 offers 32-level adjustment
   (10–41 mA). The TM1914 and TM1934 have fixed output current.

3. **TM1829 has wider timing tolerance** (T0L 150–450 ns vs TM1814's 310–410 ns)
   and a longer reset requirement (up to 500000 ns). Its cycle time is ≥ 1200 ns
   (slightly slower than the 1250 ns of TM1814/TM1914).

4. **TM1814 switches to internal control mode** (cycling WRGB patterns) if no
   signal is detected for ~500 ms. It then drives up to 2,048 cascade points
   with synchronized pattern data on DO.

5. **All inverted chips are Titan Micro products** using built-in 5 V LDO
   regulators. The external VDD supply (typically 12 V or 24 V) connects through
   a series resistor calculated as `R = (V_DC − 5.5 V) / 10 mA`.

> **Note for NeoPixelBus:** Signal inversion is a transport-layer concern
> configured via the `invert` flag in transport settings. The `OneWireTiming`
> presets (`Tm1814`, `Tm1914`, `Tm1829`) store timing values in the standard
> struct layout; the transport hardware flips the signal polarity.

---

## e-RZ Encoding Summary

CS8812, GS8206, and GS8208 all use an identical encoding called **e-RZ** (extended
return-to-zero code). The three datasheets describe the same signal format
word-for-word. GS8208B (the integrated LED package of GS8208) also uses e-RZ
internally but its datasheet describes the timing at the user level as standard
NRZ-compatible (TH+TL = 1250 ns ± 600 ns, T0H ≈ 300 ns, T1H ≈ 900 ns) because
the IC inside the package handles e-RZ re-encoding on the inter-chip SDO line.

### How e-RZ Differs from Standard NRZ

Standard one-wire NRZ (WS2812x-style) encodes each bit as a high pulse followed
by a low pulse, where the **ratio** of high-to-low determines 0 or 1. The total
bit period is typically 1250 ns (800 kHz).

e-RZ uses a **1:3 duty-cycle return-to-zero** scheme instead:

| Symbol     | High Time | Low Time | Total Period |
| ---------- | --------- | -------- | ------------ |
| **0 code** | ¼T        | ¾T       | T            |
| **1 code** | ¾T        | ¼T       | T            |

where **T = 1/800 kHz = 1250 ns** (standard speed).

Both 0 and 1 codes **return to zero** (low) before the next symbol, but the
high/low ratio is inverted between the two codes — exactly mirrored around the
midpoint. This differs from standard NRZ in two ways:

1. **Symmetric duty-cycle encoding.** A 0 is 25% high / 75% low; a 1 is 75% high /
   25% low. Standard NRZ chips have asymmetric ranges (e.g., WS2812x T0H ≈ 320 ns,  
   T1H ≈ 800 ns — roughly 1:3 but with different tolerances and no strict quartile
   relationship).

2. **Return-to-zero guarantee.** The datasheet specifies the line returns to low for
   at least ¾T on every 0 code and ¼T on every 1 code, plus allows up to 10000 ns
   inter-code idle intervals without triggering a reset. Only a gap ≥ 300000 ns
   constitutes a reset/latch. This makes e-RZ more tolerant of controller-side
   jitter between symbols.

### Backwards Compatibility

The datasheets state: *"The extension type is compatible with the traditional RZ
code. So it is suitable for most of the RZ code controller in the market."*

In practice, the ¼T/¾T timing at 800 kHz works out to:

| Parameter         | e-RZ Value    | WS2812 Typical | WS2812 Acceptance |
| ----------------- | ------------- | -------------- | ----------------- |
| T0H (0 code high) | 312.5 ns (¼T) | 350 ns         | 220–380 ns ✓      |
| T0L (0 code low)  | 937.5 ns (¾T) | 900 ns         | 580–1600 ns ✓     |
| T1H (1 code high) | 937.5 ns (¾T) | 800 ns         | 580–1600 ns ✓     |
| T1L (1 code low)  | 312.5 ns (¼T) | 450 ns         | 220–420 ns ✓      |

All four values fall within the WS2812x acceptance windows, which is why standard
WS2812-class controllers can drive these chips without modification.

### Data Format

- 8 bits per channel, 3 channels (RGB) per chip → 24 bits per IC.
- High bit transmitted first: Ch0 b7 → b0, Ch1 b7 → b0, Ch2 b7 → b0.
- Each chip consumes its first 24 bits and re-encodes + forwards the remainder on SDO.
- Data re-encoding delay: < 700 ns chip-to-chip.

### Dual-SDI Redundancy

All three chips provide **dual serial data inputs** (SDI + SDI2). SDI is the
default channel after power-on. If the chip detects data corruption on the active
channel, it automatically switches priority to the other. SDI2 discards the first
24 bits (its own pixel) and uses bits 25–48 as display data, providing one-chip
offset redundancy. Single-chip failure does not break the downstream chain.

### Internal Features

| Feature                 | Details                                                   |
| ----------------------- | --------------------------------------------------------- |
| Gamma correction        | 8-bit input → 12-bit internal (built-in curve)            |
| PWM refresh             | 8 kHz (MPWM — distributes duty across 4 sub-periods)      |
| Stagger delay           | OUT1/OUT2/OUT3 offset by 80 ns to reduce EMI              |
| Internal patterns       | 6 categories × 32 series; ~10 min cycle at 100 Hz         |
| Power-on test           | Automatic R/G/B sequence for production validation        |
| RZ data frequency range | 400 kHz – 1 MHz (CS8812/GS8208); 800 kHz typical (GS8206) |
| Reset                   | ≥ 300000 ns low                                              |

### Chip Variants

| Chip    | VDH Range   | Default I_OUT | Series LED Topology | Package      |
| ------- | ----------- | ------------- | ------------------- | ------------ |
| CS8812  | 9–15 V      | 15 mA         | 12V series RGB      | SOP8         |
| GS8206  | 5–24 V      | 17.5 mA       | 5–24V series RGB    | SOP8 / SOP16 |
| GS8208  | 9–15 V      | 15 mA         | 12V series RGB      | SOP8         |
| GS8208B | 10.5–13.5 V | 9 mA          | Integrated 5050 LED | SMD 5050     |

> **Note for NeoPixelBus:** e-RZ chips are electrically compatible with standard
> 800 kHz NRZ controllers at the signal level. The differentiation matters for
> understanding their internal re-encoding and the dual-SDI redundancy feature,
> but from a transport perspective they can be driven with `OneWireTiming::Ws2812x`
> or `OneWireTiming::Generic800`.

---

## Key Observations

1. **WS2812B/WS2812C/WS2815 use GRB** order — Green byte first, then Red, then Blue.
2. **WS2811/WS2813/WS2818 use RGB** order — the data represents R first in the stream.
3. **WS2813-RGBW uses GRBW** — Green first, then Red, Blue, White.
4. **WS2814 uses RGBW**; the **WS2814A variant uses WRGB** (White byte first!).
5. **TM1814 uses WRGB** — White byte is transmitted first (inverted NRZ).
6. **SK6812 family uses GRB** — same as WS2812x.
7. **APA107 uses GRB** despite being a one-wire chip (not SPI like APA102).
8. **e-RZ chips** (CS8812, GS8206, GS8208) use a different encoding scheme than standard NRZ.
9. **UCS high-grayscale chips**: UCS5603 = 12-bit, UCS7604/UCS8903/UCS8904 = 16-bit per channel.
10. **UCS9812** is a 12-channel (4×RGB) driver with 16-bit gray + 4-bit current per channel (240-bit frame).
11. **TM1812** is also a 12-channel (4×RGB) driver using 96-bit frames.
12. **FW1906** is a 6-channel (2×RGB) driver at 800 kHz NRZ.
13. **WS2805** is a rare 5-channel chip (R, G, B, W1, W2) using 40-bit per pixel.

