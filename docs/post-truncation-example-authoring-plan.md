# Post-Truncation Example Authoring Plan

Generated: 2026-02-25  
Basis:
- retained legacy example matrix
- current virtual protocol/transport availability
- architecture commitments in `docs/consumer-virtual-architecture.md`

## 0) Consolidated Inputs (Existing Plan -> This Style)

This plan consolidates prior example guidance from architecture docs into an executable backlog format.

Imported architecture commitments:
1. **Layer separation**: Color -> Shader -> Protocol -> Transport -> Bus composition.
2. **Protocol/transport compatibility rules**: examples must demonstrate category-correct pairings.
3. **Chip coverage commitment**: include one-wire, clock/data, and specialty families.
4. **Quick assembly pattern**: every example should read as protocol+transport assembly, not monolithic sketch logic.
5. **Platform-first validation**: examples are grouped and built by platform.
6. **Factory-first construction**: examples should prefer `makeBus<ProtocolDesc, TransportDesc>(...)` with descriptor/options inputs.

## 1) Goals

1. Make protocol vs transport responsibilities obvious in every example.
2. Group examples by platform so users can find runnable starting points quickly.
3. Ensure WS2812/WS2812x is represented across all major functional areas.
4. Prefer examples that map directly to the virtual architecture (`protocol + transport + bus + shader/topology`).
5. Make factory call-shape and descriptor intent obvious in every example.

## 2) Factory-First Architecture Rule for Examples

Every example should explicitly declare and comment these four layers:

1. **Protocol descriptor**: chip family token passed to factory (e.g., `Ws2812`, `descriptors::Ws2812x<>`, `APA102`).
2. **Transport descriptor/options**: hardware/output token + options passed to factory (e.g., `descriptors::RpPio` + `RpPioOptions`, `descriptors::Esp32RmtOneWire`).
3. **Bus**: pixel storage + show lifecycle returned from factory (`makeBus(...)`).
4. **Optional pipeline**: shader/topology/composite buses.

Required naming in each sketch:
- `using ProtocolDesc = ...;`
- `using TransportDesc = ...;`
- `using BusType = ...;`

Required construction order in code (mirrors architecture quick-assembly pattern):
1. Pick color contract
2. Pick protocol descriptor
3. Pick transport descriptor/options compatible with protocol capability
4. Build bus (`makeBus<ProtocolDesc, TransportDesc>(...)`)
5. Attach optional shaders/topology/composition

Factory call-shape requirements:
- Unwrapped one-wire/clocked forms use `makeBus(pixelCount, protocolConfig, transportConfig)` or protocol-omitted form.
- Wrapped one-wire forms must use timing-first order: `makeBus(pixelCount, protocolConfig, OneWireTiming::..., transportConfig)`.
- Do not introduce timing-last example forms.
- Prefer descriptor/options inputs over direct concrete protocol/transport construction in examples.

## 3) Folder Strategy (Grouped by Platform)

Use platform-first grouping under `examples-virtual/`:

- `examples-virtual/common/` (host-independent patterns with `Nil`/`Debug` transports)
- `examples-virtual/rp2040/`
- `examples-virtual/esp32/`
- `examples-virtual/esp8266/`
- `examples-virtual/nrf52/`

Inside each platform folder, group by functional area:

- `01-basics/`
- `02-color-and-blending/`
- `03-shaders/`
- `04-topology-and-composition/`
- `05-protocol-showcase/`
- `06-transport-showcase/`

## 4) Functional Areas + WS2812 Coverage Requirement

WS2812 must have at least one example in each area below:

1. **Basics**: set/show, clear/fill, single-pixel update.
2. **Color + blending**: `HslColor`/`HsbColor` conversions + external `ColorMath` blend/darken/lighten.
3. **Shaders**: gamma + current limiter (+ aggregate chain).
4. **Topology/composition**: panel/tiled mapping and at least one composed bus (`SegmentBus` or `ConcatBus`).
5. **Protocol/transport separation**: same WS2812 descriptor demonstrated over at least two transports on the same platform where available.

Added from architecture commitments:
6. **Bus composition**: at least one WS2812 example each for `SegmentBus`, `ConcatBus`, or `MosaicBus`.
7. **Diagnostics path**: at least one WS2812-oriented debug pipeline example using `DebugProtocol` and/or `DebugTransport`.

## 5) Example Backlog (Prioritized)

## Phase 1 — Must-Have Baseline (ship first)

| Example ID | Platform | Functional Area | Protocol Descriptor | Transport Descriptor / Options | Priority |
|---|---|---|---|---|---|
| `ws2812_basic_rp2040_pio` | RP2040 | Basics | `Ws2812` | `descriptors::RpPio` + timing-first `OneWireTiming::Ws2812` | P0 |
| `ws2812_basic_esp32_rmt` | ESP32 | Basics | `Ws2812` | `descriptors::Esp32RmtOneWire` | P0 |
| `ws2812_color_math_rp2040` | RP2040 | Color + blending | `Ws2812` | `descriptors::RpPio` + timing-first `OneWireTiming::Ws2812` | P0 |
| `ws2812_shaders_gamma_current_rp2040` | RP2040 | Shaders | `Ws2812` | `descriptors::RpPio` + timing-first `OneWireTiming::Ws2812` | P0 |
| `ws2812_topology_tiled_rp2040` | RP2040 | Topology/composition | `Ws2812` | `descriptors::RpPio` + timing-first `OneWireTiming::Ws2812` | P0 |
| `ws2812_transport_compare_esp32` | ESP32 | Protocol/transport separation | `Ws2812` | `descriptors::Esp32RmtOneWire` vs `descriptors::Esp32I2s` + timing-first `OneWireTiming::Ws2812` | P0 |

