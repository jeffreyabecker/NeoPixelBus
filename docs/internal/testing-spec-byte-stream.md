# Byte-Stream Test Specification (Protocols + OneWireWrapper)

Source inputs:
- `docs/internal/testing-plan-native-unity-arduinofake.md`
- Protocol and transport implementations under `src/protocols/` and `src/transports/`

Purpose:
Define tests that validate the exact byte stream (content, order, length, and framing) sent to the underlying transport.

Related focused specifications:
- `docs/internal/testing-spec-transports.md`
- `docs/internal/testing-spec-protocols.md`
- `docs/internal/testing-spec-hierarchical.md` (index)

## 1. Protocol Byte-Stream Tests

### 1.1 Common Transport-Capture Harness

#### 1.1.1 Transport Capture Baseline
- Description: Verify test harness can capture each `transmitBytes()` call as ordered records.
- Preconditions:
  - Transport spy records each call as `{sequence, bytes[]}`.
  - Protocol under test connected to spy transport.
- Operations:
  - Execute one protocol `update()` with a single-pixel frame.
- Expected Results:
  - Harness captures all transmit calls in strict order.
  - Captured payload bytes match total emitted bytes.

#### 1.1.2 Transaction Boundary Capture
- Description: Verify begin/end transaction markers are captured around payload emissions.
- Preconditions:
  - Transport spy tracks `beginTransaction()` and `endTransaction()` call indices.
- Operations:
  - Execute one update for each protocol family (framed and unframed).
- Expected Results:
  - Transaction boundaries align with expected protocol behavior.

### 1.2 DotStarProtocol

#### 1.2.1 FixedBrightness Pixel Stream Bytes
- Description: Validate DotStar fixed-brightness payload bytes.
- Preconditions:
  - Pixel vector with distinct RGB values.
  - `DotStarMode::FixedBrightness`.
  - Channel order variants (`RGB`, `GRB`, `BGR`).
- Operations:
  - Call `update()` and capture emitted bytes.
- Expected Results:
  - Per-pixel bytes are `[0xFF, ch1, ch2, ch3]` with channels in configured order.
  - Payload length equals `pixelCount * 4` bytes.

#### 1.2.2 Luminance Prefix Bytes
- Description: Validate DotStar luminance prefix encoding.
- Preconditions:
  - Mode set to `Luminance`.
- Operations:
  - Call `update()` and inspect payload prefixes.
- Expected Results:
  - Each pixel prefix equals `0xE0 | 31` (current implementation).
  - Channel bytes remain ordered per `channelOrder`.

#### 1.2.3 Start/End Frame Byte Counts
- Description: Verify framing bytes around DotStar payload.
- Preconditions:
  - Pixel counts around boundary values (`0`, `1`, `15`, `16`, `17`).
- Operations:
  - Call `update()` and count single-byte frame emissions.
- Expected Results:
  - Start frame emits 4 bytes of `0x00`.
  - End frame emits `4 + ceil(N/16)` bytes of `0x00`.

### 1.3 Hd108Protocol

#### 1.3.1 Per-Pixel Prefix and Channel Bytes
- Description: Validate HD108 pixel encoding bytes.
- Preconditions:
  - `Rgb16Color` and `Rgbcw16Color` test vectors.
  - Deterministic channel-order strings.
- Operations:
  - Execute `update()` and capture payload region.
- Expected Results:
  - Each pixel begins with `[0xFF, 0xFF]`.
  - Each channel encoded as big-endian `[hi, lo]` in configured order.

#### 1.3.2 Frame Byte Structure
- Description: Verify start/payload/end byte stream structure.
- Preconditions:
  - Non-zero and zero pixel-count test cases.
- Operations:
  - Execute `update()` and inspect call sequence.
- Expected Results:
  - Start frame emits 16 bytes of `0x00`.
  - Payload emitted once as contiguous span.
  - End frame emits 4 bytes of `0xFF`.

### 1.4 Ws2801Protocol

#### 1.4.1 Raw Payload Byte Order
- Description: Validate WS2801 payload byte ordering.
- Preconditions:
  - Known RGB vectors and channel-order variants.
- Operations:
  - Execute `update()` and capture transmit payload.
- Expected Results:
  - Payload is raw 3-byte pixels in configured channel order.
  - Payload length equals `pixelCount * 3`.

#### 1.4.2 Framing Absence
- Description: Confirm WS2801 emits no explicit start/end frame bytes.
- Preconditions:
  - Transport capture enabled.
