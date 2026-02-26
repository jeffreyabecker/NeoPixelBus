# Native Testing Plan (PlatformIO + Unity + ArduinoFake)

## Goal

Establish a host-side automated test workflow using:

- PlatformIO `native` test environment
- Unity as the test framework
- ArduinoFake for Arduino API mocking

The test suite is organized into three categories:

- `busses`
- `shaders`
- `protocols`

Additionally, color-type compatibility coverage should include both 8-bit and 16-bit color models, with explicit tracking for `Rgbcw16Color` paths.

This enables fast feedback for logic-level validation without requiring RP2040 hardware.

---

## Scope

### In scope

- Add a dedicated PlatformIO native test environment
- Add Unity + ArduinoFake as test dependencies
- Create a category-based `test/` layout
- Provide shared test support/fake bootstrap utilities
- Add initial smoke tests in each category
- Define local and CI test commands
- Plan coverage for 16-bit color-type behavior (`Rgb16Color`, `Rgbw16Color`, `Rgbcw16Color`) where bus/protocol behavior is color-depth dependent

### Out of scope (initial phase)

- Hardware timing/peripheral validation (PIO/DMA/ISR behavior)
- Full transport integration with physical IO
- Performance benchmarking in host tests

---

## Proposed Directory Layout

```text
test/
  support/
    FakeArduinoSetup.h
    FakeArduinoSetup.cpp
    TestHelpers.h

  busses/
    test_bus_smoke/
      test_main.cpp

  shaders/
    test_shader_smoke/
      test_main.cpp

  protocols/
    test_protocol_smoke/
      test_main.cpp
```

Conventions:

- One focused test package per behavior area.
- Shared helpers stay in `test/support`.
- Keep test names descriptive and category-prefixed.

---

## PlatformIO Configuration Plan

Add a new environment to `platformio.ini` for native tests.

### Environment objectives

- Use `platform = native`
- Compile with C++23 (`-std=gnu++23`)
- Reuse core include paths where possible
- Keep embedded-only flags isolated from native tests

### Dependency objectives

- Unity enabled for test runner
- ArduinoFake added as test dependency

### Command objectives

- Run all native tests:
  - `pio test -e native-test`
- Run single category by directory filter:
  - `pio test -e native-test -f busses`
  - `pio test -e native-test -f shaders`
  - `pio test -e native-test -f protocols`

---

## Fake Strategy (ArduinoFake)

Use a centralized bootstrap/reset pattern for mocks:

- `FakeArduinoSetup` initializes default fake behavior used by most tests.
- Each test `setUp()` resets fakes to avoid cross-test contamination.
- Avoid ad-hoc fake setup in each test file unless scenario-specific.

Target APIs to fake first:

- Timing (`millis`, `micros`, `delay`)
- GPIO interactions (`pinMode`, `digitalWrite`, `digitalRead`)
- Optional serial paths used by debug/helper flows

---

## Category Test Plan

## 1) Busses

Primary focus:

- Construction and begin/show lifecycle behavior
- Pixel index and buffer bounds behavior
- Segment mapping and count invariants
- Behavior parity across color component depths, including `Rgbcw16Color`

Initial smoke coverage:

- Create bus with minimal valid config
- Verify no-op / empty behavior is stable
- Verify expected state transitions after begin/show

## 2) Shaders

Primary focus:

- Deterministic behavior for utility shaders used for safety/composition
- Edge values and clamping behavior where mathematically defined
- Ordering/idempotence expectations for shader composition

Shader coverage scope policy:

- Out of strict automated scope: `GammaShader`, `WhiteBalanceShader`
  - Rationale: these are primarily artistic/tuning transforms and are not treated as strict correctness gates in native unit tests.
- In automated scope: `CurrentLimiterShader`, `AggregateShader`
  - Rationale: these provide safety/composition behavior with deterministic, testable outcomes.

Initial smoke coverage:

- `CurrentLimiterShader`: apply fixed high-current frame and verify limiting behavior is deterministic and bounded.
- `AggregateShader`: apply ordered shader chain and verify output ordering/composition behavior.

## 3) Protocols

Primary focus:

- Encode/frame formatting correctness
- Channel ordering and byte layout rules
- Invalid input guard behavior
- Correct handling of multi-channel 16-bit payloads, including `Rgbcw16Color`

## Cross-Cutting Color-Type Matrix (planned)

Track a shared matrix for key color models used by virtual APIs:

- 8-bit: `Rgb8Color`, `Rgbw8Color`, `Rgbcw8Color`
- 16-bit: `Rgb16Color`, `Rgbw16Color`, `Rgbcw16Color`

Initial expectation for matrix coverage:

- Bus write/read semantics remain consistent regardless of component width.
- Protocol formatting paths that depend on component bit depth are validated for 16-bit types.
- Channel indexing semantics (`R`, `G`, `B`, `W`, `C`) remain consistent for 5-channel color types at 16-bit depth.

### Priority order

Phase 1 (highest priority):

- `Rgbcw16Color` core-path coverage (5-channel + 16-bit) for bus read/write semantics.
- `Rgbcw16Color` protocol formatting checks where bit-depth/channel-count logic diverges from 8-bit behavior.

Phase 2:

- `Rgb16Color` and `Rgbw16Color` parity coverage for remaining bus/protocol seams.
- Expanded regression matrix across all 8-bit and 16-bit aliases.

Initial smoke coverage:

- Encode simple pixel input and verify byte stream shape
- Validate expected header/footer/padding semantics where defined

## Coverage Backlog by Domain

### Busses Backlog

#### Coverage snapshot

- Current automated bus coverage is only a small smoke set for `PixelBus` in `test/busses/test_bus_pixelbus_smoke/test_main.cpp`.
- Primary implementation targets for this backlog:
  - `src/buses/PixelBus.h`
  - `src/buses/SegmentBus.h`
  - `src/buses/ConcatBus.h`
  - `src/buses/MosaicBus.h`

#### Positive tests

- `PixelBus`
  - Span and iterator bulk `set/get` round-trip.
  - Partial write at end clamps correctly.
  - `show()` updates only when dirty unless `alwaysUpdate()==true`.
- `SegmentBus`
  - Local index 0 maps to parent offset.
  - Bulk write/read stays within segment.
  - Multiple segments over same parent isolate writes by range.
