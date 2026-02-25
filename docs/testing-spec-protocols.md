# Formal Test Specification: Protocols

Source:
- `docs/testing-spec-hierarchical.md` (migrated protocols section)

This document defines focused formal test cases for protocol byte-stream behavior and update contracts.
Each test includes: Description, Preconditions, Operations, and Expected Results.

## 1. Protocols

### 1.1 DotStarProtocol

#### 1.1.1 Construction and Begin
- Description: Verify initialization path.
- Preconditions: DotStar protocol with transport spy.
- Operations: Call `initialize()`.
- Expected Results: Transport `begin()` called once.

#### 1.1.2 End-Frame Extra Byte Calculation
- Description: Verify `ceil(pixelCount/16)` extra bytes.
- Preconditions: Pixel counts `[0,1,15,16,17,32]`.
- Operations: Construct protocol and inspect transmit framing counts via update.
- Expected Results: Extra byte count matches expected formula.

#### 1.1.3 FixedBrightness Serialization
- Description: Verify `0xFF` prefix and channel-order byte serialization.
- Preconditions: Known color vectors and order variants.
- Operations: Update in `FixedBrightness` mode and capture payload.
- Expected Results: Prefix and channel bytes match expected sequence.

#### 1.1.4 Luminance Serialization
- Description: Verify luminance prefix path.
- Preconditions: Mode = `Luminance`.
- Operations: Update and capture payload.
- Expected Results: Prefix bytes are `0xE0 | 31` and channel ordering remains correct.

#### 1.1.5 Framing and Transaction Sequence
- Description: Verify start frame, payload, and end frame ordering.
- Preconditions: Transport spy records transmit call granularity.
- Operations: Execute one update.
- Expected Results: Start frame (4x0x00), payload span, end frame (4x0x00 + extra), one begin/end transaction pair.

#### 1.1.6 P0: Oversized Color Span
- Description: Verify behavior when `colors.size() > pixelCount`.
- Preconditions: Oversized frame input.
- Operations: Call update under memory checks.
- Expected Results: No overflow; behavior conforms to documented guard policy.

#### 1.1.7 Channel-Order Contract Edge Cases
- Description: Validate short/null/invalid channel order behavior.
- Preconditions: Config variants for short strings, null, invalid chars.
- Operations: Update and inspect behavior/output.
- Expected Results: Behavior is deterministic and documented; no invalid memory access.

### 1.2 Hd108Protocol

#### 1.2.1 Construction/Size and Aliases
- Description: Verify frame sizing and alias behavior for RGB and RGBCW variants.
- Preconditions: `Hd108RgbProtocol` and `Hd108RgbcwProtocol` instances.
- Operations: Initialize and update with known vectors.
- Expected Results: Frame size and payload structure match formula.

#### 1.2.2 Prefix + Big-Endian Serialization
- Description: Verify per-pixel `0xFF,0xFF` prefix and big-endian channel bytes.
- Preconditions: Known 16-bit vectors and order strings.
- Operations: Update and capture payload.
- Expected Results: Prefix/channel bytes match exact expected sequence.

#### 1.2.3 Framing and Transaction Sequence
- Description: Verify start/end framing and single payload transmit.
- Preconditions: Transport spy.
- Operations: Update once.
- Expected Results: 16x `0x00` start bytes, payload, 4x `0xFF` end bytes, proper transaction pairing.

#### 1.2.4 P0: Oversized Span and Short Order
- Description: Validate overflow-prone conditions.
- Preconditions: `colors.size() > pixelCount`; short channelOrder.
- Operations: Update under memory checks.
- Expected Results: No overflow or invalid read; contract is enforced/documented.

#### 1.2.5 Null/Invalid Channel Order and Boundary Values
- Description: Validate null channelOrder safety and channel extrema serialization.
- Preconditions: Null/invalid order configs and extreme values.
- Operations: Update and inspect output.
- Expected Results: Safe behavior and exact big-endian encoding for boundaries.

### 1.3 Ws2801Protocol

#### 1.3.1 Serialization and Order Variants
- Description: Verify raw 3-byte payload order for RGB/GRB/BGR.
- Preconditions: Known vectors and order variants.
- Operations: Update and capture payload.
- Expected Results: Payload bytes match expected order and count.

