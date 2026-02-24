# Consumer Guide: Virtual NeoPixel Architecture

This document describes the consumer-facing architecture for the new virtual layer and how to compose it safely.

The virtual layer is designed around explicit seams:
- **Bus abstraction** (`IPixelBus`) for pixel ownership and composition
- **Shader abstraction** (`IShader`) for pre-render transforms
- **Protocol abstraction** (`IProtocol`) for chip byte-stream generation
- **Transport abstraction** (`ITransport` + transport category tags) for platform I/O
- **Color model** (`Color`) for pixel data

> Note: The virtual layer in this repository currently uses the `npb` namespace for consumer-facing APIs.

---

## Goals

- Modern C++ constructs (spans, iterators, etc)
    - Provide ergonomic bulk pixel APIs and iterator-friendly traversal for animation engines
    - Prefer explicit value semantics and strongly typed interfaces over implicit, fragile conventions
- Expose configurability and control over construction
    - Allow consumers to choose ownership model (owning vs borrowing) for buses, protocols, and transports
    - Keep construction order explicit so platform, protocol, and rendering choices are visible in user code
- Clearly delineate seams
    - Separate pixel storage/composition (`IPixelBus`) from pixel transforms (`IShader`)
    - Separate protocol byte-stream construction (`IProtocol`) from platform signaling (`I*Transport`)
- Programmatically express valid combinations of classes
    - Make incompatible protocol/transport pairings difficult to construct
    - Keep bus composition (`Segment`, `Concat`, `Mosaic`) constrained to compatible pixel contracts
- Encapsulate virtual-layer APIs in the `npb` namespace
    - Expose a single consumer namespace for core interfaces, helpers, and compatibility wrappers
    - Avoid leaking implementation-only symbols into consumer-facing APIs
- Complete support for all chips from the original library
    - Preserve protocol behavior parity for one-wire and clock/data families
    - Preserve specialty behavior such as framing, latch/reset timing, and in-band settings semantics
- PlatformIO usage for compilation and testing
    - Keep workflows straightforward for local development and CI
    - Ensure examples and smoke builds validate platform/protocol coverage across supported targets

## Constraints

- Avoid excessive vtable lookups on hot paths
    - Keep virtual dispatch at seam boundaries (bus/protocol/transport orchestration)
    - Keep per-pixel packing and transfer loops concrete and branch-light where practical
- Target C++23 for this workspace and associated PlatformIO environments
    - Prefer standard library facilities directly at API boundaries (for example `std::span`, concepts, and strong type constraints)
    - Keep compatibility guidance for legacy builds as documentation/planning input, not as the primary architecture baseline
- Classes taking references/pointers should use `ResourceHandle` to support compile-time or static allocation strategies
    - Allow static/global embedded allocation patterns without forcing heap-only designs
    - Keep ownership semantics explicit and auditable across nested compositions
- No bit-bang transports; all transports should use DMA-backed or hardware-assisted peripherals
    - Protocol implementations should target transport abstractions backed by SPI, I2S, RMT, PIO, UART-encoded engines, or equivalent hardware blocks
    - New transport additions should document the hardware engine used and expected update readiness behavior

---

## Architecture Overview

```text
Application
  -> IPixelBus (PixelBus / SegmentBus / ConcatBus / MosaicBus)
      -> IShader chain (0..N)
      -> IProtocol (chip framing/order/settings)
          -> ITransport (ClockDataTransportTag or SelfClockingTransportTag)
              -> Hardware peripheral + DMA/PIO/RMT/I2S/SPI
```

### 1) `IPixelBus` — Consumer abstraction over a bus

`IPixelBus` is the primary API consumers interact with:
- Owns or projects a pixel address space
- Accepts bulk writes/reads via iterators and span-like ranges
- Controls frame lifecycle (`begin`, `show`, `canShow`)

