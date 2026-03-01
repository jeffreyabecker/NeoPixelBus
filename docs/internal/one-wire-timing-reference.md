# OneWireTiming Reference

Status: reference
Source: `src/transports/OneWireTiming.h`

This document describes the `OneWireTiming` struct, its preset constants, derived helpers,
and the relationship between code-level timing values, datasheet specifications, and the
encoded-bit-pattern strategy used by `OneWireWrapper`.

---

## 1. NRZ Signalling Overview

One-wire LED chips (WS2812x, SK6812, TM1814, etc.) use an NRZ (non-return-to-zero)
self-clocking protocol on a single data line. Each data bit is a fixed-length pulse
pair — a high interval followed by a low interval — whose ratio determines whether the
bit is a zero or a one:

```
  ┌──── T0H ────┐
  │              │
──┘              └──── T0L ────── idle    ← zero bit

  ┌───────── T1H ──────────┐
  │                        │
──┘                        └── T1L ──── idle    ← one bit
```

The **bit period** is constant: `T0H + T0L ≈ T1H + T1L`.
Zero bits have a short high / long low; one bits have a long high / short low.

After all pixel data has been clocked out, the line is held low (or high, for inverted
chips) for a **reset interval** (`resetNs`) to latch the frame.

> **Signal inversion** is *not* part of `OneWireTiming`. Inverted-NRZ chips
> (TM1814, TM1914, TM1829) idle high and encode data in the low-time instead,
> but the structural timing values are recorded identically. Inversion is a
> platform/transport concern.

---

## 2. The `OneWireTiming` Struct

```cpp
struct OneWireTiming
{
    uint32_t t0hNs;   // T0H — high time for a zero bit (nanoseconds)
    uint32_t t0lNs;   // T0L — low  time for a zero bit (nanoseconds)
    uint32_t t1hNs;   // T1H — high time for a one  bit (nanoseconds)
    uint32_t t1lNs;   // T1L — low  time for a one  bit (nanoseconds)
    uint32_t resetNs; // reset / latch interval (nanoseconds)
};
```

All fields are `uint32_t`. Nano-second values are chosen to give clean arithmetic for
common 800 kHz and 400 kHz bit rates. The `resetNs` field is in **nanoseconds**.
Typical reset intervals are represented as 40,000–500,000 ns.

### Derived Helpers

| Method | Returns | Formula |
|--------|---------|---------|
| `bitPeriodNs()` | `uint32_t` | `t0hNs + t0lNs` |
| `bitRateHz()` | `float` | `1 × 10⁹ / bitPeriodNs()` |
| `bitPattern()` | `EncodedClockDataBitPattern` | 3-step or 4-step (see §4) |

---

## 3. Preset Timing Constants

### 3.1 Primary Presets

Each row is one `inline constexpr OneWireTiming` value.
"Bit period" and "Bit rate" are derived from `t0hNs + t0lNs`.