#### 1.3.2 Transaction + Latch Timing
- Description: Verify transaction calls and latch timing behavior.
- Preconditions: Fake `micros()`/`delayMicroseconds()` and transport spy.
- Operations: Update then probe `isReadyToUpdate()` across latch window.
- Expected Results: Correct transaction calls; readiness false before and true after 500 µs.

#### 1.3.3 P0: Oversized Span and Unsafe Channel Order
- Description: Validate overflow/invalid-order risks.
- Preconditions: Oversized frame and short/null order configs.
- Operations: Update with memory guards.
- Expected Results: No overflow/invalid access; behavior documented.

### 1.4 PixieProtocol

#### 1.4.1 Serialization + Transaction Behavior
- Description: Verify payload format and transaction sequencing.
- Preconditions: Transport spy.
- Operations: Update with known vectors.
- Expected Results: One begin/transmit/end sequence and correct payload bytes.

#### 1.4.2 Readiness Gate and alwaysUpdate
- Description: Verify wait-loop gating and policy flags.
- Preconditions: Transport readiness toggles and fake time.
- Operations: Evaluate `isReadyToUpdate()` before/after latch and call update.
- Expected Results: Ready only after transport-ready + 1000 µs; `alwaysUpdate()==true`.

#### 1.4.3 P0: Oversized Span and Channel Order Safety
- Description: Validate overflow and order assumptions.
- Preconditions: Oversized and short/null order inputs.
- Operations: Update under memory checks.
- Expected Results: No overflow/invalid access; contract is explicit.

#### 1.4.4 Blocking Contract Case
- Description: Validate behavior when transport never becomes ready.
- Preconditions: Transport returns not-ready continuously.
- Operations: Call update with watchdog/timeout harness.
- Expected Results: Blocking behavior is detected and documented with mitigation.

### 1.5 Lpd6803Protocol

#### 1.5.1 Packed 5-5-5 Serialization
- Description: Verify bit15 and 5-bit channel packing in big-endian bytes.
- Preconditions: Table-driven vectors with order variants.
- Operations: Update and inspect payload words.
- Expected Results: Packed words match expected formulas exactly.

#### 1.5.2 Framing and End-Frame Size
- Description: Verify start and end frame counts.
- Preconditions: Pixel counts around 8-boundaries.
- Operations: Update and count frame transmit calls.
- Expected Results: 4 start bytes and `ceil(N/8)` end bytes.

#### 1.5.3 P0: Oversized Span and Channel Order Safety
- Description: Validate overflow and unsafe order cases.
- Preconditions: Oversized frame, short/null/invalid order configs.
- Operations: Update under memory checks.
- Expected Results: No overflow/invalid access.

### 1.6 Lpd8806Protocol

#### 1.6.1 7-bit+MSB Serialization
- Description: Verify `(value >> 1) | 0x80` conversion and order variants.
- Preconditions: Known vectors including `0x00` and `0xFF`.
- Operations: Update and inspect payload.
- Expected Results: Bytes match expected transformed values.

#### 1.6.2 Symmetric Start/End Framing
- Description: Verify both frame sides use `ceil(N/32)` length.
- Preconditions: Pixel counts around 32 boundaries.
- Operations: Update and count frame calls.
- Expected Results: Start uses zero bytes and end uses `0xFF` bytes of equal count.

#### 1.6.3 P0: Oversized Span and Channel Order Safety
- Description: Validate overflow and invalid order conditions.
- Preconditions: Oversized/short/null order inputs.
- Operations: Update under memory checks.
- Expected Results: No overflow/invalid access.

### 1.7 P9813Protocol

#### 1.7.1 Header Checksum Encoding
- Description: Verify checksum header from inverted top-2 bits of B/G/R.
- Preconditions: Table vectors across channel boundaries.
- Operations: Update and inspect header bytes.
- Expected Results: Header bytes match exact bit formula.

#### 1.7.2 Framing + BGR Data Order
- Description: Verify fixed start/end frames and BGR data byte order.
- Preconditions: Transport spy and known vectors.
- Operations: Update and inspect call sequence.
- Expected Results: 4 start zeros, payload, 4 end zeros; data bytes in BGR.

#### 1.7.3 P0: Oversized Span
- Description: Validate oversized color span safety.
- Preconditions: `colors.size() > pixelCount`.
- Operations: Update under memory checks.
- Expected Results: No overflow/crash.

### 1.8 Sm168xProtocol