Concrete bus types:
- **`PixelBus`**: base concrete bus; stores pixels and delegates output to one `IProtocol`
- **`SegmentBus`**: non-owning contiguous view into another bus
- **`ConcatBus`**: 1D concatenation of multiple child buses (uneven lengths supported)
- **`MosaicBus`**: 2D composition across panel tiles

#### Topologies, Segments, and Mosaics

- **Segments** split one physical strip into logical zones without duplicating buffers
- **Concat** combines separate physical strips into one logical index space
- **Mosaic** maps global `(x, y)` coordinates into panel-local indices using layout rules
- **Topology types** (`PanelLayout`, `PanelTopology`, `TiledTopology`) define coordinate transforms, not transport/protocol behavior

Use cases:
- Segment a single long strip into head/body/tail effects
- Concatenate multiple strips for one animation timeline
- Build 2D tile walls with panel-specific wiring orientation

### 2) `IShader` — Pixel transforms before render

`IShader` applies transformations to pixel buffers before protocol emission.

Typical shader responsibilities:
- Gamma correction (`GammaShader` + method variants)
- Current limiting (`CurrentLimiterShader`)
- Chaining multiple transforms (`ShadedTransform` / shader chain)

Design intent:
- Shaders are independent of chip protocol and transport
- Shader order is explicit and user-controlled
- No transport-level side effects from shader logic

### 3) `IProtocol` — Chip-level data stream construction

`IProtocol` converts logical `Color` pixels into the exact chip stream:
- Channel order and packing
- Chip framing/latch/reset behavior
- In-band chip settings where applicable

Examples include one-wire and clock/data families:
- One-wire protocols (`*OneWireProtocol`)
- SPI/clock-data protocols (`DotStarProtocol`, `Ws2801Protocol`, etc.)
- Specialized protocol families (`Hd108Protocol`, `Tlc59711Protocol`, etc.)

Coupling rule:
- Protocol choice is tightly coupled to a compatible transport category
- Protocols should not duplicate low-level platform signaling logic such as pin selection or frequency timing

Implementation note:
- `ColorOrderTransform` is a protocol-level implementation detail, not a consumer-facing abstraction
- It provides a clean parameterization point for a family of related operations (channel count + channel-order mapping) used during protocol packing
- This keeps protocol classes focused on framing/timing/settings while reusing one consistent color-order serialization path
- Planned follow-on after one-wire transport consolidation: fold this packing logic into `Ws2812xProtocol<TColor>` as protocol-private behavior while preserving byte-stream compatibility.

### 4) `ITransport` (+ transport category tags) — Platform transport seam

Transport abstractions own platform and peripheral behavior:
- Peripheral initialization and pin binding
- DMA or hardware engine ownership (SPI, I2S, RMT, PIO, UART-encoded self-clocking, etc.)
- Transfer readiness and update cadence

`ITransport` is the runtime seam; transport category tags (`ClockDataTransportTag`, `SelfClockingTransportTag`) encode compatibility at compile time for protocol/transport pairings.

Why platform-specific:
- RP2040, ESP32, ESP8266, and nRF differ in hardware blocks and DMA capabilities
- The same protocol intent may require different low-level transport implementations per platform

Constraint reminder:
- No software bit-bang transport in the new layer

### 5) `Color` — Fundamental pixel representation

`Color` is the canonical pixel payload passed between buses, shaders, and protocols.

Consumer expectations:
- Stable indexing semantics for RGB (+ optional white channels as needed)
- Value semantics for safe copy/transform
- Compatibility with iterator/span workflows

Migration note:
- Named numeric channel index constants were removed.
- Use character-based indexing instead: `'R'`, `'G'`, `'B'`, `'W'`, `'C'`.
- White-channel mapping is explicit: `'W'` = warm white, `'C'` = cool white.

#### Color iterators

`ColorIterator` is the core range primitive used by bus APIs for bulk pixel transfer.

Key characteristics:
- Random-access iterator semantics (supports offset arithmetic and STL-style algorithms)
- Suitable for both read and write paths through mutable `Color&` access
- Position-based traversal so callers can operate on full ranges or sub-ranges efficiently