| Constant | T0H (ns) | T0L (ns) | T1H (ns) | T1L (ns) | Reset (µs) | Bit Period | Bit Rate | Encoding | Chips Covered |
|----------|----------|----------|----------|----------|------------|------------|----------|----------|---------------|
| `Ws2812x` | 400 | 850 | 800 | 450 | 300 | 1250 ns | 800 kHz | 3-step | WS2812B, WS2812C, WS2812B-V5W, WS2815, WS2815B, WS2818B, SK6812, SK6813, APA107, HC2912C, GS8208B |
| `Ws2811` | 500 | 2000 | 1200 | 1300 | 50 | 2500 ns | 400 kHz | 3-step | WS2811, WS2811C, TM1803, TM1804, TM1809 |
| `Ws2805` | 300 | 790 | 790 | 300 | 300 | 1090 ns | ~918 kHz | 4-step | WS2805 (RGBWW), WS2814, WS2814A–C |
| `Sk6812` | 400 | 850 | 800 | 450 | 80 | 1250 ns | 800 kHz | 3-step | SK6812, SK6812-White, SK6813, SK6813HV, LC8812 |
| `Tm1814` | 360 | 720 | 720 | 360 | 200 | 1080 ns | ~926 kHz | 3-step | TM1814 (inverted, WRGB), TM1903 |
| `Tm1914` | 360 | 720 | 720 | 360 | 200 | 1080 ns | ~926 kHz | 3-step | TM1914, TM1934 (inverted) |
| `Tm1829` | 300 | 800 | 800 | 300 | 500 | 1100 ns | ~909 kHz | 4-step | TM1829 (inverted) |
| `Apa106` | 350 | 1360 | 1360 | 350 | 50 | 1710 ns | ~585 kHz | 4-step | APA106 |
| `Tx1812` | 300 | 600 | 600 | 300 | 80 | 900 ns | ~1111 kHz | 3-step | TX1812 |
| `Gs1903` | 300 | 900 | 900 | 300 | 40 | 1200 ns | ~833 kHz | 4-step | GS1903 |
| `Generic800` | 400 | 850 | 800 | 450 | 50 | 1250 ns | 800 kHz | 3-step | General 800 kHz fallback |
| `Generic400` | 500 | 2000 | 1200 | 1300 | 50 | 2500 ns | 400 kHz | 3-step | General 400 kHz fallback |

### 3.2 Aliases

These are `= SomePrimary` assignments — identical timing under a different chip name.

| Alias | Points To | Rationale |
|-------|-----------|-----------|
| `Ws2816` | `Ws2812x` | WS2816 timing is identical to WS2812x (800 kHz, same pulse widths) |
| `Ws2813` | `Ws2812x` | WS2813 timing is identical to WS2812x; backup-pin feature is protocol-level |
| `Ws2814` | `Ws2805` | WS2814 uses the same ~918 kHz timing as WS2805 |
| `Lc8812` | `Sk6812` | LC8812 is timing-compatible with SK6812 |

### 3.3 `timing::` Namespace Shortcuts

Every preset and alias is re-exported as a `constexpr` reference in `lw::timing::`:

```cpp
lw::timing::Ws2812x   // → lw::OneWireTiming::Ws2812x
lw::timing::Tm1814    // → lw::OneWireTiming::Tm1814
// etc.
```

This allows concise usage at call sites:

```cpp
auto bus = makeBus<RgbColor>(pixelCount, timing::Ws2812x, transportSettings);
```

---

## 4. Encoded Bit Pattern (3-Step vs. 4-Step)

One-wire data cannot be transmitted directly over clocked SPI/PIO transports. The
`OneWireWrapper` re-encodes each data bit into **SPI clock cycles** that approximate the
NRZ waveform:

- **3-step** (most common): each data bit → 3 SPI bits. A zero bit becomes `100`, a one
  bit becomes `110`. The SPI clock runs at 3× the NRZ bit rate.
- **4-step**: each data bit → 4 SPI bits. A zero bit becomes `1000`, a one bit becomes
  `1110`. Used when T1H is significantly wider than T0H.

### Selection Rule

`bitPattern()` decides automatically:

```cpp
constexpr EncodedClockDataBitPattern bitPattern() const
{
    bool fourStep = (2 * t1hNs) > (3 * t0hNs);
    return fourStep ? FourStep : ThreeStep;
}
```

The heuristic: if `T1H > 1.5 × T0H`, the timing skew is large enough that 4 SPI bits
per data bit provide a better waveform match.

### Which Presets Get Which Pattern?

| Pattern | Condition | Presets |
|---------|-----------|---------|
| **3-step** | `2×T1H ≤ 3×T0H` | Ws2812x, Ws2811, Sk6812, Tm1814, Tm1914, Tx1812, Generic800, Generic400 |
| **4-step** | `2×T1H > 3×T0H` | Ws2805, Tm1829, Apa106, Gs1903 |

### Transport Clock Calculation

For `OneWireWrapper`, the encoded SPI clock rate is:

```
R_clk = bitRateHz() × bitPattern()

Example — Ws2812x:
  800 kHz × 3 = 2.4 MHz encoded clock

Example — Ws2805:
  ~918 kHz × 4 = ~3.67 MHz encoded clock
```

