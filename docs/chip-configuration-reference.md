# Chip Configuration Reference

This document covers common LED chip configuration choices for NeoPixelBus, using aliases and features available in `src/original`.

> Notes
>
> - Voltage and segment grouping are hardware concerns and are **not** configured in software.
> - In grouped strips (for example, 12V/24V variants), one logical pixel in software can represent multiple physical LEDs.
> - Color order varies by vendor and batch; choose the matching `*Feature` type for your strip.

## Recommended configuration matrix

| Type | Voltage | Comments | NeoPixelBus configuration guidance |
|---|---|---|---|
| SK6812 | 5 V / 12 V | RGBW | Use `NeoPixelBus<NeoRgbwFeature, NeoSk6812Method>` (or `NeoGrbwFeature` / `NeoBrgwFeature` to match your order). |
| WS2811 | 5 V | usually found in IP68 sealed 12mm pixel strings | Use `NeoPixelBus<NeoGrbFeature, NeoWs2811Method>`. |
| WS2812B | 5 V |  | Use `NeoPixelBus<NeoGrbFeature, NeoWs2812xMethod>` (or `NeoWs2812Method` for explicit 800 Kbps alias). |
| WS2813 | 5 V | has a backup data line | Use `NeoPixelBus<NeoGrbFeature, NeoWs2813Method>`. |
| APA102 | 5 V | using 2 data pins, Clock and Data | Use DotStar classes, for example `NeoPixelBus<DotStarBgrFeature, DotStarSpiMethod>` (or another DotStar color order). |
| LPD8806 | 5 V | using 2 data pins, Clock and Data | Use `NeoPixelBus<Lpd8806GrbFeature, Lpd8806SpiMethod>`. |
| WS2801 | 5 V | using 2 data pins, Clock and Data | Use `NeoPixelBus<NeoRgbFeature, Ws2801SpiMethod>` (adjust RGB order with feature type). |
| SK9822 | 5 V | using 2 data pins, Clock and Data | Use DotStar classes, for example `NeoPixelBus<DotStarBgrFeature, DotStarSpiMethod>`. |
| UCS8903 | 5 V |  | No dedicated `NeoUcs8903Method` alias. Use `NeoRgbUcs8903Feature` with `NeoWs2812xMethod` and validate timing on hardware. |
| GS8208 | 12 V |  | No explicit `GS8208` alias in `src/original`. Start with `NeoWs2813Method` (or `NeoWs2812xMethod`) and validate timing. |
| TM1814 | 12 V | RGBW | Use `NeoPixelBus<NeoWrgbTm1814Feature, NeoTm1814Method>`. |
| WS2805 | 12 V / 24 V | RGBCCT, 3-LED groups (12V) / 6-LED groups (24V) as one logical LED. Backup data line. Two white channels for Correlated Color Temperature (CCT) (this option is available since WLED 0.15.0-b2) | For CCT strips use `NeoPixelBus<NeoGrbcwxFeature, NeoWs2805Method>`. |
| WS2811 | 12 V | usually 3-LED segments, has data-line resistor | Use `NeoPixelBus<NeoGrbFeature, NeoWs2811Method>`. Account for grouped pixels in your animation/layout logic. |
| WS2814 | 12 V / 24 V | RGBW, 3-LED groups (12V) / 6-LED groups (24V) as one logical LED. Must be controlled as SK6812 type, color order: BRG, swap W and G (this option is available since WLED 0.14.0-b1) | Use `NeoPixelBus<NeoBrgwFeature, NeoWs2814Method>` as a starting point for BRG-family order, then verify channel mapping on hardware. |
| WS2815 | 12 V | has a backup data line | No explicit `NeoWs2815Method` alias. Start with `NeoPixelBus<NeoGrbFeature, NeoWs2813Method>` and validate timing. |
| FW1906 | 24 V | RGBCCT, 6-LED groups as one logical LED. Two white channels for Correlated Color Temperature (CCT) (this option is available since WLED 0.15.0-b2) | No explicit `FW1906` alias in `src/original`. Start with `NeoGrbcwxFeature` + `NeoWs2805Method` and validate protocol compatibility. |
| LPD6803 | 12 V |  | Use `NeoPixelBus<Lpd6803GrbFeature, Lpd6803SpiMethod>`. |
| P9813 | 5-24 V |  | Use `NeoPixelBus<P9813BgrFeature, P9813SpiMethod>`. |
| TM1829 | 5-24 V |  | Use `NeoPixelBus<NeoRgbFeature, NeoTm1829Method>`. |
| UCS8904 | 5-24 V | RGBW | No dedicated `NeoUcs8904Method` alias. Use `NeoRgbwUcs8904Feature` with `NeoWs2812xMethod` and validate timing. |

## Quick examples

```cpp
typedef NeoPixelBus<NeoGrbFeature, NeoWs2812xMethod> Ws2812bStrip;
typedef NeoPixelBus<NeoRgbwFeature, NeoSk6812Method> Sk6812Strip;
typedef NeoPixelBus<NeoWrgbTm1814Feature, NeoTm1814Method> Tm1814Strip;
typedef NeoPixelBus<DotStarBgrFeature, DotStarSpiMethod> Apa102Strip;
typedef NeoPixelBus<NeoGrbcwxFeature, NeoWs2805Method> Ws2805CctStrip;
```
