# LED Chip Datasheet Summary

Extracted from PDFs in `docs/data-sheets/`.

> **Timing convention**: ranges shown are min–max from the datasheet.
> Chips marked **(inverted)** use inverted NRZ encoding (idle-high, data encoded in low-time).
> Chips marked **(e-RZ)** use extended return-to-zero encoding.

## One-Wire NRZ — 800 kHz Class

| Chip | Bit Depth | Channels | Wires | Signal Timing (typical) | Data Rate | Default Channel Order | Notes |
|------|-----------|----------|-------|-------------------------|-----------|----------------------|-------|
| WS2812B | 8 | 3 (RGB) | 1-wire NRZ | T0H 220–380 ns, T1H 750–1600 ns, T0L 750–1600 ns, T1L 220–420 ns, reset ≥280 µs | 800 kbps | **GRB** | |
| WS2812B-V5W | 8 | 3 (RGB) | 1-wire NRZ | T0H 220–380 ns, T1H 580–1000 ns, T0L 580–1000 ns, T1L 580–1000 ns, reset ≥280 µs | 800 kbps | **GRB** | 5V I/O variant |
| WS2812C | 8 | 3 (RGB) | 1-wire NRZ | T0H 220–380 ns, T1H 580–1000 ns, T0L 580–1000 ns, T1L 220–420 ns, reset ≥280 µs | 800 kbps | **GRB** | |
| WS2811 | 8 | 3 (RGB) | 1-wire NRZ | T0H 220–380 ns, T1H 580–1000 ns, T0L 580–1000 ns, T1L 580–1000 ns | 800 kbps | **RGB** | External driver IC |
| WS2811C | 8 | 3 (RGB) | 1-wire NRZ | T0H 220–380 ns, T1H 580–1000 ns, T0L 580–1000 ns, T1L 580–1000 ns | 800 kbps | **RGB** | |
| WS2813 | 8 | 3 (RGB) | 1-wire NRZ | T0H 300–450 ns, T1H 750–1000 ns, reset ≥280 µs | 800 kbps | **RGB** | Backup data pin |
| WS2813-RGBW | 8 | 4 (RGBW) | 1-wire NRZ | T0H 220–380 ns, T1H 580–1000 ns, reset ≥280 µs | 800 kbps | **GRBW** | 32-bit per pixel |
| WS2813B-V5W | 8 | 3 (RGB) | 1-wire NRZ | T0H 220–380 ns, T1H 580–1000 ns, reset ≥280 µs | 800 kbps | **GRB** | |
| WS2814 | 8 | 4 (RGBW) | 1-wire NRZ | T0H 220–380 ns, T1H 580–1000 ns, T0L 580–1000 ns, T1L 580–1000 ns | 800 kbps | **RGBW** | Dual data input |
| WS2814A | 8 | 4 (RGBW) | 1-wire NRZ | T0H 220–380 ns, T1H 580–1000 ns | 800 kbps | **WRGB** | Note: W byte first! |
| WS2814B | 8 | 4 (RGBW) | 1-wire NRZ | T0H 220–380 ns, T1H 580–1000 ns, T0L 580–1000 ns, T1L 580–1000 ns | 800 kbps | **RGBW** | |
| WS2814C | 8 | 4 (RGBW) | 1-wire NRZ | T0H 220–380 ns, T1H 580–1000 ns, T0L 580–1000 ns, T1L 580–1000 ns | 800 kbps | **RGBW** | |
| WS2815 | 8 | 3 (RGB) | 1-wire NRZ | T0H 220–380 ns, T1H 580–1600 ns, T0L 580–1600 ns, T1L 220–420 ns, reset ≥280 µs | 800 kbps | **GRB** | 12V, backup data pin |
| WS2815B | 8 | 3 (RGB) | 1-wire NRZ | T0H 220–380 ns, T1H 580–840 ns, T0L 900–5000 ns, T1L 600–5000 ns, reset ≥280 µs | 800 kbps | **GRB** | |
| WS2818 | 8 | 3 (RGB) | 1-wire NRZ | T0H 220–480 ns, T1H 750–2000 ns, T0L 750–2000 ns, T1L 220–480 ns | 800 kbps | **RGB** | 12V/24V, dual data input |
| WS2818B | 8 | 3 (RGB) | 1-wire NRZ | T0H 220–380 ns, T1H 580–1000 ns, T0L 580–1000 ns, T1L 580–1000 ns | 800 kbps | **RGB** | |
| WS2805 | 8 | 5 (RGBWW) | 1-wire NRZ | T0H 220–380 ns, T1H 580–1000 ns, T0L 580–1000 ns, T1L 580–1000 ns | 800 kbps | **RGBW1W2** | 40-bit per pixel, 5-channel |
| SK6812 | 8 | 3 (RGB) | 1-wire NRZ | TH+TL = 1.25 µs ± 600 ns | 800 kbps | **GRB** | |
| SK6812-White | 8 | 3 (WWW) | 1-wire NRZ | Same as SK6812 | 800 kbps | **WWW** | White-only variant |
| SK6813 | 8 | 3 (RGB) | 1-wire NRZ | TH+TL = 1.25 µs ± 600 ns | 800 kbps | **GRB** | Backup data pin |
| SK6813HV | 8 | 3 (RGB) | 1-wire NRZ | TH+TL = 1.25 µs ± 600 ns | 800 kbps | **GRB** | High-voltage variant |
| APA107 | 8 | 3 (RGB) | 1-wire NRZ | TH+TL = 1.25 µs ± 600 ns | 800 kbps | **GRB** | |
| HC2912C-2020 | 8 | 3 (RGB) | 1-wire NRZ | Same as WS2812x class | 800 kbps | **GRB** | 2020 package |
| SM16703P | 8 | 3 (RGB) | 1-wire NRZ | T0H 200–400 ns, T1H 800–1000 ns | 800 kbps | **RGB** | |
| SM16703SP | 8 | 3 (RGB) | 1-wire NRZ | T0H 200–400 ns | 800 kbps | **RGB** | |
| SM16704 | 8 | 4 (RGBW) | 1-wire NRZ | Same as SM16703 class | 800 kbps | **RGB(W)** | 32-bit; datasheet says "order of RGB" |
| SM16704PB | 8 | 3 (RGB) | 1-wire NRZ | T0H 200–400 ns, T1H 800–1000 ns | 800 kbps | **RGB** | |
| GS8208B | 8 | 3 (RGB) | 1-wire NRZ | TH+TL = 1.25 µs ± 600 ns | 800 kbps | **RGB** | Explicitly states RGB order |
| UCS1903 | 8 | 3 (RGB) | 1-wire NRZ | 800 kbps class | 800 kbps | **RGB** | OUTR/OUTG/OUTB |
| UCS2903 | 8 | 3 (RGB) | 1-wire NRZ | 800 kbps class | 800 kbps | **RGB** | External R_EXT |
| UCS2904 | 8 | 4 (RGBW) | 1-wire NRZ | 800 kbps class | 800 kbps | **RGBW** | |
| UCS5603 | 12 | 3 (RGB) | 1-wire NRZ | 800 kbps class | 800 kbps | **RGB** | 4096-level grayscale, bad-point resume |
| UCS7604 | 16 | 4 (RGBW) | 1-wire NRZ | 800 kbps class | 800 kbps | **RGBW** | 65536-level grayscale |
| UCS8903 | 16 | 3 (RGB) | 1-wire NRZ | 800 kbps class | 800 kbps | **RGB** | 48-bit per pixel (16-bit/ch) |
| UCS8904 | 16 | 4 (RGBW) | 1-wire NRZ | 800 kbps class | 800 kbps | **RGBW** | 64-bit per pixel (16-bit/ch) |
| UCS8904B | 16 | 4 (RGBW) | 1-wire NRZ | 800 kbps class | 800 kbps | **RGBW** | 64-bit per pixel (16-bit/ch) |
| UCS9812 | 16+4 | 12 (4×RGB) | 1-wire NRZ | 1100 kbps class | 1100 kbps | **RGB** | 240-bit frame; 16-bit gray + 4-bit current per ch |
| TM1812 | 8 | 12 (4×RGB) | 1-wire NRZ | T0H 350–450 ns, T1H 700–1000 ns | 800 kbps | **RGB** | 96-bit per chip (4 LED groups) |

