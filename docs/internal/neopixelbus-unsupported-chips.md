# NeoPixelBus Chips Not Ported to LumaWave

> **Status:** Reference — documents NeoPixelBus-supported chips/features absent from LumaWave, with reasoning.  
> **NeoPixelBus version:** 2.8.4  
> **LumaWave revision:** HEAD as of 2026-03-02

---

## One-Wire Chips With Per-Pixel Settings (SM168x One-Wire Family)

NeoPixelBus supports several SM168x one-wire variants that encode per-pixel gain/current settings as trailing bytes appended to the NRZ data stream. These are **distinct products** from the SPI-clocked SM168x chips that LumaWave already supports via `Sm168xProtocol`.

| Chip | NeoPixelBus Feature | Channels | Per-Pixel Settings | Notes |
|------|---------------------|----------|--------------------|-------|
| SM16803PB | `NeoRgbSm16803pbFeature` | RGB (3ch) | 4-bit gain ×3 (1.8–19 mA), 2 settings bytes | Internal current reference |
| SM16823E | `NeoRgbSm16823eFeature` | RGB (3ch) | 4-bit gain ×3 (60–350 mA, external R), 2 settings bytes | External current reference |
| SM16804EB | `NeoRgbwSm16804ebFeature` | RGBW (4ch) | 4-bit gain ×4 (1.8–19 mA), 2 settings bytes | Internal current reference |
| SM16824E | `NeoRgbwSm16824eFeature` | RGBW (4ch) | 4-bit gain ×4 (60–350 mA, external R), 2 settings bytes | External current reference |
| SM16825E | `NeoRgbwcSm16825eFeature` / `NeoRgbcwSm16825eFeature` | RGBWC (5ch, 16-bit) | 5-bit gain ×5 (10.2–310 mA), 4 settings bytes | 16-bit precision |

**Why not ported:**

LumaWave's `Ws2812xProtocol` handles standard one-wire NRZ encoding (3–5 channel, 8/16-bit) but does not support per-pixel trailing settings bytes. The existing `Sm168xProtocol` (SPI-clocked) handles gain settings in an SPI framing context, which is a different wire format.

Porting the one-wire SM168x variants would require either:
1. A dedicated `Sm168xOneWireProtocol` that appends per-pixel gain bytes after the color data, or
2. Extending `Ws2812xProtocol` with an optional settings-suffix mechanism.

These chips are uncommon in hobby use and the per-pixel settings pattern is only shared within this family. Priority is low unless demand materialises.

---

## TM1829

| Chip | NeoPixelBus Support | Channels | Signal | Timing |
|------|---------------------|----------|--------|--------|
| TM1829 | `NeoTm1829Method` (all platforms), Speed class: `NeoBitsSpeedTm1829` | RGB (3ch) | **Inverted** (idle-high) | 300/800/800/300 ns, 200 µs reset |

**LumaWave status:** The `OneWireTiming::Tm1829` timing profile exists in `OneWireTiming.h`. The chip **can** be driven today by manually specifying:
- `Ws2812xProtocol` with `timing::Tm1829`
- A transport configured with `invert = true`

**Why no convenience descriptor:**

The TM1829 is functionally equivalent to a generic inverted one-wire RGB chip. A descriptor alias (`Tm1829`) would be a trivial addition (inheriting from `Ws2812x` with `timing::Tm1829` and `ChannelOrder::RGB`). However, the TM1829 is a legacy chip with minimal ongoing market presence. Adding the descriptor is straightforward and can be done on demand.

---

## Intertek

| Chip | NeoPixelBus Support | Channels | Signal | Timing |
|------|---------------------|----------|--------|--------|
| Intertek | `NeoEspBitBangSpeedIntertek800Kbps` (ESP bit-bang only) | RGB (3ch) | Normal | 800 kbps, **12,470 µs reset** (~12.5 ms) |

**Why not ported:**

- Extremely niche chip with an aberrant 12.5 ms reset time (40× longer than WS2812x).
- In NeoPixelBus, only supported via ESP bit-bang — not available on RMT, I2S, PIO, or any DMA path.
- The long reset time is incompatible with most real-time animation use cases (caps effective frame rate to ~80 fps for transport idle alone).
- Could theoretically work with LumaWave's `OneWireTiming{...}` system if someone defined the timing, but there's no known user demand.

---

## MBI6033

| Chip | NeoPixelBus Support | Channels | Bit Depth | Interface |
|------|---------------------|----------|-----------|-----------|
| MBI6033 | `Mbi6033Method` (bit-bang only) | 12ch PWM (4 RGB pixels / chip) | 16-bit | Clk + Data (software bit-bang) |

**Why not ported:**

- Non-standard SPI-like protocol with a custom reset sequence (clock toggle pattern).
- In NeoPixelBus, only available via `TwoWireBitBangImple` — no hardware SPI support due to the non-standard clock-toggle reset.
- Very low market adoption; the TLC59711 serves the same niche (12-channel 16-bit PWM) with standard SPI and is already supported by LumaWave.
- Porting would require either a custom transport or a protocol that emits the reset clock-toggle, adding complexity for a near-zero audience.

---

## DMX512

| Protocol | NeoPixelBus Support | Interface | Platform |
|----------|---------------------|-----------|----------|
| DMX512 | `NeoEsp8266Dmx512Method` | Serial (250 Kbps, Break/MAB framing) | **ESP8266 only** (I2S-based DMX) |

**Why not ported:**

