# Chip Gap Analysis (Legacy vs New Virtual Codebase)

This compares legacy method inventory (`src/original/internal/methods`) to the new virtual emitter inventory (`src/virtual/emitters`).

| Status | Chip / protocol family | Evidence in current tree | Gap detail |
|---|---|---|---|
| Partial | MBI6033 | legacy has `Mbi6033GenericMethod.h`; virtual side still centered on 8-bit channel model | Add/confirm true 16-bit color pipeline support for MBI6033 parity (feature model + emitter serialization behavior) |
| Missing | Pixie Stream | legacy has `PixieStreamMethod.h`; virtual emitter not present | Add `src/virtual/emitters/PixieStreamEmitter.h` (always-update behavior) |
| Missing | DMX512 | legacy has `NeoEsp8266I2sDmx512Method.h`; virtual emitter not present | Add `src/virtual/emitters/Dmx512Emitter.h` + ESP8266 self-clocking transport |
| Partial | UCS8903 | legacy exposes `NeoRgbUcs8903Feature` alias in `NeoColorFeatures.h`; no virtual UCS8903-specific type/alias | Decide whether to add a UCS8903 compatibility alias and/or true 16-bit RGB feature model in virtual API |
| Partial | UCS8904 | legacy exposes `NeoRgbwUcs8904Feature` alias in `NeoColorFeatures.h`; no virtual UCS8904-specific type/alias | Decide whether to add a UCS8904 compatibility alias and/or true 16-bit RGBW feature model in virtual API |
| Partial | TM1814 / TM1914 | one-wire timings exist in `OneWireTiming.h` | In-band settings bytes still deferred (emitter-owned encoding) |
| Partial | SM168x (3/4/5-channel variants) | `Sm16716Emitter` exists; legacy feature set includes `NeoSm168x3/4/5Features` | Gain/settings packing variants still deferred |

Quick conclusion: the only fully uncaptured chip/protocol families in the new codebase are **Pixie Stream** and **DMX512**. MBI6033/TM1814/TM1914/SM168x/UCS8903/UCS8904 are currently tracked as partial parity areas (including 16-bit color-model alignment work where applicable).