- `ConcatBus`
  - Global index resolves across uneven child lengths.
  - `pixelCount()` equals child sum.
  - `begin()/show()` fan out to all children.
  - `remove()` updates mapping table and total count.
- `MosaicBus`
  - 2D `set/get` resolves to correct panel and local index.
  - Linear `setPixelColors/getPixelColors` matches panel-by-panel flattening.
  - `canShow()` returns true only when all children are ready.

#### Negative tests

- P0
  - `PixelBus` and `BusDriverPixelBus` bulk methods with `offset > pixelCount` (iterator + span forms) should no-op without write/read overflow.
- `PixelBus` / `BusDriverPixelBus`
  - Out-of-range `setPixelColor` is a no-op.
  - Out-of-range `getPixelColor` returns default color.
- `SegmentBus`
  - Operations with `offset >= segmentLength` are no-op.
  - Oversize writes clamp to segment end.
  - Segment length exceeding practical parent space does not corrupt parent out of range.
- `ConcatBus`
  - Write/read starting beyond total pixel count is no-op.
  - Removing non-member bus returns false.
  - Adding null `ResourceHandle` is ignored.
- `MosaicBus`
  - Out-of-bounds `(x, y)` returns default/no-op behavior.
  - Tile index resolving past provided bus count is safely handled.
  - Empty bus list reports width/height as zero.

#### Suggested next step

- Implement this backlog as test stubs under `test/busses/`, prioritizing P0 cases first.

### Topologies Backlog

#### Coverage snapshot

- No dedicated topology tests are currently present.
- Primary implementation targets for topology mapping:
  - `src/topologies/PanelLayout.h`
  - `src/topologies/PanelTopology.h`
  - `src/topologies/TiledTopology.h`

#### Positive tests

- `PanelLayout::mapLayout`
  - Golden-coordinate assertions for all 16 layouts on a 4x4 panel (table-driven expected indices).
- `tilePreferredLayout`
  - Parity mapping correctness for each base group (`RowMajor`, alternating row/column groups).
- `PanelTopology`
  - `mapProbe` in-bounds returns expected index.
  - `map` clamps edges correctly.
  - `pixelCount == width * height`.
- `TiledTopology`
  - `mapProbe` correctness across tile boundaries.
  - `map` clamping at global mosaic edges.
  - `topologyHint` returns `FirstOnPanel`, `InPanel`, and `LastOnPanel` at expected coordinates.

#### Negative tests

- `PanelTopology`
  - Out-of-bounds `mapProbe` returns `std::nullopt` for negative coordinates and for `>= width/height`.
- `TiledTopology`
  - Out-of-bounds `mapProbe` returns `std::nullopt`.
  - `topologyHint` returns `OutOfBounds` for invalid coordinates.
- P0
  - Zero-dimension configs (`panelWidth == 0` and/or `panelHeight == 0`) for `PanelTopology::map` and `TiledTopology::map` should be explicitly guarded (avoid clamp underflow path risk).
- `TiledTopology`
  - Mismatch between `tilesWide * tilesHigh` assumptions and probing behavior remains safely bounded for nominal but non-existent tiles.

#### Suggested next step

- Implement this backlog as test stubs under `test/topologies/`, prioritizing P0 cases first.

### Colors Backlog

#### Coverage snapshot

- No dedicated native tests currently target `src/colors/Color.h` or `src/colors/ColorIterator.h`.
- Existing native tests are currently focused on bus/protocol smoke behavior.

#### Positive tests

- `BasicColor` construction
  - Default constructor initializes all channels to zero.
  - Variadic constructor writes provided prefix values and leaves remaining channels zero.
  - Constructor conversion from smaller integer literals preserves value for both 8-bit and 16-bit aliases.
- Indexing semantics
  - Integral index accessor round-trips channel values for all supported aliases (`Rgb8Color`, `Rgbw8Color`, `Rgbcw8Color`, `Rgb16Color`, `Rgbw16Color`, `Rgbcw16Color`).
  - Channel-character access (`'R'`, `'G'`, `'B'`, `'W'`, `'C'`) maps to expected channels for 3/4/5-channel colors.
  - Character accessor accepts lower-case variants (`'r'`, `'g'`, `'b'`, `'w'`, `'c'`).
- Equality and constants
  - `operator==` returns true for equal channel arrays and false for any channel mismatch.
  - `ChannelCount` and `MaxComponent` values match alias definitions (8-bit max 255, 16-bit max 65535).
- Channel order constants
  - `ChannelOrder::*` strings and `Length*` constants are internally consistent (length equals runtime string length).
- Conversion helpers
  - `widen` replicates 8-bit channel bytes into 16-bit (`v -> (v << 8) | v`).
  - `narrow` extracts high byte from 16-bit channel (`v -> v >> 8`).
  - `expand<N,M>` copies source channels in order and zero-initializes added channels.
  - `compress<N,M>` keeps leading channels in order and drops trailing channels.

#### Negative tests

- P0
  - Out-of-range integral indexing is undefined in current implementation; add guard expectations at call sites and verify no tests rely on unchecked out-of-bounds access.
- Channel fallback behavior
  - Unknown channel character maps to index 0 by design; verify this fallback explicitly so accidental changes are detected.
  - `'W'`/`'C'` on colors with insufficient channel count map to index 0 by design; verify this behavior explicitly.
- Boundary/value stress
  - `widen` and `narrow` behavior at extrema (`0x00`, `0xFF`, `0x0000`, `0xFFFF`).
  - `expand`/`compress` with minimum/maximum practical channel counts used by the library (3, 4, 5) preserve ordering and do not leak garbage values.

### ColorIterator Backlog

#### Coverage snapshot

- No dedicated native tests currently target `src/colors/ColorIterator.h`.
- Existing native tests are currently focused on bus/protocol smoke behavior.

#### Positive tests

- `ColorIteratorT` iterator contract
  - Pre/post increment and decrement move position correctly.
  - `+`, `-`, `+=`, `-=` arithmetic matches expected position math.
  - Difference operator returns expected distance between begin/end.
  - Dereference and `operator[]` return mutable references to underlying storage.
  - Equality and ordering operators are consistent for iterators over the same range.
- STL algorithm compatibility
  - `std::copy` from `SolidColorSourceT` into span-backed destination yields uniform frame.
  - `std::copy` from `SpanColorSourceT` to another destination preserves sequence.