See also: `docs/internal/bitrate-calculation-guide.md`.

---

## 5. Datasheet Cross-Reference

The table below maps preset constants to the datasheet-specified acceptable timing
ranges. The code values are **typical nominal midpoints** chosen to work reliably across
the acceptance window.

### Ws2812x (used for WS2812B, WS2812C, SK6812, etc.)

| Parameter | Datasheet Range | Code Value |
|-----------|----------------|------------|
| T0H | 220–380 ns | **400 ns** |
| T0L | 580–1600 ns | **850 ns** |
| T1H | 580–1600 ns | **800 ns** |
| T1L | 220–420 ns | **450 ns** |
| Reset | ≥ 280 µs | **300 µs** |

> Code values are slightly above the datasheet midpoints for reliable margin.
> The WS2812B datasheet specifies TH+TL = 1.25 µs ± 600 ns; the code's 1250 ns
> period is dead-center.

### Ws2811 (WS2811, also covers 400 kHz class)

| Parameter | Datasheet Range | Code Value |
|-----------|----------------|------------|
| T0H | 220–380 ns (fast) / 500–900 ns (slow) | **500 ns** |
| T0L | 580–1000 ns / 1600–2000 ns | **2000 ns** |
| T1H | 580–1000 ns / 1200–2000 ns | **1200 ns** |
| T1L | 580–1000 ns / 1300–2000 ns | **1300 ns** |
| Reset | ≥ 50 µs | **50 µs** |

> WS2811 supports both 400 kHz and 800 kHz modes; the code preset uses the 400 kHz
> (slow) set that is the chip's default when the speed-select pin is not tied high.

### Ws2805 (WS2805, WS2814)

| Parameter | Datasheet Range | Code Value |
|-----------|----------------|------------|
| T0H | 220–380 ns | **300 ns** |
| T0L | 580–1000 ns | **790 ns** |
| T1H | 580–1000 ns | **790 ns** |
| T1L | 580–1000 ns | **300 ns** |
| Reset | ≥ 280 µs | **300 µs** |

> The near-equal T0L/T1H with a 4-step encoding gives the sharpest zero/one
> differentiation for these chips.

### Sk6812

| Parameter | Datasheet Range | Code Value |
|-----------|----------------|------------|
| T0H | — | **400 ns** |
| T0L | — | **850 ns** |
| T1H | — | **800 ns** |
| T1L | — | **450 ns** |
| Reset | ≥ 80 µs | **80 µs** |

> Timing is identical to Ws2812x; only the reset interval differs (80 µs vs 300 µs).
> The SK6812 datasheet specifies TH+TL = 1.25 µs ± 600 ns.

### Tm1814 / Tm1914 (Inverted NRZ)

| Parameter | Datasheet Range | Code Value |
|-----------|----------------|------------|
| T0L | 310–410 ns | **360 ns** |
| T1L | 650–1000 ns | **720 ns** |
| T0H | (derived) | **720 ns** |
| T1H | (derived) | **360 ns** |
| Reset | ≥ 200 µs | **200 µs** |

> **Inverted encoding**: the TM1814/TM1914 datasheets specify low-time durations
> (T0L, T1L) because the line idles high. In the `OneWireTiming` struct, the values
> are stored using the *same field semantics* as non-inverted chips — the transport
> layer handles the actual signal inversion. The datasheet's T0L maps to `t0hNs` in
> the struct (both represent the "short pulse" for a zero bit).

### Tm1829 (Inverted NRZ)

| Parameter | Datasheet Range | Code Value |
|-----------|----------------|------------|
| T0L | 150–450 ns | **300 ns** |
| T1L | 600–1000 ns | **800 ns** |
| T0H | (derived) | **800 ns** |
| T1H | (derived) | **300 ns** |
| Reset | ≥ 500 µs | **500 µs** |

### Apa106