- DMX512 is a fundamentally different protocol class — it's a serial bus protocol with Break/MAB framing, not a pixel LED protocol.
- In NeoPixelBus, DMX512 is only supported on ESP8266 via I2S repurposed as a DMX transmitter — extremely platform-specific.
- LumaWave's architecture separates protocol from transport, which could theoretically support DMX512 as a protocol + UART transport combination. However, DMX512 is better served by dedicated DMX libraries (e.g., `esp_dmx`) that handle the full DMX specification including receiving, RDM, etc.
- Not porting is a deliberate scope decision: LumaWave targets LED pixel protocols, not lighting control bus protocols.

---

## WS2821

| Protocol | NeoPixelBus Support | Interface | Platform |
|----------|---------------------|-----------|----------|
| WS2821 | `NeoEsp8266Ws2821Method` | Serial (750 Kbps, Break/MAB framing) | **ESP8266 only** (I2S-based) |

**Why not ported:**

- WS2821 is a DMX-like serial protocol at 750 Kbps with Break/MAB inter-packet gaps — essentially a proprietary variant of DMX512.
- Same platform restriction as DMX512: ESP8266 I2S only in NeoPixelBus.
- Same scope rationale as DMX512 — this is a serial bus protocol, not a pixel LED NRZ or SPI protocol.
- Near-zero market presence outside of professional DMX-controlled installations.

---

## 7-Segment Display Features

| Feature | NeoPixelBus Support | Notes |
|---------|---------------------|-------|
| `NeoAbcdefgpsSegmentFeature` | `NeoPixelSegmentBus` wrapper + segment feature class | Maps `SetString()` → segment bitmasks |
| `NeoBacedfpgsSegmentFeature` | Alternate wiring order | |

**Why not ported:**

- 7-segment display support is an application-layer concern, not a protocol or transport issue.
- The segment bitmask mapping is trivially implementable in user code or a thin utility.
- LumaWave's scope is the pixel bus pipeline (protocol/transport/shader/bus); display-type-specific helpers are left to application code.

---

## Platforms Not Supported

These are platform/method gaps, not chip gaps. NeoPixelBus supports these platforms via dedicated ASM or peripheral drivers:

| Platform | NeoPixelBus Method Type | Why Not in LumaWave |
|----------|------------------------|---------------------|
| **AVR (ATmega, ATtiny)** | Cycle-counted inline ASM bit-bang | AVR lacks C++17 STL support. LumaWave requires C++17 and STL (`std::vector`, `std::unique_ptr`, `std::tuple`, etc.). AVR's severe RAM constraints (2–8 KB) are also incompatible with LumaWave's buffer architecture (minimum ~7 bytes/pixel). |
| **MegaAVR** | Same as AVR | Same as AVR. |
| **ARM (Teensy, SAMD, STM32)** | ASM bit-bang (`NeoArmMethod`) | No hardware-accelerated path — only bit-bang. LumaWave's architecture assumes DMA/peripheral-based transports. Bit-bang transports are inherently timing-sensitive and require platform-specific ASM, conflicting with the virtual-first / portable transport model. Could be added as platform-specific transport implementations if demand warrants. |
| **NRF52840** | PWM DMA (3-4 channels) | Not yet targeted. The NRF52840's PWM+DMA peripheral is capable and would map well to LumaWave's transport model. This is a reasonable future addition — lower priority than the primary ESP32/ESP8266/RP2040 targets. |

### Parallel / Multi-Channel Output

| Feature | NeoPixelBus Support | LumaWave Status |
|---------|---------------------|-----------------|
| ESP32 I2S 8-channel parallel | `NeoEsp32I2sX8...Method` | Not yet implemented |
| ESP32-S3 LCD 8/16-channel parallel | `NeoEsp32LcdX8...Method` / `X16` | Not yet implemented |
| RP2040 PIO ×4 parallel | `NeoRp2040x4...Method` | Not yet implemented |

**Why not yet ported:**

Parallel output requires a single transport to multiplex multiple strands' data simultaneously into one DMA buffer. LumaWave's current `ITransport` contract is single-strand (`transmitBytes()` sends one contiguous byte stream). Supporting parallel output would require either:
1. A multi-strand transport variant that accepts interleaved or parallel byte streams, or
2. A compositor that pre-interleaves strand data and feeds a single DMA transport.

This is an architectural decision that needs design work and is tracked as a future enhancement.

---

## Summary Table

| Chip / Feature | Category | Priority | Effort | Blocked By |
|----------------|----------|----------|--------|------------|
| SM168x one-wire (PB/E variants) | Protocol gap | Low | Medium — new protocol class or Ws2812x extension | Design decision on settings-suffix pattern |
| TM1829 descriptor | Convenience alias | Trivial | Trivial — add descriptor struct | Nothing |
| Intertek | Niche chip | Very low | Low — just a timing profile | No demand |
| MBI6033 | Niche SPI chip | Very low | Medium — custom reset pattern | No demand |
| DMX512 / WS2821 | Scope exclusion | N/A | N/A | Deliberate scope boundary |
| 7-Segment features | Application-layer | N/A | N/A | Out of scope |
| AVR / MegaAVR platform | Platform gap | N/A | Very high — C++17/STL incompatible | Fundamental C++ standard requirement |
| ARM bit-bang platform | Platform gap | Low | Medium — per-platform ASM transport | Architecture preference for DMA paths |
| NRF52840 platform | Platform gap | Medium | Medium — PWM DMA transport | Not yet prioritised |
| Parallel multi-channel | Transport gap | Medium-High | High — transport architecture extension | `ITransport` contract design |