- `SolidColorSourceT`
  - `begin()`/`end()` distance equals `pixelCount`.
  - Writing through iterator reference mutates the single backing color as expected.
- `SpanColorSourceT`
  - `begin()`/`end()` distance equals span size.
  - Iterator writes mutate original span buffer in place.
  - Pointer+size constructor and span constructor produce equivalent iteration behavior.

#### Negative tests

- P0
  - `SpanColorSourceT` with span size > 65535 truncates end position to `uint16_t`; add explicit regression coverage documenting current limit and expected behavior.
  - `ColorIteratorT` arithmetic underflow/overflow risk due to `uint16_t` position; add tests to ensure callers clamp ranges before iterator math that crosses boundaries.
- Iterator comparison caveat
  - Equality compares position only (not accessor identity); add tests that document intended usage (compare iterators from same range) and prevent misuse assumptions.
- Default-constructed iterator safety
  - Default iterators compare equal, but dereference without accessor is invalid; add tests that avoid dereference and codify this contract.
- Range responsibility
  - Source/iterator do not enforce bounds; add tests around consumer APIs (bus set/get) to ensure range clamping occurs at bus layer, not iterator layer.

#### Suggested next step

- Add a new `test/colors/` category with focused suites for `Color` and `ColorIterator` semantics, with P0 items implemented first.

### Transports Backlog

#### OneWireWrapper

##### Coverage snapshot

- No dedicated native tests currently target `src/transports/OneWireWrapper.h`.
- Wrapper behavior spans three seams that need coverage:
  - Bitstream encoding (`encode3StepBytes`, `encode4StepBytes`, `encodeStepBytes`)
  - Transaction orchestration (`manageTransaction`, passthrough transport calls)
  - Update readiness timing (`begin`, `updateFrameTiming`, `isReadyToUpdate`)

##### Positive tests

- Construction and lifecycle
  - Wrapper constructs from `OneWireWrapperConfig<TTransportConfig>` and forwards base transport config.
  - `begin()` calls wrapped transport `begin()`, resets frame duration, and initializes frame end timestamp.
- Encoding helpers
  - `encode3StepBytes` returns expected encoded byte count for representative payload sizes (`0`, `1`, `N`).
  - `encode4StepBytes` returns expected encoded byte count for representative payload sizes (`0`, `1`, `N`).
  - Golden pattern checks for known source bytes (`0x00`, `0xFF`, `0x80`, `0x01`) for both 3-step and 4-step modes.
- Transmit path behavior
  - `transmitBytes()` encodes input according to selected bit pattern and forwards encoded span to wrapped transport.
  - With `manageTransaction=true`, wrapper calls `beginTransaction()` then `transmitBytes()` then `endTransaction()` in order.
  - With `manageTransaction=false`, wrapper skips transaction calls and only forwards transmit.
  - Encoded buffer capacity resizes appropriately when input length changes between frames.
- Timing and readiness
  - With `clockRateHz==0`, frame duration uses `timing.resetUs`.
  - With non-zero bit rate, frame duration uses `max(encodedDurationUs, timing.resetUs)`.
  - `isReadyToUpdate()` returns true only when both wrapped transport is ready and reset timing window has elapsed.
- Protocol integration (OneWire wrapper + `Ws2812xProtocol`)
  - For `channelOrder` lengths 3/4/5, transmitted encoded length matches `pixelCount * resolvedChannelCount * encodedBitsPerDataBit`.
  - Null/empty `channelOrder` fallback in protocol still yields valid wrapper transmit sizing and readiness timing.
  - 16-bit `Ws2812xProtocol` input (narrowed to wire bytes) yields the same wrapper encoded-length math as 8-bit for equal frame byte count.

##### Negative tests

- P0
  - 3-step encoding bit packing across byte boundaries is validated with golden vectors to catch dropped/corrupted carry bits.
  - Large payload sizes do not cause encoded size arithmetic/resize issues (capacity and transmitted length remain consistent).
- Configuration edge cases
  - Invalid or non-standard `bitPattern` enum values are handled safely (no crash/UB in encode/transmit path).
  - `clockRateHz` very low values do not produce invalid timing behavior (duration remains bounded and non-zero).
- Transmit edge cases
  - Empty input span is a safe no-op (no spurious transmit/transaction calls unless explicitly intended by contract).
  - Repeated transmit calls with alternating payload sizes do not leak stale encoded data into output span.
- Readiness edge cases
  - Wrapped transport not-ready state forces `isReadyToUpdate()==false` even after reset interval elapsed.
  - Reset interval not elapsed forces `isReadyToUpdate()==false` even when wrapped transport reports ready.
  - Micros rollover scenario remains safe under unsigned subtraction semantics.
- Contract and compatibility checks
  - Wrapper preserves `TransportCategory = OneWireTransportTag` for protocol compatibility.
  - Compile-time constraints reject transports that are not `TaggedTransportLike<..., TransportTag>` or not config-constructible.
- Protocol-integration edge cases
  - `Ws2812xProtocol` channel-order strings longer than the color channel count (clamped by `resolveChannelCount`) do not cause wrapper encode size mismatch.
  - `pixelCount == 0` protocol updates produce zero-length source frames and safe wrapper behavior.

##### Suggested next step

- Add `test/transports/test_transport_onewirewrapper/` with a stub wrapped transport and deterministic fake time source (`micros`) to exercise encode, transaction, and readiness paths.

### Protocols Backlog

#### DotStarProtocol

##### Coverage snapshot

- No dedicated native tests currently target `src/protocols/DotStarProtocol.h`.
- Current protocol tests focus on debug wrappers and do not validate DotStar framing or serialization.

##### Positive tests

- Construction and lifecycle
  - `initialize()` calls `bus->begin()` exactly once.
  - `_endFrameExtraBytes` math matches `ceil(pixelCount / 16)` for representative counts (`0`, `1`, `15`, `16`, `17`, `32`).
- Serialization (`FixedBrightness` mode)
  - Per-pixel prefix byte is `0xFF`.
  - Channel payload follows configured `channelOrder` for `RGB`, `GRB`, and `BGR` inputs.
  - Serialized payload length equals `pixelCount * 4` bytes.