| Parameter | Datasheet Range | Code Value |
|-----------|----------------|------------|
| T0H | ~350 ns | **350 ns** |
| T0L | ~1360 ns | **1360 ns** |
| T1H | ~1360 ns | **1360 ns** |
| T1L | ~350 ns | **350 ns** |
| Reset | ≥ 50 µs | **50 µs** |

> APA106 has a notably long bit period (1710 ns → ~585 kHz) and strong T1H/T0H skew,
> giving it 4-step encoding.

### Tx1812

| Parameter | Datasheet Range | Code Value |
|-----------|----------------|------------|
| T0H | ~300 ns | **300 ns** |
| T0L | ~600 ns | **600 ns** |
| T1H | ~600 ns | **600 ns** |
| T1L | ~300 ns | **300 ns** |
| Reset | ≥ 80 µs | **80 µs** |

> TX1812 runs at ~1.1 MHz (900 ns period), the fastest preset in the table.

### Gs1903

| Parameter | Datasheet Range | Code Value |
|-----------|----------------|------------|
| T0H | ~300 ns | **300 ns** |
| T0L | ~900 ns | **900 ns** |
| T1H | ~900 ns | **900 ns** |
| T1L | ~300 ns | **300 ns** |
| Reset | ≥ 40 µs | **40 µs** |

---

## 6. Chip-to-Preset Mapping Summary

Quick lookup: given a chip name, which `OneWireTiming` preset to use.

| Chip(s) | Preset | Rate | Order | Ch |
|---------|--------|------|-------|----|
| WS2812B, WS2812B-V5W, WS2812C | `Ws2812x` | 800 kHz | GRB | 3 |
| WS2816 | `Ws2816` (= Ws2812x) | 800 kHz | GRB | 3 |
| WS2813, WS2813B-V5W | `Ws2813` (= Ws2812x) | 800 kHz | RGB | 3 |
| WS2813-RGBW | `Ws2813` (= Ws2812x) | 800 kHz | GRBW | 4 |
| WS2815, WS2815B | `Ws2812x` | 800 kHz | GRB | 3 |
| WS2818 | `Ws2811` variant¹ | 800 kHz | RGB | 3 |
| WS2818B | `Ws2812x` | 800 kHz | RGB | 3 |
| WS2811, WS2811C | `Ws2811` | 400 kHz | RGB | 3 |
| WS2805 | `Ws2805` | ~918 kHz | RGBW1W2 | 5 |
| WS2814, WS2814B, WS2814C | `Ws2814` (= Ws2805) | ~918 kHz | RGBW | 4 |
| WS2814A | `Ws2814` (= Ws2805) | ~918 kHz | WRGB | 4 |
| SK6812, SK6812-White | `Sk6812` | 800 kHz | GRB | 3 |
| SK6813, SK6813HV | `Sk6812` | 800 kHz | GRB | 3 |
| LC8812 | `Lc8812` (= Sk6812) | 800 kHz | GRB | 3 |
| APA107 | `Ws2812x`² | 800 kHz | GRB | 3 |
| HC2912C-2020 | `Ws2812x` | 800 kHz | GRB | 3 |
| TM1814 | `Tm1814` | ~926 kHz | WRGB | 4 |
| TM1903 | `Tm1814`³ | ~926 kHz | RGB | 3 |
| TM1914, TM1934 | `Tm1914` | ~926 kHz | RGB | 3 |
| TM1829 | `Tm1829` | ~909 kHz | RGB | 3 |
| TM1812 | `Tx1812`⁴ | ~1111 kHz | RGB | 12 |
| TM1803, TM1804, TM1809 | `Ws2811` / `Generic400` | 400 kHz | RGB | 3 |
| APA106 | `Apa106` | ~585 kHz | RGB | 3 |
| TX1812 | `Tx1812` | ~1111 kHz | RGB | 3 |
| GS1903 | `Gs1903` | ~833 kHz | RGB | 3 |
| SM16703P, SM16703SP | `Ws2812x` | 800 kHz | RGB | 3 |
| SM16704, SM16704PB | `Ws2812x` | 800 kHz | RGB | 3/4 |
| GS8208B | `Ws2812x` | 800 kHz | RGB | 3 |
| UCS1903, UCS2903 | `Ws2812x` | 800 kHz | RGB | 3 |
| UCS2904 | `Ws2812x` | 800 kHz | RGBW | 4 |
| UCS5603 (12-bit) | `Ws2812x` | 800 kHz | RGB | 3 |
| UCS7604 (16-bit) | `Ws2812x` | 800 kHz | RGBW | 4 |
| UCS8903 (16-bit) | `Ws2812x` | 800 kHz | RGB | 3 |
| UCS8904/B (16-bit) | `Ws2812x` | 800 kHz | RGBW | 4 |
| UCS9812 | *custom*⁵ | 1100 kHz | RGB | 12 |
| FW1906 | `Ws2812x` | 800 kHz | RGB | 6 |
| LB1908 | `Tm1814`³ | 800 kHz | RGB | 3 |