- Operations:
  - Execute `update()`.
- Expected Results:
  - Only one payload transmit call is emitted (excluding transaction markers).

### 1.5 PixieProtocol

#### 1.5.1 Payload Byte Stream
- Description: Validate Pixie payload stream bytes.
- Preconditions:
  - Known vectors and channel-order variants.
- Operations:
  - Execute `update()` after readiness is true.
- Expected Results:
  - Payload bytes equal configured channel order per pixel.
  - Payload length equals `pixelCount * 3`.

#### 1.5.2 No Extra Framing Bytes
- Description: Confirm Pixie stream does not prepend/append explicit frame bytes.
- Preconditions:
  - Transport capture enabled.
- Operations:
  - Execute one update.
- Expected Results:
  - Exactly one payload transmit with no additional framing transmit calls.

### 1.6 Lpd6803Protocol

#### 1.6.1 5-5-5 Packed Pixel Bytes
- Description: Validate packed 16-bit words emitted as big-endian bytes.
- Preconditions:
  - Vectors with non-trivial RGB values.
  - Channel-order variants.
- Operations:
  - Execute `update()` and inspect payload bytes.
- Expected Results:
  - Each pixel emits 2 bytes matching `0x8000 | (ch1&0xF8)<<7 | (ch2&0xF8)<<2 | (ch3>>3)`.

#### 1.6.2 Start/End Frame Byte Stream
- Description: Verify LPD6803 framing byte stream.
- Preconditions:
  - Pixel counts around 8-pixel boundaries.
- Operations:
  - Execute `update()` and count framing bytes.
- Expected Results:
  - Start emits 4 bytes of `0x00`.
  - End emits `ceil(N/8)` bytes of `0x00`.

### 1.7 Lpd8806Protocol

#### 1.7.1 7-Bit+MSB Payload Bytes
- Description: Validate LPD8806 channel transform bytes.
- Preconditions:
  - Values including `0x00`, `0x01`, `0xFE`, `0xFF`.
- Operations:
  - Execute `update()` and inspect payload.
- Expected Results:
  - Each channel byte equals `(value >> 1) | 0x80`.

#### 1.7.2 Symmetric Frame Stream
- Description: Verify frame-byte stream before/after payload.
- Preconditions:
  - Pixel counts around 32-pixel boundaries.
- Operations:
  - Execute `update()` and count frame transmit calls.
- Expected Results:
  - Start emits `ceil(N/32)` bytes of `0x00`.
  - End emits `ceil(N/32)` bytes of `0xFF`.

### 1.8 P9813Protocol

#### 1.8.1 Header + BGR Payload Bytes
- Description: Validate P9813 per-pixel 4-byte packet.
- Preconditions:
  - RGB vectors covering top-bit combinations.
- Operations:
  - Execute `update()` and inspect pixel packets.
- Expected Results:
  - Header byte equals `0xC0 | ((~B>>6)&0x03)<<4 | ((~G>>6)&0x03)<<2 | ((~R>>6)&0x03)`.
  - Data bytes are `[B, G, R]`.

#### 1.8.2 Fixed Frame Bytes
- Description: Verify fixed start/end frame stream bytes.
- Preconditions:
  - Non-zero and zero pixel-count cases.
- Operations:
  - Execute `update()` and inspect frame calls.
- Expected Results:
  - Start emits 4 bytes `0x00`.
  - End emits 4 bytes `0x00`.

### 1.9 Sm168xProtocol

#### 1.9.1 Variant Payload Bytes
- Description: Validate payload region bytes for 3/4/5-channel variants.
- Preconditions:
  - Variant matrix with matching color aliases.
- Operations:
  - Execute `update()` and split frame into payload/trailer segments.
- Expected Results:
  - Payload bytes match channel-order mapping for active channel count.

#### 1.9.2 Settings Trailer Bytes
- Description: Validate gain trailer bit-packing bytes per variant.
- Preconditions:
  - Gain vectors including out-of-range values.
- Operations:
  - Execute `update()` and inspect final settings bytes.
- Expected Results:
  - 3-channel trailer: `[IC0][(IC1<<4)|IC2]`.
  - 4-channel trailer: `[(IC0<<4)|IC1][(IC2<<4)|IC3]`.
  - 5-channel trailer: bytes match documented bit layout with tail bits `0b00011111`.
  - Gain masks are applied (`0x0F` for 3/4-ch, `0x1F` for 5-ch).