- Serialization (`Luminance` mode)
  - Per-pixel prefix byte is `0xE0 | 31` with current implementation.
  - Channel payload still follows configured `channelOrder`.
  - Mode switch only changes prefix behavior, not channel byte ordering.
- Framing and transaction behavior
  - `beginTransaction()` is called before any frame bytes.
  - Start frame transmits exactly 4 single-byte zero spans.
  - Pixel buffer transmits as a single contiguous span of expected length.
  - End frame transmits 4 fixed single-byte zeros plus extra `ceil(N/16)` zeros.
  - `endTransaction()` is called exactly once after all frame bytes.
- Readiness and update policy
  - `isReadyToUpdate()` delegates to transport readiness.
  - `alwaysUpdate()` returns `false`.

##### Negative tests

- P0
  - `update()` with `colors.size() > pixelCount` should be guarded; current implementation can overrun `_byteBuffer` during serialization.
  - `channelOrder` length shorter than 3 should be rejected or safely handled; current indexing reads first 3 chars unconditionally.
- Configuration edge cases
  - `channelOrder == nullptr` behavior is currently unsafe (direct indexing in update path); add regression coverage and decide contract.
  - Non-R/G/B characters in `channelOrder` map through `Color::indexFromChannel` fallback and should be explicitly documented/validated.
- Frame boundary cases
  - `pixelCount == 0` transmits start + end framing correctly with zero-length pixel payload.
  - Very large `pixelCount` values preserve end-frame extra-byte math without truncation anomalies.
- State and consistency
  - Repeated updates with different frame contents do not leak stale data between frames.
  - Transport not-ready state behavior remains external (protocol does not block); callers must gate via `isReadyToUpdate()`.

##### Suggested next step

- Add `test/protocols/test_protocol_dotstar/` with a transport spy that captures each transmit call and payload to assert framing and byte order.

#### Hd108Protocol

##### Coverage snapshot

- No dedicated native tests currently target `src/protocols/Hd108Protocol.h`.
- Implementation supports `uint16_t` components with `ChannelCount >= 3` and uses fixed start/end framing plus big-endian channel bytes.

##### Positive tests

- Construction and lifecycle
  - `initialize()` calls `bus->begin()` exactly once.
  - Internal frame payload size matches `pixelCount * (2 + ChannelCount * 2)`.
  - Alias instantiations compile and behave as expected for `Hd108RgbProtocol` and `Hd108RgbcwProtocol`.
- Serialization correctness
  - Each pixel starts with prefix bytes `0xFF, 0xFF`.
  - Channel values are emitted big-endian (`hi`, `lo`) per configured `channelOrder`.
  - `Rgb16Color` serializes exactly 3 channels; `Rgbcw16Color` serializes exactly 5 channels.
  - Serialized payload length equals `pixelCount * BytesPerPixel`.
- Framing and transaction behavior
  - `beginTransaction()` is called before any frame bytes.
  - Start frame transmits exactly 16 single-byte zero spans.
  - Pixel buffer transmits once as contiguous payload span.
  - End frame transmits exactly 4 single-byte `0xFF` spans.
  - `endTransaction()` is called exactly once after all frame bytes.
- Readiness and update policy
  - `isReadyToUpdate()` delegates to transport readiness.
  - `alwaysUpdate()` returns `false`.

##### Negative tests

- P0
  - `update()` with `colors.size() > pixelCount` should be guarded; current implementation can overrun `_byteBuffer` during serialization.
  - `channelOrder` length shorter than `ChannelCount` should be rejected or safely handled; current indexing assumes sufficient characters.
- Configuration edge cases
  - `channelOrder == nullptr` is unsafe in current code path (`_settings.channelOrder[ch]`); add regression coverage and define contract.
  - Unknown channel letters in `channelOrder` resolve via color fallback index behavior and should be explicitly validated.
- Data boundary cases
  - Channel extrema (`0x0000`, `0xFFFF`, mixed patterns) serialize to exact big-endian bytes.
  - `pixelCount == 0` transmits only framing (start + end) with zero-length payload.
  - Repeated updates with different frame contents do not leak stale payload bytes.
- Compile-time contract cases
  - Non-`uint16_t` component colors fail concept constraints.
  - Colors with channel count less than 3 fail concept constraints.

##### Suggested next step

- Add `test/protocols/test_protocol_hd108/` with table-driven vectors for channel-order and big-endian payload assertions.

#### Ws2801Protocol

##### Coverage snapshot

- No dedicated native tests currently target `src/protocols/Ws2801Protocol.h`.
- Implementation serializes raw 3-byte RGB payloads with no explicit start/end frames and enforces latch timing via `delayMicroseconds(500)`.

##### Positive tests

- Construction and lifecycle
  - `initialize()` calls `bus->begin()` exactly once.
  - Internal payload buffer size matches `pixelCount * 3` bytes.
- Serialization correctness
  - Serialized payload bytes follow configured `channelOrder` for `RGB`, `GRB`, and `BGR`.
  - Payload length equals `pixelCount * 3` for matching input frame size.
  - Multi-pixel serialization preserves per-pixel channel ordering across the frame.
- Transaction behavior
  - `beginTransaction()` is called once before payload transmit.
  - `transmitBytes()` is called once with contiguous payload span.
  - `endTransaction()` is called once after payload transmit.
- Latch and readiness behavior
  - `update()` records end timestamp and calls `delayMicroseconds(500)`.
  - `isReadyToUpdate()` returns false before latch window and true after latch window.
  - `alwaysUpdate()` returns `false`.

##### Negative tests

- P0
  - `update()` with `colors.size() > pixelCount` should be guarded; current implementation can overrun `_byteBuffer`.
  - `channelOrder` length shorter than 3 should be rejected or safely handled; current indexing assumes three valid characters.
- Configuration edge cases
  - `channelOrder == nullptr` is unsafe in current update path (`_settings.channelOrder[channel]`); add regression coverage and define contract.
  - Non-R/G/B channel characters map via color fallback index behavior and should be explicitly validated.
- Boundary/state cases
  - `pixelCount == 0` produces safe zero-length payload transmit behavior.
  - Repeated updates with different frame contents do not leak stale payload bytes.
  - Micros rollover remains safe for latch interval computation using unsigned subtraction.

##### Suggested next step