**Notes:**
1. WS2818 has wider timing tolerance (T0H up to 480 ns), but Ws2811 800 kHz mode or
   Ws2812x both work.
2. APA107 is a one-wire chip (not SPI like APA102) with WS2812x-compatible timing.
3. TM1903 and LB1908 share the TM1814 timing family (~926 kHz, 360/720 ns pulses).
4. TM1812 is a 12-channel driver; timing is close to TX1812.
5. UCS9812 runs at ~1100 kHz — not yet a dedicated preset.

---

## 7. Chips NOT Covered by OneWireTiming

These chips from the datasheet collection use **two-wire SPI** (clock + data) or **e-RZ
encoding** and are handled by separate protocol/transport types, not `OneWireTiming`:

### Two-Wire SPI Chips

| Chip | Bit Depth | Max Clock | Protocol |
|------|-----------|-----------|----------|
| APA102 / APA102-1515 / APA102-2020 | 8 + 5-bit brightness | ~30 MHz | DotStar |
| HD107S | 8 + 5-bit brightness | 30 MHz | DotStar-compatible |
| HD108 | 16 + 5-bit brightness | 32 MHz | HD108 |
| SM16716 | 8 | 30 MHz | SM16716 |
| LPD6803 | 5 | 24 MHz | LPD6803 |
| WS2801 / WS2801S | 8 | 25 MHz | WS2801 |
| P9813 | 8 | — | P9813 |
| LPD8806 | 7 | — | LPD8806 |

### e-RZ Encoded Chips

| Chip | Bit Depth | Data Rate | Description |
|------|-----------|-----------|-------------|
| CS8812 | 8 (12-bit Γ) | 800 kHz | 12V; extended return-to-zero code |
| GS8206 | 8 (12-bit Γ) | 800 kHz | 5–24V; same e-RZ encoding as CS8812 |
| GS8208 | 8 (12-bit Γ) | 800 kHz | 12V; same e-RZ encoding as CS8812 |

---

## 8. Usage in the Codebase

### Protocol Settings

Protocols that carry timing embed a `OneWireTiming` field in their settings struct:

```cpp
// Ws2812xProtocol
struct Ws2812xProtocolSettings {
    OneWireTiming timing = timing::Ws2812x;
};

// Tm1814Protocol
struct Tm1814ProtocolSettings {
    OneWireTiming timing = timing::Tm1814;
    Tm1814CurrentSettings current{};
    const char* channelOrder = "WRGB";
};
```

### OneWireWrapper

`OneWireWrapper` reads `timing` from the transport config to:

1. Select 3-step or 4-step encoding via `bitPattern()`.
2. Compute the required transport clock rate via `bitRateHz() × bitPattern()`.
3. Encode pixel data bytes into the SPI-compatible bit stream.

### Factory (`makeBus`)

The `makeBus()` factory propagates timing from the call site into both the
`OneWireWrapperSettings` and the protocol settings:

```cpp
auto bus = makeBus<RgbColor>(count, timing::Ws2812x, transportSettings);
```

`MakeBus.h` uses `assignProtocolTimingIfPresent()` to write the timing value into
protocol settings structs that expose a `.timing` member.
