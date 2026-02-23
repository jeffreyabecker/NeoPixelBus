# Chip Gap Analysis (Legacy vs New Virtual Codebase)

This compares legacy method inventory (`src/original/internal/methods`) to the new virtual protocol inventory (`src/virtual/protocols`).

| Status | Chip / protocol family | Evidence in current tree | Gap detail |
|---|---|---|---|
| Partial | MBI6033 | legacy has `Mbi6033GenericMethod.h`; virtual side still centered on 8-bit channel model | Add/confirm true 16-bit color pipeline support for MBI6033 parity (feature model + emitter serialization behavior) |
| Closed | Pixie Stream | `src/virtual/protocols/PixieProtocol.h` now present with always-update behavior | Gap closed for virtual Pixie stream path |
| Closed | DMX512 | `src/virtual/protocols/Dmx512Protocol.h` and `src/virtual/buses/Esp8266I2SSelfClockingTransport.h` now present | Gap closed for virtual DMX512 path; keep validating framing/timing parity against legacy behavior |
| Partial | UCS8903 | legacy exposes `NeoRgbUcs8903Feature` alias in `NeoColorFeatures.h`; no virtual UCS8903-specific type/alias | Decide whether to add a UCS8903 compatibility alias and/or true 16-bit RGB feature model in virtual API |
| Partial | UCS8904 | legacy exposes `NeoRgbwUcs8904Feature` alias in `NeoColorFeatures.h`; no virtual UCS8904-specific type/alias | Decide whether to add a UCS8904 compatibility alias and/or true 16-bit RGBW feature model in virtual API |
| Partial | TM1814 / TM1914 | one-wire timings exist in `OneWireTiming.h` | In-band settings bytes still deferred (emitter-owned encoding) |
| Partial | SM168x (3/4/5-channel variants) | `Sm16716Protocol` exists; legacy feature set includes `NeoSm168x3/4/5Features` | Gain/settings packing variants still deferred |

Quick conclusion: there are currently no fully uncaptured chip/protocol families in the new codebase. MBI6033/TM1814/TM1914/SM168x/UCS8903/UCS8904 are tracked as partial parity areas (including 16-bit color-model alignment work where applicable), while Pixie Stream and DMX512 are now closed.