#### 1.8.1 Variant Resolution and Frame Sizing
- Description: Verify channel/settings sizes for 3/4/5 variants.
- Preconditions: Protocol instances for each variant.
- Operations: Initialize/update and inspect frame lengths.
- Expected Results: Sizes match variant formulas.

#### 1.8.2 Payload Serialization by Variant
- Description: Verify channel-order payload serialization for each variant.
- Preconditions: Known vectors and order strings.
- Operations: Update and inspect payload segment.
- Expected Results: Payload bytes match expected order and length.

#### 1.8.3 Settings Trailer Encoding
- Description: Verify gain packing and masks (`0x0F` / `0x1F`) by variant.
- Preconditions: Gain vectors including out-of-range values.
- Operations: Update and inspect trailer bytes.
- Expected Results: Encoded trailer matches variant bit layout and masking rules.

#### 1.8.4 P0: Oversized Span and Unsafe Order
- Description: Validate overflow and short/null order behavior.
- Preconditions: Oversized and malformed configs.
- Operations: Update under memory checks.
- Expected Results: No overflow/invalid access.

#### 1.8.5 Compile-Time Contract Cases
- Description: Verify static constraints for component type and channel range.
- Preconditions: Negative compile tests.
- Operations: Attempt invalid template instantiations.
- Expected Results: Compilation fails with expected static-assert diagnostics.

### 1.9 Sm16716Protocol

#### 1.9.1 Buffer Size and Start-Bit Prefix
- Description: Verify buffer size formula and 50-bit zero prefix.
- Preconditions: Pixel-count matrix.
- Operations: Update and inspect serialized stream.
- Expected Results: Buffer size and first 50 bits are correct.

#### 1.9.2 Separator Bit + MSB-First Packing
- Description: Verify per-pixel separator and channel packing across byte boundaries.
- Preconditions: Boundary-focused vector set.
- Operations: Update and compare against golden bitstream.
- Expected Results: Exact bit-level match across boundary positions.

#### 1.9.3 P0: Oversized Span and Unsafe Order
- Description: Validate overflow and order assumptions.
- Preconditions: Oversized frame and short/null order configs.
- Operations: Update under memory checks.
- Expected Results: No out-of-bounds writes/reads.

### 1.10 Tlc5947Protocol

#### 1.10.1 Strategy Resolution and Module Sizing
- Description: Verify active channel count, pixels-per-module, module count, and payload size.
- Preconditions: Strategy matrix and pixel-count matrix.
- Operations: Construct/update with each strategy.
- Expected Results: All derived sizing values match formulas.

#### 1.10.2 12-bit Narrowing and Reverse Packing
- Description: Verify `value >> 4` narrowing and 2x12-bit to 3-byte reverse-order packing.
- Preconditions: Table-driven 16-bit vectors.
- Operations: Update and inspect module payload bytes.
- Expected Results: Packed bytes match expected reverse ordering.

#### 1.10.3 Tail Fill Strategy Matrix
- Description: Verify `Zero`, `RepeatFirstPixel`, and `RepeatLastPixel` behavior for partial modules.
- Preconditions: Partial final module scenarios.
- Operations: Update per strategy and inspect tail channels.
- Expected Results: Tail bytes match configured strategy.

#### 1.10.4 P0: isReadyToUpdate Contract
- Description: Regression-guard current behavior where readiness returns true independent of transport.
- Preconditions: Transport spy with controllable ready state.
- Operations: Query protocol readiness.
- Expected Results: Behavior matches documented current contract (and flags future drift).

### 1.11 Tlc59711Protocol

#### 1.11.1 Header Bitfield Encoding
- Description: Verify command/control/brightness bit packing and masking.
- Preconditions: Config matrix across control flags and brightness boundaries.
- Operations: Construct/update config and inspect header bytes.
- Expected Results: Header bytes match expected bitfield maps.

#### 1.11.2 Reverse Chip/Pixel Serialization
- Description: Verify reversed chip order, reversed pixel order in chip, and BGR16 byte replication.
- Preconditions: Multi-chip pixel vectors.
- Operations: Update and inspect full payload.
- Expected Results: Ordering and replicated 16-bit channel bytes match expected wire format.

#### 1.11.3 Latch Guard Invocation
- Description: Verify post-transmit latch delay call.
- Preconditions: Fake `delayMicroseconds` hook.
- Operations: Update once.
- Expected Results: `delayMicroseconds(20)` invoked once per update.