- Add `test/protocols/test_protocol_ws2801/` with a transport spy and fake time hooks (`micros`, `delayMicroseconds`) to validate framing-free payload and latch timing.

#### PixieProtocol

##### Coverage snapshot

- No dedicated native tests currently target `src/protocols/PixieProtocol.h`.
- Implementation is one-wire category, serializes 3-byte RGB payload, blocks in `update()` until ready, and uses 1000 µs latch gating.

##### Positive tests

- Construction and lifecycle
  - `initialize()` calls `bus->begin()` exactly once.
  - Internal payload buffer size matches `pixelCount * 3` bytes.
- Serialization correctness
  - Serialized payload bytes follow configured `channelOrder` for `RGB`, `GRB`, and `BGR`.
  - Payload length equals `pixelCount * 3` for matching input frame size.
  - Multi-pixel ordering is stable across full frame serialization.
- Transaction behavior
  - `beginTransaction()` is called once before payload transmit.
  - `transmitBytes()` is called once with contiguous payload span.
  - `endTransaction()` is called once after payload transmit.
- Readiness and update policy
  - `update()` waits (yield loop) until `isReadyToUpdate()` becomes true before serializing/transmitting.
  - `isReadyToUpdate()` requires both transport readiness and latch interval (`>= 1000 µs`) since last update.
  - `alwaysUpdate()` returns `true`.

##### Negative tests

- P0
  - `update()` with `colors.size() > pixelCount` should be guarded; current implementation can overrun `_byteBuffer`.
  - `channelOrder` length shorter than 3 should be rejected or safely handled; current indexing assumes three valid characters.
- Configuration edge cases
  - `channelOrder == nullptr` is unsafe in current update path (`_settings.channelOrder[channel]`); add regression coverage and define contract.
  - Non-R/G/B channel characters map via color fallback index behavior and should be explicitly validated.
- Blocking/readiness edge cases
  - If transport never becomes ready, `update()` can block indefinitely; add watchdog-style test strategy to document expected behavior and mitigation.
  - Latch not elapsed must keep protocol not-ready even when transport reports ready.
  - Transport not-ready must keep protocol not-ready even after latch elapsed.
- Boundary/state cases
  - `pixelCount == 0` yields safe zero-length transmit behavior once ready.
  - Repeated updates with alternating frame sizes/content do not leak stale bytes.
  - Micros rollover remains safe for latch interval computation using unsigned subtraction.

##### Suggested next step

- Add `test/protocols/test_protocol_pixie/` with a transport spy plus deterministic fake `micros()` and `yield()` observation to validate readiness gating and serialization.

#### Lpd6803Protocol

##### Coverage snapshot

- No dedicated native tests currently target `src/protocols/Lpd6803Protocol.h`.
- Implementation packs RGB into 5-5-5 (bit15 set), frames with fixed 4-byte zero start frame and `ceil(N/8)` zero end-frame bytes.

##### Positive tests

- Construction and lifecycle
  - `initialize()` calls `bus->begin()` exactly once.
  - Internal pixel payload buffer size equals `pixelCount * 2` bytes.
  - End-frame size calculation matches `ceil(pixelCount / 8)` for representative counts (`0`, `1`, `7`, `8`, `9`, `16`).
- Serialization correctness
  - Per-pixel packed word sets bit 15 and uses top 5 bits of each ordered channel.
  - Packed bytes are transmitted big-endian (`hi` then `lo`).
  - `channelOrder` variants (`RGB`, `GRB`, `BGR`) produce expected packed outputs.
  - Payload length equals `pixelCount * 2` for matching input frame size.
- Framing and transaction behavior
  - `beginTransaction()` occurs before any transmit.
  - Start frame transmits exactly 4 single-byte `0x00` spans.
  - Pixel payload transmits once as a contiguous span.
  - End frame transmits exactly `ceil(N/8)` single-byte `0x00` spans.
  - `endTransaction()` occurs once after framing and payload.
- Readiness and update policy
  - `isReadyToUpdate()` delegates to transport readiness.
  - `alwaysUpdate()` returns `false`.

##### Negative tests

- P0
  - `update()` with `colors.size() > pixelCount` should be guarded; current implementation can overrun `_byteBuffer`.
  - `channelOrder` shorter than 3 is unsafe; current indexing assumes three accessible characters.
- Configuration edge cases
  - `channelOrder == nullptr` is unsafe in current update path and should be covered as a regression/contract test.
  - Unknown channel letters should be explicitly validated against color fallback indexing behavior.
- Boundary/state cases
  - `pixelCount == 0` yields safe start/end framing with zero-length pixel payload.
  - Repeated updates with changing frame contents do not leak stale bytes in transmitted payload.
  - Large `pixelCount` values preserve `ceil(N/8)` end-frame math without truncation anomalies.

##### Suggested next step

- Add `test/protocols/test_protocol_lpd6803/` with table-driven packed-word vectors and transmit-call sequencing assertions.

#### Lpd8806Protocol

##### Coverage snapshot

- No dedicated native tests currently target `src/protocols/Lpd8806Protocol.h`.
- Implementation emits per-channel `(value >> 1) | 0x80`, with both start and end frame lengths equal to `ceil(N/32)`.

##### Positive tests

- Construction and lifecycle
  - `initialize()` calls `bus->begin()` exactly once.
  - Internal payload buffer size equals `pixelCount * 3` bytes.
  - Frame-size calculation matches `ceil(pixelCount / 32)` for representative counts (`0`, `1`, `31`, `32`, `33`, `64`).
- Serialization correctness
  - Each serialized channel byte equals `(orderedChannel >> 1) | 0x80`.
  - `channelOrder` variants (`RGB`, `GRB`, `BGR`) produce expected payload ordering.
  - Serialized payload length equals `pixelCount * 3` for matching input frame size.
  - Channel edge values map correctly (`0x00 -> 0x80`, `0xFF -> 0xFF`).
- Framing and transaction behavior
  - `beginTransaction()` occurs before any transmit.
  - Start frame transmits exactly `ceil(N/32)` single-byte `0x00` spans.
  - Pixel payload transmits once as contiguous span.
  - End frame transmits exactly `ceil(N/32)` single-byte `0xFF` spans.
  - `endTransaction()` occurs once after framing and payload.
- Readiness and update policy
  - `isReadyToUpdate()` delegates to transport readiness.
  - `alwaysUpdate()` returns `false`.

