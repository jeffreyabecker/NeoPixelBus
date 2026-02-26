# Formal Test Specification: Transports

Source:
- `docs/testing-spec-hierarchical.md` (migrated transport section)

This document defines focused formal test cases for transport-layer behavior.
Each test includes: Description, Preconditions, Operations, and Expected Results.

## 1. Transports

### 1.1 OneWireWrapper

#### 1.1.1 Construction and Begin Initialization
- Description: Verify wrapper construction and begin-time state initialization.
- Preconditions:
  - Wrapped transport spy and wrapper config prepared.
- Operations:
  - Construct wrapper and call `begin()`.
- Expected Results:
  - Underlying transport `begin()` called.
  - Internal frame timing baseline is initialized.

#### 1.1.2 3-Step/4-Step Encode Length
- Description: Verify encoded output size for representative input lengths.
- Preconditions:
  - Input vectors of size 0, 1, and N bytes.
- Operations:
  - Invoke `encode3StepBytes` and `encode4StepBytes`.
- Expected Results:
  - Encoded byte counts match expected formulas.

#### 1.1.3 Encode Golden Patterns
- Description: Verify bit pattern correctness for canonical source bytes.
- Preconditions:
  - Golden vectors for `0x00`, `0xFF`, `0x80`, `0x01`.
- Operations:
  - Encode with 3-step and 4-step modes and compare outputs.
- Expected Results:
  - Encoded output matches golden vectors exactly.

#### 1.1.4 Transaction Management On/Off
- Description: Verify transaction orchestration with `manageTransaction` enabled/disabled.
- Preconditions:
  - Wrapped transport records call sequence.
- Operations:
  - Transmit with `manageTransaction=true`, then false.
- Expected Results:
  - Enabled: `beginTransaction -> transmit -> endTransaction`.
  - Disabled: only transmit is called.

#### 1.1.5 Timing and Readiness Gate
- Description: Verify readiness requires transport readiness and reset window completion.
- Preconditions:
  - Deterministic fake `micros()` and transport readiness control.
- Operations:
  - Transmit frame; probe readiness before and after threshold.
- Expected Results:
  - Ready only when both conditions are satisfied.

#### 1.1.6 Bitrate-Dependent Frame Duration
- Description: Verify duration selection for zero and non-zero bitrates.
- Preconditions:
  - Two configs: `clockRateHz=0` and non-zero.
- Operations:
  - Transmit frames and inspect computed readiness timing.
- Expected Results:
  - Zero bitrate uses `timing.resetUs`.
  - Non-zero bitrate uses `max(encodedUs, timing.resetUs)`.

#### 1.1.7 Protocol Integration Length Consistency (Ws2812x)
- Description: Verify encoded transmit lengths for integrated `Ws2812xProtocol` cases.
- Preconditions:
  - Wrapper + Ws2812x pipeline test harness.
- Operations:
  - Run channelOrder length 3/4/5 and null/empty cases.
- Expected Results:
  - Encoded length matches `pixelCount * resolvedChannelCount * encodedBitsPerDataBit`.

#### 1.1.8 P0: Byte-Boundary Carry Integrity
- Description: Verify no carry-bit loss at encoded byte boundaries.
- Preconditions:
  - Golden vectors targeting boundary-crossing bit sequences.
- Operations:
  - Encode and compare outputs.
- Expected Results:
  - No dropped/corrupted boundary bits.

#### 1.1.9 P0: Large Payload Resizing Stability
- Description: Verify large payloads do not trigger arithmetic or resize errors.
- Preconditions:
  - Large input datasets.
- Operations:
  - Repeatedly transmit varying large lengths.
- Expected Results:
  - Capacity and transmitted lengths stay consistent and valid.

#### 1.1.10 Edge Contract Cases
- Description: Validate remaining wrapper edge contracts.
- Preconditions:
  - Configurable enum and ready-state controls.
- Operations:
  - Use non-standard bitPattern values.
  - Use very low bitrates.
  - Test empty payload transmit.
  - Simulate micros rollover.
- Expected Results:
  - Behavior remains safe and bounded.
  - No stale encoded data leak between calls.