## Phase 2 — Cross-Protocol Coverage (common chips)

| Example ID | Platform | Functional Area | Protocol Descriptor | Transport Descriptor / Options | Priority |
|---|---|---|---|---|---|
| `dotstar_basic_esp32_dma_spi` | ESP32 | Basics | `APA102` | `descriptors::Esp32DmaSpi` | P1 |
| `hd108_basic_esp32_dma_spi` | ESP32 | Basics | `Hd108` (descriptor token) | `descriptors::Esp32DmaSpi` | P1 |
| `pixie_basic_esp8266_uart` | ESP8266 | Protocol showcase | `Pixie` (descriptor token) | `descriptors::Esp8266DmaUart` + timing-first `OneWireTiming::Ws2812` | P1 |

Chip-family coverage minimum for Phase 2:
- One-wire descriptor family: `Ws2812` / `descriptors::Ws2812x<>`
- Clock/data descriptor family: `APA102` or `Ws2801` descriptor token
- Specialty descriptor family: `Hd108` or `Pixie` descriptor token

## Phase 3 — Composition + Diagnostics

| Example ID | Platform | Functional Area | Protocol Descriptor | Transport Descriptor / Options | Priority |
|---|---|---|---|---|---|
| `ws2812_concat_segments_esp32` | ESP32 | Topology/composition | `Ws2812` | `descriptors::Esp32RmtOneWire` | P2 |
| `ws2812_debug_protocol_pipeline_common` | Common | Diagnostics | `Debug` descriptor token | `descriptors::NeoPrint` / debug transport descriptor token | P2 |
| `ws2812_mosaic_bus_rp2040` | RP2040 | Topology/composition | `Ws2812` | `descriptors::RpPio` + timing-first `OneWireTiming::Ws2812` | P2 |

## Phase 4 — Platform Expansion Parity

| Example ID | Platform | Functional Area | Protocol Descriptor | Transport Descriptor / Options | Priority |
|---|---|---|---|---|---|
| `ws2812_basic_esp8266_uart` | ESP8266 | Basics | `Ws2812` | `descriptors::Esp8266DmaUart` + timing-first `OneWireTiming::Ws2812` | P3 |
| `ws2812_basic_nrf52_pwm` | nRF52 | Basics | `Ws2812` | `Nrf52PwmOneWire` descriptor token | P3 |
| `ws2812_shaders_esp32` | ESP32 | Shaders | `Ws2812` | `descriptors::Esp32RmtOneWire` | P3 |
| `ws2812_topology_esp32` | ESP32 | Topology/composition | `Ws2812` | `descriptors::Esp32RmtOneWire` | P3 |

## 6) Protocol vs Transport Clarity Checklist (per example)

Every example must include:

1. A comment block titled **Protocol Descriptor** that explains chip-specific frame semantics and selected descriptor.
2. A comment block titled **Transport Descriptor** that explains peripheral/path and selected options.
3. A short "swap matrix" in comments showing at least one alternative protocol and one alternative transport.
4. A statement of why the chosen pair is valid (timing/category compatibility).
5. The exact `makeBus<...>(...)` call shape used by the example (including timing-first ordering where applicable).

## 7) Example Template Standard

Each example should follow this layout:

1. **Section A**: type aliases (`ProtocolDesc`, `TransportDesc`, `BusType`).
2. **Section B**: options/settings structs for protocol and transport descriptors.
3. **Section C**: bus construction (`makeBus<ProtocolDesc, TransportDesc>(...)`).
4. **Section D**: functional-area logic (e.g., shader chain, topology mapping).
5. **Section E**: protocol/transport swap notes.

Include policy for examples:
- Include only public headers (`#include <LumaWave.h>` and `#include <Arduino.h>` when runtime APIs require it).
- Do not include internal headers directly from examples (`factory/*`, `protocols/*`, `transports/*`).

## 8) Acceptance Criteria

The example plan is complete when:

1. All Phase 1 examples are implemented and compile for their target platform envs.
2. WS2812 examples exist for every functional area listed in Section 4.
3. At least one example per major platform (RP2040, ESP32, ESP8266) explicitly compares transports.
4. Protocol/transport naming and comment conventions are consistent across all examples.
5. Example README index links by platform first, then by functional area.
6. Architecture-doc construction order is visible and consistent in all P0/P1 examples.
7. Chip-family coverage minimum (one-wire + clock/data + specialty) is satisfied by implemented examples.

## 9) Mapping: Existing Architecture Plan -> Example Backlog

| Existing Architecture Commitment | Example-Plan Representation |
|---|---|
| Layer separation (`IShader`, `IProtocol`, `ITransport`) | Section 2 + per-example checklist + template standard |
| Protocol/transport category compatibility | Section 2 construction order + Section 6 checklist item 4 |
| Build smoke targets under `examples-virtual/` | Section 3 folder strategy + Section 8 acceptance criteria |
| Chip coverage commitment | Section 5 Phase 2 + chip-family minimum |
| Composition capabilities (`SegmentBus`, `ConcatBus`, `MosaicBus`) | Section 4 item 6 + Phase 3 backlog |

## 10) Out of Scope (for this plan)

- Legacy-only constructs that are intentionally not ported (`NeoPixelAnimator`, ring topology, seven-segment legacy bus).
- ESP32 parallel LCD/I2S legacy patterns not represented by current virtual transport surface.