##### Negative tests

- P0
  - `update()` with `colors.size() > pixelCount` should be guarded; current implementation can overrun `_byteBuffer`.
  - `channelOrder` shorter than 3 is unsafe; current indexing assumes three accessible characters.
- Configuration edge cases
  - `channelOrder == nullptr` is unsafe in current update path and should be covered as a regression/contract test.
  - Unknown channel letters should be explicitly validated against color fallback indexing behavior.
- Boundary/state cases
  - `pixelCount == 0` produces safe behavior (zero frame bytes on both sides and zero-length payload transmit).
  - Repeated updates with changing content do not leak stale payload bytes.
  - Large `pixelCount` values preserve `ceil(N/32)` frame math without truncation anomalies.

##### Suggested next step

- Add `test/protocols/test_protocol_lpd8806/` with transport-spy capture for framing-byte counts and table-driven channel transform assertions.

#### P9813Protocol

##### Coverage snapshot

- No dedicated native tests currently target `src/protocols/P9813Protocol.h`.
- Implementation uses fixed BGR wire order with checksum header and fixed 4-byte start/end zero framing.

##### Positive tests

- Construction and lifecycle
  - `initialize()` calls `bus->begin()` exactly once.
  - Internal payload buffer size equals `pixelCount * 4` bytes.
- Serialization correctness
  - Header byte matches `0xC0 | ((~B>>6)&0x03)<<4 | ((~G>>6)&0x03)<<2 | ((~R>>6)&0x03)`.
  - Data bytes are emitted in fixed B, G, R order.
  - Serialized payload length equals `pixelCount * 4` for matching input frame size.
  - Multi-pixel serialization preserves per-pixel header/data grouping.
- Framing and transaction behavior
  - `beginTransaction()` occurs before any transmit.
  - Start frame transmits exactly 4 single-byte `0x00` spans.
  - Pixel payload transmits once as contiguous span.
  - End frame transmits exactly 4 single-byte `0x00` spans.
  - `endTransaction()` occurs once after framing and payload.
- Readiness and update policy
  - `isReadyToUpdate()` delegates to transport readiness.
  - `alwaysUpdate()` returns `false`.

##### Negative tests

- P0
  - `update()` with `colors.size() > pixelCount` should be guarded; current implementation can overrun `_byteBuffer`.
- Boundary/state cases
  - `pixelCount == 0` transmits start/end framing with zero-length payload safely.
  - Repeated updates with differing content do not leak stale payload bytes.
  - Header generation for extreme channel values (`0x00`, `0xFF`, mixed) remains correct.

##### Suggested next step

- Add `test/protocols/test_protocol_p9813/` with table-driven header-byte vectors and transmit-call sequencing assertions.

#### Sm168xProtocol

##### Coverage snapshot

- No dedicated native tests currently target `src/protocols/Sm168xProtocol.h`.
- Implementation supports 3/4/5-channel variants with variant-specific settings trailer packing.

##### Positive tests

- Construction and lifecycle
  - `initialize()` calls `bus->begin()` exactly once.
  - Variant channel count resolution is correct (`3`, `4`, `5`).
  - Variant settings trailer size resolution is correct (`2`, `2`, `4`).
  - Total frame size equals `pixelCount * resolvedChannelCount + resolvedSettingsSize`.
- Pixel payload serialization
  - Payload bytes follow configured `channelOrder` for each variant.
  - Unused payload bytes remain zero-filled when frame color count is below `pixelCount`.
  - Payload and settings sections do not overlap.
- Settings encoding correctness
  - Three-channel settings pack as `[IC0][IC1:high nibble | IC2:low nibble]`.
  - Four-channel settings pack as `[IC0|IC1][IC2|IC3]` nibble pairs.
  - Five-channel settings pack to 4 bytes using current bit layout and tail `0b00011111` bits.
  - Gain masking rules apply (`0x0F` for 3/4-ch, `0x1F` for 5-ch).
- Transaction and policy behavior
  - `beginTransaction()`/`transmitBytes()`/`endTransaction()` each occur once per update.
  - `isReadyToUpdate()` delegates to transport readiness.
  - `alwaysUpdate()` returns `false`.

##### Negative tests

- P0
  - `update()` with `colors.size() > pixelCount` should be guarded; current implementation can overrun payload region.
  - `channelOrder` length shorter than resolved channel count is unsafe; current indexing assumes required characters exist.
- Configuration edge cases
  - `channelOrder == nullptr` is unsafe in current code path; add regression coverage and define contract.
  - Unknown channel letters map via color fallback indexing; validate explicitly to avoid silent remap regressions.
  - Gains outside legal range are masked, not rejected; verify masking behavior remains stable.
- Variant mismatch cases
  - Variant channel count greater than `TColor::ChannelCount` should be rejected or documented; current indexing may alias fallback channels.
  - Five-channel variant with non-5-channel color aliases should be explicitly covered as contract behavior.
- Compile-time contract cases
  - Non-`uint8_t` color component types fail static assertions.
  - Channel counts outside 3..5 fail static assertions.

##### Suggested next step

- Add `test/protocols/test_protocol_sm168x/` with variant-parameterized vectors for payload + settings trailer encoding.

#### Sm16716Protocol

##### Coverage snapshot

- No dedicated native tests currently target `src/protocols/Sm16716Protocol.h`.
- Implementation pre-packs a non-byte-aligned bitstream with 50 zero start bits and 25 bits per pixel.

##### Positive tests

- Construction and lifecycle
  - `initialize()` calls `bus->begin()` exactly once.
  - Buffer size matches `ceil((50 + pixelCount * 25) / 8)`.
- Bitstream serialization correctness
  - First 50 bits are zero in serialized output.
  - Each pixel contributes one high separator bit followed by 3 channel bytes in configured order.
  - Bit packing is MSB-first across byte boundaries.
  - Table-driven vectors verify known pixel inputs at boundary positions (where bytes split).
- Transaction and policy behavior
  - `beginTransaction()`/`transmitBytes()`/`endTransaction()` each occur once per update.
  - Entire stream transmits in one contiguous payload span.
  - `isReadyToUpdate()` delegates to transport readiness.
  - `alwaysUpdate()` returns `false`.

##### Negative tests