Consumer usage patterns:
- Stream generated frames into a bus using `begin()`/`end()` iterator pairs
- Pull pixel data out of a bus into an existing buffer via iterator destinations
- Reuse standard algorithm patterns (`copy`, range slicing via iterator math) for effects pipelines

#### Color sources

Color sources are lightweight range adapters that produce `ColorIterator` pairs.

Built-in source types:
- `SolidColorSource` (also available as `FillColorSource`): yields one constant color for N pixels
- `SpanColorSource`: wraps a contiguous color buffer and exposes it as an iterator range

Why sources matter:
- Avoids duplicating special-purpose fill/copy APIs on every bus type
- Enables the same `setPixelColors`/`getPixelColors` pattern across `PixelBus`, `SegmentBus`, `ConcatBus`, and `MosaicBus`
- Keeps transformations composable with shader and protocol stages

Practical examples:
- Fill a segment with one color using `SolidColorSource`
- Push a prepared frame buffer to a bus using `SpanColorSource`
- Read back pixels into a mutable span-backed buffer for diagnostics or stateful effects

Design direction:
- Keep color representation protocol-agnostic at the API seam
- Perform protocol-specific narrowing/packing only inside `IProtocol` implementations

---

## Valid Combination Rules

The model should make invalid combinations hard (or impossible) to express.

Recommended compile-time rules:
- `PixelBus` accepts an `IProtocol` that is compatible with the bus `Color` type
- `IProtocol` accepts only compatible transport category tags (`ClockDataTransportTag` vs `SelfClockingTransportTag`)
- `ConcatBus`/`MosaicBus` children should share the same `Color` contract
- Resource ownership is explicit via `ResourceHandle<T>` (owning or borrowing)

Hot-path performance guidance:
- Favor one virtual dispatch per frame boundary, not per-channel/per-bit operations
- Keep inner loops in protocol packing and transport transfer paths non-virtual where practical

---

## Construction and Ownership Model (`ResourceHandle`)

`ResourceHandle<T>` provides a consistent ownership seam:
- **Borrowing**: wrap an lvalue reference (caller retains lifetime)
- **Owning**: wrap a `std::unique_ptr` (resource lifetime attached to parent)

Benefits:
- Works with static/global allocation patterns common in embedded systems
- Supports dynamic composition/factory patterns without API forks
- Keeps ownership explicit for buses, protocols, and shared components

### Ownership semantics by component

- **`PixelBus`**
    - Owns pixel storage internally
    - Owns or borrows its protocol via `ResourceHandle<IProtocol>`
    - If borrowing, protocol object must outlive the bus

- **`SegmentBus`**
    - Non-owning view into a parent `IPixelBus`
    - Never takes ownership of parent resources
    - Parent bus must outlive every segment view

- **`ConcatBus` / `MosaicBus`**
    - Accept child buses as `ResourceHandle<IPixelBus>` entries
    - Can mix owning and borrowing children in one composition
    - Borrowed children must outlive the composite bus

- **Protocols (`IProtocol`)**
    - May own or borrow shaders/transports based on settings type
    - Ownership should be explicit in protocol settings (for example `ResourceHandle<ITransport>`)

- **Settings structs**
    - Settings objects are typically passed by value into constructors, but may carry references/handles internally
    - Ownership is determined by settings field types, not by the fact that settings are copied/moved
    - `ResourceHandle<T>` fields preserve owning/borrowing mode when moved into protocol/bus instances
    - Raw references in settings (for example `Print&`) are always borrowed and must outlive the constructed object

- **Transports (`ITransport`)**
    - Own platform signaling state (peripheral claims, DMA handles, readiness state)
    - Should release hardware resources in destructors when owned

### Lifetime rules