## One-Wire NRZ — 800 kHz Class, Inverted (Idle-High)

| Chip | Bit Depth | Channels | Wires | Signal Timing (typical) | Data Rate | Default Channel Order | Notes |
|------|-----------|----------|-------|-------------------------|-----------|----------------------|-------|
| TM1814 | 8 | 4 (WRGB) | 1-wire NRZ **(inverted)** | T0L 310–410 ns, T1L 650–1000 ns, reset ≥200 µs | 800 kHz | **WRGB** | Inverted NRZ; W byte first |
| TM1903 | 8 | 3 (RGB) | 1-wire NRZ **(inverted)** | T0H 310–410 ns, T1H 650–1000 ns, reset ≥50 µs | 800 kHz | **RGB** | Same timing family as TM1814 |
| TM1914 | 8 | 3 (RGB) | 1-wire NRZ **(inverted)** | T0L 310–410 ns, T1L 650–1000 ns, reset ≥200 µs | 800 kHz | **RGB** | Dual-channel input (DI/FDI) |
| TM1934 | 8 | 3 (RGB) | 1-wire NRZ **(inverted)** | T0H 310–410 ns, T1H 650–1000 ns, reset ≥200 µs | 800 kHz | **RGB** | Dual-channel input (DI/FDI) |
| TM1829 | 8 | 3 (RGB) | 1-wire NRZ **(inverted)** | T0L 150–450 ns, T1L 600–1000 ns | 800 kHz | **RGB** | Constant current adjustable |
| LB1908 | 8 | 3 (RGB) | 1-wire NRZ | T0H 250–350 ns, T1H 700–900 ns, reset ≥200 µs | 800 kHz | **RGB** | 12V, dual data input |