- P0
  - `update()` with `colors.size() > pixelCount` should be guarded; current serialization can write beyond allocated bitstream.
  - `channelOrder` shorter than 3 is unsafe; current indexing assumes three accessible characters.
- Configuration edge cases
  - `channelOrder == nullptr` is unsafe in current update path; add regression coverage and define contract.
  - Unknown channel letters map via fallback indexing; validate explicitly.
- Bit packing boundary cases
  - Pixel boundaries that land at bit offsets near byte ends pack correctly without shifted/corrupted carry.
  - `pixelCount == 0` still emits valid 50-bit start-frame-only stream.
  - Repeated updates with smaller/larger logical frames do not leak stale bits.

##### Suggested next step

- Add `test/protocols/test_protocol_sm16716/` with bit-accurate golden vectors and byte-boundary stress cases.

#### Tlc5947Protocol

##### Coverage snapshot

- No dedicated native tests currently target `src/protocols/Tlc5947Protocol.h`.
- Implementation transforms 16-bit input to 12-bit channels, packs 24 channels/module into 36 bytes, and supports pixel/tail-fill strategies.

##### Positive tests

- Construction and lifecycle
  - `initialize()` calls `bus->begin()` exactly once.
  - Active channel count resolves correctly for all pixel strategies (`UseColorChannelCount`, `ForceRgb`, `ForceRgbw`, `ForceRgbcw`).
  - Module count and payload size calculations match expected values across representative pixel counts.
- Serialization correctness
  - 16-bit input values are narrowed to 12-bit via `value >> 4`.
  - Two 12-bit channels are packed into 3 bytes in expected order.
  - Channel ordering falls back to default channel map when `channelOrder` is null or shorter than required.
  - Module channel stream is reversed as specified before packing.
- Tail fill behavior
  - `Zero` leaves tail channels at zero.
  - `RepeatFirstPixel` fills remaining module channels from first pixel in module.
  - `RepeatLastPixel` fills remaining module channels from last valid pixel in module.
- Transaction and policy behavior
  - `beginTransaction()`/`transmitBytes()`/`endTransaction()` each occur once per update.
  - `isReadyToUpdate()` current behavior returns `true`.
  - `alwaysUpdate()` returns `false`.

##### Negative tests

- P0
  - `isReadyToUpdate()` currently ignores transport readiness; add explicit regression test to capture current contract and flag if behavior changes.
- Configuration edge cases
  - Invalid `channelOrder` characters map via color index fallback and should be explicitly validated.
  - `pixelCount == 0` yields safe no-data behavior without crashes.
  - Force strategies on lower-channel color aliases (`ForceRgbw`/`ForceRgbcw`) clamp active channels as designed.
- Tail and boundary cases
  - Partial final module with each tail-fill strategy produces deterministic packed output.
  - Empty color span with non-zero `pixelCount` does not read invalid memory and still yields valid frame structure.

##### Suggested next step

- Add `test/protocols/test_protocol_tlc5947/` with table-driven pack vectors and strategy matrix coverage.

#### Tlc59711Protocol

##### Coverage snapshot

- No dedicated native tests currently target `src/protocols/Tlc59711Protocol.h`.
- Implementation builds a per-chip 4-byte header + 24-byte data block, transmits chips in reverse order, and applies 20 µs latch guard.

##### Positive tests

- Construction and lifecycle
  - `initialize()` calls `bus->begin()` exactly once.
  - Chip count resolves to `ceil(pixelCount / 4)` and payload size equals `chipCount * 28`.
- Header encoding
  - Header bits reflect control flags (`outtmg`, `extgck`, `tmgrst`, `dsprpt`, `blank`) correctly.
  - Brightness fields (`bcRed`, `bcGreen`, `bcBlue`) are masked to 7 bits and packed correctly.
  - `updateConfig()` changes header bytes used on subsequent updates.
- Data serialization correctness
  - Chip order is reversed on wire (last logical chip first).
  - Pixel order is reversed within each chip.
  - Channel order is BGR per pixel.
  - 8-bit channel values are expanded to 16-bit by byte replication.
  - Out-of-range pixels (padding in last chip) serialize as zeros.
- Transaction and policy behavior
  - `beginTransaction()`/`transmitBytes()`/`endTransaction()` each occur once per update.
  - `delayMicroseconds(20)` is called for latch guard.
  - `isReadyToUpdate()` current behavior returns `true`.
  - `alwaysUpdate()` returns `false`.

##### Negative tests

- P0
  - `isReadyToUpdate()` currently ignores transport readiness; add regression test to capture current contract and detect accidental behavior shifts.
- Configuration edge cases
  - Extreme brightness inputs (`0`, `127`, `>127`) remain masked and packed predictably.
  - `pixelCount == 0` produces zero-length payload behavior without invalid accesses.
- Boundary/state cases
  - Colors span shorter than configured pixel count results in zero-filled padded channels only.
  - Repeated updates with changed config and color frames do not leak stale header/data bytes.

##### Suggested next step

- Add `test/protocols/test_protocol_tlc59711/` with chip-order vectors, header bitfield checks, and latch-guard call assertions.

#### Tm1814Protocol

##### Coverage snapshot

- No dedicated native tests currently target `src/protocols/Tm1814Protocol.h`.
- Implementation prefixes 8-byte current settings block (with inverse bytes), then pixel payload in configurable WRGB order, with ready-wait loop in `update()`.

##### Positive tests

- Construction and lifecycle
  - `initialize()` calls `bus->begin()` exactly once.
  - Frame buffer size equals `8 + pixelCount * 4`.
- Settings encoding correctness
  - Current values are clamped to `[65, 380]` mA before encoding.
  - Encoded current uses `(limited - 65) / 5`.
  - First 4 settings bytes follow configured channel order.
  - Next 4 settings bytes are bitwise inverse of first 4 bytes.
- Pixel serialization correctness
  - Pixel bytes follow configured 4-channel order.
  - Payload length matches `pixelCount * 4` for matching input frame size.
- Readiness and policy behavior
  - `update()` waits until transport ready before transmit.
  - `isReadyToUpdate()` delegates to transport readiness.
  - `alwaysUpdate()` returns `false`.

##### Negative tests

- P0
  - `update()` with `colors.size() > pixelCount` should be guarded; current implementation can overrun `_frameBuffer`.
  - `channelOrder` shorter than 4 is unsafe; current indexing assumes four accessible characters.