- Borrowed dependency: creator manages lifetime and must keep dependency alive longer than consumer
- Owned dependency: consumer manages lifetime automatically and destroys dependency on teardown
- Views (`SegmentBus`) are always borrowed; treat them as aliases, not containers
- Composite buses should avoid dangling borrowed children during dynamic reconfiguration
- Settings-to-object transfer does not convert borrowed resources into owned resources automatically

### Settings ownership conventions

- Prefer `ResourceHandle<T>` inside settings whenever ownership may vary by caller
- Use references in settings only for stable, externally managed resources (for example long-lived `Print` or hardware singletons)
- Avoid storing temporary objects behind borrowed settings references
- Keep ownership intent visible in settings names/docs (for example `bus`, `transport`, `shader` fields)
- When adding new settings fields, document whether each field is copied, owned, or borrowed

### Example: owned transport with borrowed output sink

```cpp
// Serial is borrowed (externally owned by Arduino runtime)
// PrintTransport is owned by PixieProtocol via ResourceHandle

auto protocol = std::make_unique<npb::PixieProtocol>(
    pixelCount,
    nullptr,
    npb::PixieProtocolSettings{
        std::make_unique<npb::PrintTransport>(Serial),
        npb::ChannelOrder::RGB
    });

auto bus = std::make_unique<npb::PixelBus>(pixelCount, std::move(protocol));
```

Interpretation:
- `Serial` is borrowed and must remain valid for the lifetime of `PrintTransport`
- `PrintTransport` is owned by `PixieProtocol`
- `PixieProtocol` is owned by `PixelBus`
- Destroying `PixelBus` tears down the owned chain automatically

### Practical construction patterns

- **Static/embedded pattern (mostly borrowed)**
    - Use global/static protocol and transport instances
    - Pass references into `ResourceHandle` for deterministic allocation

- **Factory/dynamic pattern (mostly owned)**
    - Build protocol/transport with `std::make_unique`
    - Pass owning handles to parent objects for RAII-style teardown

- **Hybrid pattern**
    - Borrow long-lived hardware singleton transports
    - Own short-lived protocol wrappers, shader chains, or composite buses

### Ownership review checklist

- For every `ResourceHandle<T>`, decide and document: owner vs borrower
- Ensure every borrowed object has a clear longer-lived scope
- Ensure composite bus children follow a consistent lifetime contract
- Keep protocol/transport teardown behavior symmetric with initialization

---

## Language Baseline and Compatibility Notes

Current baseline:
- The active build/test workflow in this workspace targets C++23.
- Public APIs in the virtual layer use standard modern C++ constructs directly (for example `std::span` and concepts).

Compatibility notes:
- Legacy C++11 migration guidance remains useful for planning and compatibility analysis.
- New architecture and usage examples in this document assume modern toolchains first.

---

## Chip Coverage Commitment

The virtual architecture targets complete chip-family coverage present in the original library, including:
- One-wire families
- Clock/data SPI-style families
- High-bit-depth and specialty chipsets

Coverage principle:
- Protocol behavior parity with original library semantics
- Platform transport implementation selected per target capabilities

---

## PlatformIO Workflow (Compilation + Testing)

Recommended consumer workflow:

1. Add one or more PlatformIO environments in `platformio.ini`
2. Build smoke targets (for example under `examples-virtual/`)
3. Validate protocol/transport combinations per target platform
4. Keep at least one modern compile path in CI/local checks; add legacy compatibility builds only where required

Typical commands:
- `pio run`
- `pio run -e <env>`
- `pio test -e <env>` (where test targets are configured)

---

## Quick Assembly Pattern

1. Pick a `Color` contract
2. Pick a platform transport (`ITransport`) with the needed transport category tag
3. Pick a chip `IProtocol` compatible with that transport
4. Build a `PixelBus` with `ResourceHandle<IProtocol>`
5. Optionally compose with `SegmentBus`, `ConcatBus`, or `MosaicBus`
6. Attach shaders and render via `show()`

This separation keeps application logic stable while allowing protocol and hardware evolution independently.