## One-Wire NRZ — 400 kHz Class

| Chip | Bit Depth | Channels | Wires | Signal Timing (typical) | Data Rate | Default Channel Order | Notes |
|------|-----------|----------|-------|-------------------------|-----------|----------------------|-------|
| TM1803 | 8 | 3 (RGB) | 1-wire NRZ | T0H ≈680 ns, T1H ≈1360 ns, T0L ≈1360 ns, T1L ≈680 ns | 400 kbps | **RGB** | |
| TM1804 | 8 | 3 (RGB) | 1-wire NRZ | T0H 350–900 ns, T1H 700–2000 ns (has slow/fast modes) | 400 kbps | **RGB** | Dual-speed datasheet |
| TM1809 | 8 | 3 (RGB) | 1-wire NRZ | T0H 250–750 ns, T1H 530–1350 ns (variable, slow/fast) | 400 kbps | **RGB** | Dual-speed datasheet |

## One-Wire e-RZ (Extended Return-to-Zero) — 800 kHz

| Chip | Bit Depth | Channels | Wires | Signal Timing (typical) | Data Rate | Default Channel Order | Notes |
|------|-----------|----------|-------|-------------------------|-----------|----------------------|-------|
| CS8812 | 8 (12-bit Γ) | 3 (RGB) | 1-wire **(e-RZ)** | e-RZ encoding, reset ≥300 µs | 800 kHz | **RGB** | 12V; built-in 12-bit gamma; dual SDI |
| GS8206 | 8 (12-bit Γ) | 3 (RGB) | 1-wire **(e-RZ)** | e-RZ encoding, reset ≥300 µs | 800 kHz | **RGB** | 5–24V; built-in 12-bit gamma; dual SDI |
| GS8208 | 8 (12-bit Γ) | 3 (RGB) | 1-wire **(e-RZ)** | e-RZ encoding, reset ≥300 µs | 800 kHz | **RGB** | 12V; built-in 12-bit gamma; dual SDI |

## One-Wire — Special / 6-Channel

| Chip | Bit Depth | Channels | Wires | Signal Timing (typical) | Data Rate | Default Channel Order | Notes |
|------|-----------|----------|-------|-------------------------|-----------|----------------------|-------|
| FW1906 | 8 | 6 (2×RGB) | 1-wire NRZ | T0H 320–400 ns, T1H 640–800 ns, cycle ≈1250 ns | 800 kHz | **RGB** (×2) | 24V; 6ch drives 2 LED groups |

## Two-Wire SPI (Clock + Data)

| Chip | Bit Depth | Channels | Wires | Signal Timing | Data Rate | Default Channel Order | Notes |
|------|-----------|----------|-------|---------------|-----------|----------------------|-------|
| APA102 | 8 + 5-bit brightness | 3 (RGB) | 2-wire SPI | CLK ≥ 30 ns high/low | Up to ~30 MHz | **RGB** | 32-bit frame: 111+5 brightness + 8B + 8G + 8R (BGR on wire) |
| APA102-1515 | 8 + 5-bit brightness | 3 (RGB) | 2-wire SPI | CLK ≥ 30 ns high/low | Up to ~30 MHz | **RGB** | 1515 package |
| APA102-2020 | 8 + 5-bit brightness | 3 (RGB) | 2-wire SPI | CLK ≥ 30 ns high/low | Up to ~30 MHz | **RGB** | 2020 package |
| HD107S | 8 + 5-bit brightness | 3 (RGB) | 2-wire SPI | CLK ≥ 30 ns | Up to 30 MHz | **RGB** | APA102-compatible |
| HD108 | 16 + 5-bit brightness | 3 (RGB) | 2-wire SPI | CLK ≥ 30 ns | Up to 32 MHz | **RGB** | 65536-level grayscale per ch |
| SM16716 | 8 | 3 (RGB) | 2-wire (DCLK+DIN) | CLK up to 30 MHz | Up to 30 MHz | **RGB** | 3ch open-drain constant current |
| LPD6803 | 5 | 3 (RGB) | 2-wire SPI | CLK up to 24 MHz | Up to 24 MHz | **RGB** | 15-bit color (5-bit/ch), reversed-gamma |
| WS2801 | 8 | 3 (RGB) | 2-wire SPI | 8 ns typical delay | Up to 25 MHz | **RGB** | |
| WS2801S | 8 | 3 (RGB) | 2-wire SPI | 8 ns typical delay | Up to 25 MHz | **RGB** | Variant of WS2801 |

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