- Configuration edge cases
  - `channelOrder == nullptr` is unsafe in current update path and should be covered as a regression/contract case.
  - Unknown channel letters currently fall through to white current mapping in settings path; validate explicitly.
- Blocking/readiness edge cases
  - Transport permanently not-ready causes indefinite wait loop in `update()`; document and test mitigation expectations.
  - `pixelCount == 0` still transmits settings-only frame safely when ready.

##### Suggested next step

- Add `test/protocols/test_protocol_tm1814/` with deterministic transport readiness control and table-driven settings/payload vectors.

#### Tm1914Protocol

##### Coverage snapshot

- No dedicated native tests currently target `src/protocols/Tm1914Protocol.h`.
- Implementation emits 6-byte settings preamble (`FF FF mode ~FF ~FF ~mode`) plus RGB payload and waits for transport readiness before transmit.

##### Positive tests

- Construction and lifecycle
  - `initialize()` calls `bus->begin()` exactly once.
  - Frame buffer size equals `6 + pixelCount * 3`.
- Mode and settings encoding
  - Mode byte maps correctly (`DinFdinAutoSwitch=0xFF`, `DinOnly=0xF5`, `FdinOnly=0xFA`).
  - Inverse settings bytes are correct (`~0xFF`, `~0xFF`, `~mode`).
- Pixel serialization correctness
  - Pixel bytes follow configured `channelOrder`.
  - Payload length matches `pixelCount * 3` for matching input frame size.
- Readiness and policy behavior
  - `update()` waits until transport ready before transmit.
  - `isReadyToUpdate()` delegates to transport readiness.
  - `alwaysUpdate()` returns `false`.

##### Negative tests

- P0
  - `update()` with `colors.size() > pixelCount` should be guarded; current implementation can overrun `_frameBuffer`.
  - `channelOrder` shorter than 3 is unsafe; current indexing assumes three accessible characters.
- Configuration edge cases
  - `channelOrder == nullptr` is unsafe in current update path and should be covered as a regression/contract case.
  - Unknown channel letters map via color fallback indexing and should be explicitly validated.
- Blocking/readiness edge cases
  - Transport permanently not-ready causes indefinite wait loop in `update()`; document and test mitigation expectations.
  - `pixelCount == 0` yields settings-only frame behavior without payload overflow.

##### Suggested next step

- Add `test/protocols/test_protocol_tm1914/` with mode-matrix settings validation and readiness-gated update tests.

#### Ws2812xProtocol

##### Coverage snapshot

- No dedicated native tests currently target `src/protocols/Ws2812xProtocol.h` directly.
- Current plan includes wrapper-level integration checks, but not protocol-level serialization and contract coverage.

##### Positive tests

- Construction and lifecycle
  - `initialize()` calls `bus->begin()` exactly once.
  - Frame size resolves to `pixelCount * resolvedChannelCount`.
  - Constructor overloads (settings object vs direct `(pixelCount, channelOrder, transport)`) produce equivalent behavior.
- Channel order and count resolution
  - `channelOrder == nullptr` resolves to default `GRB`.
  - Empty `channelOrder` string resolves channel count to `LengthGRB`.
  - `resolveChannelCount` clamps to `min(strlen(channelOrder), TColor::ChannelCount)`.
  - Overlong channel-order strings do not exceed color channel count in output frame sizing.
- Serialization correctness
  - 8-bit colors serialize channel bytes in requested order.
  - 16-bit colors serialize high byte only (`value >> 8`) per channel.
  - 3/4/5 channel color aliases serialize with correct byte count and ordering.
  - `colors.size() == pixelCount` produces exactly full-frame payload.
- Readiness and policy behavior
  - `update()` waits (yield loop) until `isReadyToUpdate()` is true.
  - `isReadyToUpdate()` delegates to transport readiness.
  - `alwaysUpdate()` returns `false`.

##### Negative tests

- P0
  - `update()` with `colors.size() > pixelCount` should be guarded; current implementation can overrun allocated frame buffer.
  - `malloc` failure path (`_data == nullptr`) is not currently guarded in `update()`/`serialize`; add regression coverage and define expected behavior.
- Configuration edge cases
  - `channelOrder` shorter than required channel count is currently accepted and falls back through color indexing behavior; validate and document this contract.
  - Unknown channel characters in `channelOrder` map via color fallback indexing and should be explicitly validated.
- Blocking/readiness edge cases
  - Transport permanently not-ready causes indefinite wait loop in `update()`; document and test mitigation expectations.
  - `pixelCount == 0` should produce safe zero-length transmit behavior.
- State consistency
  - Repeated updates with varying frame content do not leak stale bytes.
  - 8-bit vs 16-bit type parity for equal wire-byte expectations remains stable across channel-order variants.

##### Suggested next step

- Add `test/protocols/test_protocol_ws2812x/` with a transport spy and table-driven vectors for channel-order/count resolution and 8-bit/16-bit serialization parity.

---

## Phased Implementation

### Phase 1 — Environment bootstrap

1. Add `native-test` env in `platformio.ini`
2. Add Unity + ArduinoFake dependencies
3. Validate with an empty smoke test

### Phase 2 — Test structure

1. Create `test/support` utilities
2. Create `busses`, `shaders`, `protocols` test roots
3. Add one smoke test package per category

### Phase 3 — Baseline coverage

1. Add core assertions for each category
2. Ensure deterministic execution on host
3. Document category-specific patterns in test files

### Phase 4 — CI integration

1. Add `pio test -e native-test` to CI
2. Keep firmware build as separate CI step
3. Gate PRs on native test pass

---

## Definition of Done

- Native test environment runs locally with a single command
- Category folders exist and are discoverable by PlatformIO
- Shared fake setup utilities are used by tests
- At least one passing smoke test exists for each category
- CI runs native tests on each PR

---

## Risks and Mitigations

- Risk: host/native differences from embedded runtime behavior.
  - Mitigation: treat native tests as logic verification only; keep on-device validation for timing-critical paths.

- Risk: brittle tests due to over-mocking internals.
  - Mitigation: mock only Arduino boundaries and assert public behavior.

- Risk: dependency/API drift in ArduinoFake.
  - Mitigation: centralize wrappers in `test/support` and avoid direct deep coupling in every test.