#### 1.11.4 P0: isReadyToUpdate Contract
- Description: Regression-guard current always-ready behavior.
- Preconditions: Transport reports not-ready.
- Operations: Query protocol readiness.
- Expected Results: Behavior matches current documented contract.

### 1.12 Tm1814Protocol

#### 1.12.1 Current Clamp and Encode
- Description: Verify current clamping `[65,380]` and encoding `(limited-65)/5`.
- Preconditions: Current test vectors below, within, and above limits.
- Operations: Update and inspect first 4 settings bytes.
- Expected Results: Encoded current bytes match clamped formula.

#### 1.12.2 Settings Inversion Bytes
- Description: Verify second settings half is bitwise inverse of first.
- Preconditions: Protocol with known settings.
- Operations: Update and inspect settings bytes 0..7.
- Expected Results: Bytes 4..7 equal `~bytes[0..3]` respectively.

#### 1.12.3 Payload Serialization and Order
- Description: Verify WRGB (or configured order) payload serialization.
- Preconditions: Known frame vectors and order variants.
- Operations: Update and inspect payload section.
- Expected Results: Payload bytes match expected order and count.

#### 1.12.4 P0: Oversized Span and Channel Order Safety
- Description: Validate overflow and short/null order risks.
- Preconditions: Oversized frame + malformed order configs.
- Operations: Update under memory checks.
- Expected Results: No overflow/invalid access.

#### 1.12.5 Blocking Readiness Contract
- Description: Verify behavior when transport remains not-ready.
- Preconditions: Transport forced not-ready.
- Operations: Call update under watchdog harness.
- Expected Results: Blocking behavior detected and documented with mitigation.

### 1.13 Tm1914Protocol

#### 1.13.1 Mode Encoding Matrix
- Description: Verify mode byte values for all `Tm1914Mode` enums.
- Preconditions: One case per mode.
- Operations: Update and inspect settings byte 2.
- Expected Results: Mode byte matches enum mapping (`FF`, `F5`, `FA`).

#### 1.13.2 Settings Inversion and Payload Order
- Description: Verify inverse settings bytes and payload serialization by order.
- Preconditions: Known frame vectors and order variants.
- Operations: Update and inspect settings + payload.
- Expected Results: Inversion bytes correct; payload bytes ordered as configured.

#### 1.13.3 P0: Oversized Span and Channel Order Safety
- Description: Validate overflow and short/null order behavior.
- Preconditions: Oversized and malformed order scenarios.
- Operations: Update under memory checks.
- Expected Results: No overflow/invalid access.

#### 1.13.4 Blocking Readiness Contract
- Description: Verify wait-loop behavior under permanently not-ready transport.
- Preconditions: Transport forced not-ready.
- Operations: Execute update with watchdog harness.
- Expected Results: Blocking behavior documented and mitigated by caller policy.

### 1.14 Ws2812xProtocol

#### 1.14.1 Constructor Equivalence and Frame Sizing
- Description: Verify settings-based and direct constructors produce same sizing/behavior.
- Preconditions: Equivalent configs and pixel counts.
- Operations: Construct both forms and compare frame behavior.
- Expected Results: Equivalent wire output and frame sizing.

#### 1.14.2 Channel Order/Count Resolution
- Description: Verify null/empty/overlong channel-order handling.
- Preconditions: Config matrix including null, empty, and long strings.
- Operations: Construct and update with each variant.
- Expected Results: Default GRB fallback and clamped channel count behave as expected.

#### 1.14.3 8-bit and 16-bit Serialization
- Description: Verify 8-bit direct and 16-bit high-byte serialization across 3/4/5 channel colors.
- Preconditions: Type matrix and table vectors.
- Operations: Update and inspect payload bytes.
- Expected Results: Byte stream matches expected per-type conversion and order.

#### 1.14.4 Readiness Wait-Loop Contract
- Description: Verify update waits until transport reports ready.
- Preconditions: Transport readiness controllable.
- Operations: Call update while toggling readiness.
- Expected Results: Transmission occurs only after ready state is reached.

#### 1.14.5 P0: Oversized Span and Allocation Failure Path
- Description: Validate overflow and null-buffer risk cases.
- Preconditions:
  - `colors.size() > pixelCount` case.
  - Allocation-failure injection for `_data`.
- Operations:
  - Update under memory checks and failure injection.
- Expected Results:
  - No invalid writes/reads or crash.
  - Behavior follows explicit failure-handling contract.