### 1.10 Sm16716Protocol

#### 1.10.1 Bitstream Packing Bytes
- Description: Validate SM16716 packed bitstream bytes.
- Preconditions:
  - Bit-accurate golden vectors for boundary-crossing cases.
- Operations:
  - Execute `update()` and compare captured bytes to golden stream.
- Expected Results:
  - First 50 bits are zero.
  - Each pixel contributes 1 separator bit + 24 channel bits.
  - Packing is MSB-first across byte boundaries.

### 1.11 Tlc5947Protocol

#### 1.11.1 Module Byte Stream (12-bit Packed)
- Description: Validate TLC5947 module byte payload stream.
- Preconditions:
  - 16-bit color vectors and strategy matrix.
- Operations:
  - Execute `update()` and inspect payload by module.
- Expected Results:
  - Values narrowed to 12 bits (`>>4`).
  - Two channels packed into 3 bytes in expected reverse order.
  - Module payload length is 36 bytes each.

#### 1.11.2 Tail Fill Byte Effects
- Description: Verify tail-fill strategy effects in emitted bytes.
- Preconditions:
  - Partial final-module scenarios for all tail-fill strategies.
- Operations:
  - Execute `update()` with each strategy and compare tail regions.
- Expected Results:
  - Tail bytes match `Zero`, `RepeatFirstPixel`, or `RepeatLastPixel` policy.

### 1.12 Tlc59711Protocol

#### 1.12.1 Header Byte Stream
- Description: Validate per-chip 4-byte header serialization.
- Preconditions:
  - Config matrix for control flags and brightness fields.
- Operations:
  - Execute `update()` and inspect each chip header in stream.
- Expected Results:
  - Header bytes match expected bitfield packing and 7-bit brightness masks.

#### 1.12.2 Reversed Chip/Pixel Payload Stream
- Description: Validate chip and pixel reverse ordering in emitted payload bytes.
- Preconditions:
  - Multi-chip frame vectors with unique per-pixel values.
- Operations:
  - Execute `update()` and map stream back to logical chip/pixel order.
- Expected Results:
  - Stream is last-chip-first.
  - Within each chip, pixels are serialized in reverse order.
  - Channel bytes are BGR, 16-bit big-endian (8-bit replicated to 16-bit).

### 1.13 Tm1814Protocol

#### 1.13.1 Settings Preamble Bytes
- Description: Validate TM1814 first 8 bytes settings preamble.
- Preconditions:
  - Current settings vectors including clamp boundaries.
- Operations:
  - Execute `update()` and inspect first 8 bytes.
- Expected Results:
  - Bytes 0..3 are encoded currents per channel-order.
  - Bytes 4..7 are bitwise inverse of bytes 0..3.

#### 1.13.2 Pixel Payload Bytes
- Description: Validate 4-channel payload byte order and length.
- Preconditions:
  - Known `Rgbw8Color` frame and channel order.
- Operations:
  - Execute `update()` and inspect payload region.
- Expected Results:
  - Payload bytes match configured order.
  - Payload length equals `pixelCount * 4`.

### 1.14 Tm1914Protocol

#### 1.14.1 Mode Preamble Bytes
- Description: Validate TM1914 6-byte preamble encoding.
- Preconditions:
  - Mode matrix for all enum values.
- Operations:
  - Execute `update()` and inspect bytes 0..5.
- Expected Results:
  - Bytes `[0,1]=0xFF,0xFF`.
  - Byte 2 equals encoded mode (`0xFF`, `0xF5`, `0xFA`).
  - Bytes 3..5 are bitwise inverse of bytes 0..2.

#### 1.14.2 RGB Payload Bytes
- Description: Validate payload byte stream and order.
- Preconditions:
  - Known RGB frame and channel-order variants.
- Operations:
  - Execute `update()` and inspect payload.
- Expected Results:
  - Payload bytes match configured channel order and expected length.

### 1.15 Ws2812xProtocol

#### 1.15.1 Channel-Order/Count Resolved Byte Length
- Description: Validate output payload length from resolved channel count.
- Preconditions:
  - `channelOrder` matrix: null, empty, short, exact, overlong.
  - Color alias/type matrix (3/4/5 channels, 8-bit and 16-bit).
- Operations:
  - Execute `update()` and capture transmitted payload length.
- Expected Results:
  - Length equals `pixelCount * resolvedChannelCount`.
  - `resolvedChannelCount = min(strlen(orderResolved), TColor::ChannelCount)` with empty-order fallback to GRB length.

#### 1.15.2 8-bit and 16-bit Wire Component Bytes
- Description: Validate per-component byte conversion in wire stream.
- Preconditions:
  - Paired 8-bit and 16-bit vectors for equivalent logical values.
- Operations:
  - Execute updates and compare payload bytes.
- Expected Results:
  - 8-bit values transmitted directly.
  - 16-bit values transmit high byte (`value >> 8`).

#### 1.15.3 Payload Byte Sequence by Order
- Description: Validate exact per-pixel byte sequence for each channel-order variant.
- Preconditions:
  - Distinct per-channel values to detect swaps.
- Operations:
  - Execute `update()` with order variants and compare payload.
- Expected Results:
  - Byte sequence matches configured channel order exactly.

## 2. OneWireWrapper Byte-Stream Tests

### 2.1 Static Encoding Functions

#### 2.1.1 encode3StepBytes Golden Vectors
- Description: Validate exact 3-step encoded byte stream.
- Preconditions:
  - Golden source/destination vectors for boundary patterns.
- Operations:
  - Call `encode3StepBytes()`.
- Expected Results:
  - Encoded bytes match golden output exactly.

#### 2.1.2 encode4StepBytes Golden Vectors
- Description: Validate exact 4-step encoded byte stream.
- Preconditions:
  - Golden vectors including boundary-crossing bit patterns.
- Operations:
  - Call `encode4StepBytes()`.
- Expected Results:
  - Encoded bytes match golden output exactly.

#### 2.1.3 Generic encodeStepBytes Consistency
- Description: Verify generic encoder matches specialized wrappers.
- Preconditions:
  - Same source data for generic and specialized calls.
- Operations:
  - Compare outputs from `encodeStepBytes()` vs `encode3StepBytes()/encode4StepBytes()`.
- Expected Results:
  - Outputs are byte-identical for equivalent settings.

### 2.2 Runtime Wrapper Stream Emission

#### 2.2.1 Transmit Encoded Payload (3-Step)
- Description: Verify wrapper forwards exact 3-step encoded stream to underlying transport.
- Preconditions:
  - Wrapper configured for `ThreeStep`.
  - Underlying transport spy captures payload bytes.
- Operations:
  - Call `transmitBytes(source)`.
- Expected Results:
  - Underlying payload exactly equals expected 3-step encoded bytes.

#### 2.2.2 Transmit Encoded Payload (4-Step)
- Description: Verify wrapper forwards exact 4-step encoded stream.
- Preconditions:
  - Wrapper configured for `FourStep`.
- Operations:
  - Call `transmitBytes(source)`.
- Expected Results:
  - Underlying payload exactly equals expected 4-step encoded bytes.

#### 2.2.3 Encoded Length and Capacity Behavior
- Description: Verify encoded payload length and buffer resize behavior across varying source sizes.
- Preconditions:
  - Sequence of source frames with changing lengths.
- Operations:
  - Transmit multiple frames and record captured lengths.
- Expected Results:
  - Emitted lengths match expected encoding lengths each call.
  - No stale trailing bytes appear in captured stream.

#### 2.2.4 Transaction-Wrapped Stream Emission
- Description: Verify stream emission sequencing with transaction management.
- Preconditions:
  - Cases with `manageTransaction=true` and `false`.
- Operations:
  - Transmit one frame per case.
- Expected Results:
  - `true`: beginTransaction before transmit, endTransaction after.
  - `false`: transmit without transaction wrappers.

### 2.3 Wrapper + Protocol Integration Stream Checks

#### 2.3.1 Ws2812x Through Wrapper Byte-Length Consistency
- Description: Verify wrapper-transmitted encoded byte length matches protocol payload size and encoding ratio.
- Preconditions:
  - Ws2812x protocol using wrapper transport.
  - Channel order/count matrix.
- Operations:
  - Execute protocol update and capture wrapper output stream length.
- Expected Results:
  - Encoded length equals `protocolPayloadBytes * encodedBitsPerDataBit` (per current implementation contract).

#### 2.3.2 Zero-Length Source Frame Handling
- Description: Verify safe handling when protocol source frame is zero bytes.
- Preconditions:
  - `pixelCount == 0` protocol configuration.
- Operations:
  - Execute update and capture transport stream.
- Expected Results:
  - No invalid access or stale payload emission.
  - Stream behavior matches documented no-op/empty-frame contract.
