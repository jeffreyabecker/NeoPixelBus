# Formal Test Specification: Color, ColorIterator, CurrentLimiterShader, AggregateShader

Source inputs:
- `docs/testing-plan-native-unity-arduinofake.md`
- `docs/testing-spec-hierarchical.md` (migrated color + ColorIterator sections)
- `src/virtual/colors/Color.h`
- `src/virtual/colors/ColorIterator.h`
- `src/virtual/colors/CurrentLimiterShader.h`
- `src/virtual/colors/AggregateShader.h`

Notes:
- This specification uses canonical class names from source: `CurrentLimiterShader` and `AggregateShader`.
- Scope aligns with plan policy: strict automated coverage for `CurrentLimiterShader` and `AggregateShader`; `GammaShader` and `WhiteBalanceShader` remain out-of-scope here.

## 1. Color Domain

### 1.1 BasicColor Construction and Representation

#### 1.1.1 Default Construction Zero Initialization
- Description: Verify default-constructed color objects initialize all channels to zero.
- Preconditions:
  - Instances created for `Rgb8Color`, `Rgbw8Color`, `Rgbcw8Color`, `Rgb16Color`, `Rgbw16Color`, and `Rgbcw16Color`.
- Operations:
  - Read all channels by integral index for each instance.
- Expected Results:
  - Every channel value is zero.

#### 1.1.2 Variadic Constructor Prefix Assignment
- Description: Verify variadic constructor writes provided arguments in order and leaves remaining channels zero.
- Preconditions:
  - Construct each alias with 1..N arguments (where N <= channel count).
- Operations:
  - Read all channels and compare to expected prefix + zero tail.
- Expected Results:
  - Channel values match expected prefix assignment and zero-fill behavior.

#### 1.1.3 Component Type and Channel Metadata
- Description: Verify static metadata (`ChannelCount`, `MaxComponent`, `ComponentType`) for all aliases.
- Preconditions:
  - Alias type set available.
- Operations:
  - Evaluate compile-time/static values and runtime equivalents used in tests.
- Expected Results:
  - 8-bit aliases report max 255; 16-bit aliases report max 65535.
  - Channel counts match alias names (3, 4, 5).

### 1.2 Channel Access and Mapping Semantics

#### 1.2.1 Integral Index Read/Write Round-Trip
- Description: Verify mutable and const integral indexing behavior.
- Preconditions:
  - Colors initialized with distinct channel values.
- Operations:
  - Write channels using mutable `operator[](index)`.
  - Read back via const and mutable access paths.
- Expected Results:
  - All writes/readbacks match exactly.

#### 1.2.2 Character Index Mapping (Upper/Lower Case)
- Description: Verify `'R'/'G'/'B'/'W'/'C'` and lowercase equivalents map correctly.
- Preconditions:
  - Distinct values assigned per channel where available.
- Operations:
  - Access channels using character index for uppercase and lowercase variants.
- Expected Results:
  - Returned values correspond to expected mapped channels.

#### 1.2.3 Unknown Channel Fallback Behavior
- Description: Verify unknown channel characters map to channel index 0 by design.
- Preconditions:
  - Color with distinct channel 0 value.
- Operations:
  - Access using invalid channel characters (e.g., `'X'`, `'?'`).
- Expected Results:
  - Access resolves to channel 0 consistently.

#### 1.2.4 W/C Fallback on Lower-Channel Colors
- Description: Verify `'W'` and `'C'` fallback semantics for insufficient channel counts.
- Preconditions:
  - 3-channel and 4-channel color instances with distinct values.
- Operations:
  - Access `'W'` and `'C'` on lower-channel aliases.
- Expected Results:
  - `'W'` maps to index 0 when N<=3.
  - `'C'` maps to index 0 when N<=4.

### 1.3 Equality and ChannelOrder Constants

#### 1.3.1 Equality Operator Correctness
- Description: Verify `operator==` behavior for equal and unequal channel arrays.
- Preconditions:
  - Pair of equal colors and pair with one differing channel.
- Operations:
  - Compare equality for both pairs.
- Expected Results:
  - Equal pair returns true; differing pair returns false.

#### 1.3.2 ChannelOrder String/Length Consistency
- Description: Verify `ChannelOrder::*` string constants and `Length*` constants are consistent.
- Preconditions:
  - Access to all ChannelOrder constants.
- Operations:
  - Compare `Length*` to computed string lengths.
- Expected Results:
  - All `Length*` values match corresponding string lengths.

### 1.4 Conversion Helper Functions

#### 1.4.1 widen Conversion Formula
- Description: Verify `widen` applies `(v << 8) | v` per channel.
- Preconditions:
  - 8-bit input vectors including edge and mixed values.
- Operations:
  - Apply `widen` and inspect 16-bit results.
- Expected Results:
  - Each output channel matches formula exactly.

#### 1.4.2 narrow Conversion Formula
- Description: Verify `narrow` applies `v >> 8` per channel.
- Preconditions:
  - 16-bit input vectors including edge and mixed values.
- Operations:
  - Apply `narrow` and inspect 8-bit results.
- Expected Results:
  - Each output channel equals source high byte.

#### 1.4.3 expand Ordering and Zero-Fill
- Description: Verify `expand<N,M>` preserves order and zero-fills added channels.
- Preconditions:
  - Source colors where `N > M`.
- Operations:
  - Apply `expand` across representative channel-size transitions.
- Expected Results:
  - First M channels copied exactly; remaining channels are zero.

#### 1.4.4 compress Ordering
- Description: Verify `compress<N,M>` preserves leading channel order.
- Preconditions:
  - Source colors where `N < M`.
- Operations:
  - Apply `compress` and inspect output.
- Expected Results:
  - Output contains first N source channels in original order.

### 1.5 Safety/Boundary Contracts

#### 1.5.1 P0 Contract: Out-of-Range Integral Index Use
- Description: Ensure tests codify that direct out-of-range integral indexing is undefined and must be guarded at call sites.
- Preconditions:
  - Guarded wrapper/helpers available in tests.
- Operations:
  - Attempt boundary-crossing scenarios only through guard helpers.
- Expected Results:
  - Guard policy prevents undefined direct out-of-range indexing from being treated as valid behavior.

#### 1.5.2 Boundary Stress for Conversion Helpers
- Description: Verify conversion helper behavior at numeric extrema.
- Preconditions:
  - Inputs including `0x00`, `0xFF`, `0x0000`, `0xFFFF`.
- Operations:
  - Execute `widen`, `narrow`, `expand`, and `compress` boundary vectors.
- Expected Results:
  - Outputs match expected formulas and contain no stray values.

## 2. ColorIterator Domain

### 2.1 Random-Access Iterator Contract

#### 2.1.1 Increment/Decrement Semantics
- Description: Verify pre/post increment and decrement semantics on `ColorIteratorT`.
- Preconditions:
  - Iterator over mutable span-backed storage.
- Operations:
  - Apply `++it`, `it++`, `--it`, `it--` with known baseline position.
- Expected Results:
  - Position and dereference behavior match random-access iterator expectations.

#### 2.1.2 Arithmetic and Distance Semantics
- Description: Verify `+`, `-`, `+=`, `-=` and iterator difference operator.
- Preconditions:
  - Begin/end iterators over known-size source.
- Operations:
  - Perform arithmetic shifts and compute differences.
- Expected Results:
  - Position and distance values match expected index math.

#### 2.1.3 Dereference and Subscript Reference Semantics
- Description: Verify dereference and subscript return mutable references to underlying storage.
- Preconditions:
  - Source storage initialized with sentinel values.
- Operations:
  - Mutate through `*it` and `it[n]`.
- Expected Results:
  - Underlying storage is mutated at corresponding positions.

#### 2.1.4 Comparison Semantics
- Description: Verify equality and ordering operators are position-consistent for same-range iterators.
- Preconditions:
  - Multiple iterators over same source.
- Operations:
  - Compare equal and non-equal positions.
- Expected Results:
  - Comparisons reflect position ordering correctly.

### 2.2 Source Adapter Semantics

#### 2.2.1 SolidColorSource Range Length
- Description: Verify `SolidColorSourceT` begin/end distance equals `pixelCount`.
- Preconditions:
  - Solid source configured with known pixel count.
- Operations:
  - Compute iterator distance.
- Expected Results:
  - Distance equals configured `pixelCount`.

#### 2.2.2 SolidColorSource Mutability Contract
- Description: Verify mutations through source iterator affect backing color instance.
- Preconditions:
  - Solid source with mutable color object.
- Operations:
  - Write through iterator dereference.
- Expected Results:
  - Backing color reflects mutation.

#### 2.2.3 SpanColorSource Constructor Equivalence
- Description: Verify span constructor and pointer+size constructor produce equivalent iteration behavior.
- Preconditions:
  - Same underlying buffer exposed through both constructors.
- Operations:
  - Iterate and compare values/mutations across both source variants.
- Expected Results:
  - Behavior is equivalent in order, values, and mutability.

#### 2.2.4 STL Interop with std::copy
- Description: Verify source adapters interoperate with standard algorithms.
- Preconditions:
  - Destination buffers allocated and initialized.
- Operations:
  - Use `std::copy` from `SolidColorSourceT` and `SpanColorSourceT`.
- Expected Results:
  - Destination buffers match expected copied values.

### 2.3 Safety/Boundary Contracts

#### 2.3.1 P0 Contract: Span Size Truncation (>65535)
- Description: Verify behavior when `SpanColorSourceT::end()` position truncates to `uint16_t`.
- Preconditions:
  - Source span larger than 65535 elements.
- Operations:
  - Build iterators and evaluate effective distance behavior under test harness policy.
- Expected Results:
  - Truncation behavior is explicitly documented and regression guarded.

#### 2.3.2 P0 Contract: Iterator Arithmetic Overflow/Underflow
- Description: Verify callers clamp iterator arithmetic near `uint16_t` boundaries.
- Preconditions:
  - Boundary-position iterators and guard helpers.
- Operations:
  - Attempt boundary-crossing arithmetic through guarded paths.
- Expected Results:
  - Guard policy prevents unsafe arithmetic from being treated as valid behavior.

#### 2.3.3 Position-Only Equality Caveat
- Description: Verify equality is position-based and does not validate accessor identity.
- Preconditions:
  - Two iterators with same position but different accessors.
- Operations:
  - Compare iterators for equality.
- Expected Results:
  - Equality outcome follows implementation contract (position-only), and tests document intended same-range usage.

#### 2.3.4 Default-Constructed Iterator Contract
- Description: Verify safe behavior contract for default iterators.
- Preconditions:
  - Default-constructed iterators.
- Operations:
  - Compare for equality; do not dereference.
- Expected Results:
  - Default iterators compare equal; dereference is not used in valid tests.

## 3. CurrentLimiterShader Domain

### 3.1 Deterministic Limiting and Estimation

#### 3.1.1 No-Limit Path (`maxMilliamps == 0`)
- Description: Verify shader bypass path when max budget is disabled.
- Preconditions:
  - Settings with `maxMilliamps = 0`.
  - Non-empty color frame with non-zero values.
- Operations:
  - Apply shader to frame.
  - Read `lastEstimatedMilliamps()`.
- Expected Results:
  - Frame remains unchanged.
  - `lastEstimatedMilliamps() == 0`.

#### 3.1.2 Under-Budget Pass-Through
- Description: Verify no scaling when estimated pixel current is within budget.
- Preconditions:
  - Settings with realistic per-channel current values and sufficient budget.
- Operations:
  - Apply shader to representative frame.
- Expected Results:
  - Output frame equals input frame.
  - Estimated milliamps reflect weighted draw + controller + standby components.

#### 3.1.3 Over-Budget Scaling
- Description: Verify deterministic scaling when estimated draw exceeds budget.
- Preconditions:
  - Frame and settings configured to exceed budget.
- Operations:
  - Apply shader and inspect output channels.
- Expected Results:
  - All channels are uniformly scaled using integer rounding formula `(value*scale+127)/255`.
  - Scaled estimate does not exceed effective budget.

#### 3.1.4 Controller-Dominant Cutoff (`max <= controller`)
- Description: Verify full pixel cutoff path when controller draw consumes budget.
- Preconditions:
  - `maxMilliamps <= controllerMilliamps`.
  - Non-empty frame.
- Operations:
  - Apply shader and inspect output and estimate.
- Expected Results:
  - Pixel channels are scaled to zero.
  - Estimated value equals controller + standby draw term.

### 3.2 Current Draw Math and Options

#### 3.2.1 Weighted Draw Estimation by Channel
- Description: Verify weighted draw uses configured `milliampsPerChannel` coefficients.
- Preconditions:
  - Distinct per-channel coefficients and color vectors.
- Operations:
  - Apply shader to known vectors and compare estimate against computed reference.
- Expected Results:
  - Estimated draw follows weighted-sum math exactly.

#### 3.2.2 RGBW Derating Enabled
- Description: Verify RGBW derating path applies 3/4 factor for channel count >= 4.
- Preconditions:
  - 4-channel or 5-channel color type.
  - `rgbwDerating = true`.
- Operations:
  - Apply shader and compare estimate to non-derated baseline.
- Expected Results:
  - Pixel weighted draw is reduced by integer `(draw * 3) / 4` factor.

#### 3.2.3 RGBW Derating Disabled
- Description: Verify no derating when `rgbwDerating = false`.
- Preconditions:
  - Same frame/settings as derating-enabled test except flag false.
- Operations:
  - Apply shader and compare estimates.
- Expected Results:
  - Estimate reflects non-derated weighted draw.

#### 3.2.4 Standby Current Budget Interaction
- Description: Verify standby current is deducted from budget and included in estimate.
- Preconditions:
  - Non-zero `standbyMilliampsPerPixel` and non-empty frame.
- Operations:
  - Apply shader under near-budget conditions.
- Expected Results:
  - Effective pixel budget is reduced by standby draw.
  - Estimated value includes standby contribution.

### 3.3 Boundary and Safety Cases

#### 3.3.1 Empty Frame Behavior
- Description: Verify deterministic behavior on empty color spans.
- Preconditions:
  - Empty frame; non-zero budget settings.
- Operations:
  - Apply shader.
- Expected Results:
  - No mutation errors.
  - Estimate reflects fixed terms only, per implementation path.

#### 3.3.2 Extreme Component Values
- Description: Verify stable output for min/max component values.
- Preconditions:
  - Frames containing all-zero and all-max components.
- Operations:
  - Apply shader under constrained and unconstrained budgets.
- Expected Results:
  - No overflow artifacts.
  - Scaling remains bounded and deterministic.

#### 3.3.3 Scale Clamp and Rounding Stability
- Description: Verify scale factor clamping and rounding behavior at boundaries.
- Preconditions:
  - Cases yielding computed scale near `0`, `1`, `254`, `255`, and above 255 before clamp.
- Operations:
  - Apply shader and inspect output.
- Expected Results:
  - Scale is clamped to `[0,255]`.
  - Channel rounding matches formula exactly.

## 4. AggregateShader Domain

### 4.1 Composition Order and Pass-Through

#### 4.1.1 Ordered Shader Application
- Description: Verify shaders are applied in stored order.
- Preconditions:
  - Two or more deterministic shader stubs that produce order-sensitive output.
- Operations:
  - Build `AggregateShader` with known order and apply to frame.
- Expected Results:
  - Output matches sequential application in insertion order.

#### 4.1.2 Null Shader Handle Skip
- Description: Verify null shader entries are skipped safely.
- Preconditions:
  - Aggregate settings containing null and non-null shader handles.
- Operations:
  - Apply aggregate shader.
- Expected Results:
  - Non-null shaders apply as expected.
  - Null entries do not throw/crash and do not alter output directly.

#### 4.1.3 Empty Shader List No-Op
- Description: Verify aggregate with no shaders is a no-op.
- Preconditions:
  - Aggregate settings with empty shader list.
- Operations:
  - Apply to non-empty frame.
- Expected Results:
  - Frame remains unchanged.

### 4.2 Ownership and Wrapper Behavior

#### 4.2.1 OwningAggregateShaderT Equivalence
- Description: Verify `OwningAggregateShaderT` behavior matches equivalent `AggregateShader` composition.
- Preconditions:
  - Same shader sequence instantiated both ways.
- Operations:
  - Apply both aggregates to identical input frames.
- Expected Results:
  - Outputs are identical.

#### 4.2.2 Frame Mutation Consistency Across Repeated Calls
- Description: Verify repeated applications produce deterministic composed output.
- Preconditions:
  - Deterministic shader chain and fixed input.
- Operations:
  - Apply aggregate multiple times under controlled reset conditions.
- Expected Results:
  - Outputs remain consistent with chain semantics each run.

### 4.3 Boundary and Contract Cases

#### 4.3.1 Mixed Null/Valid Chain Stability
- Description: Verify chain stability when null entries exist between valid shaders.
- Preconditions:
  - Shader list pattern like `[validA, null, validB, null]`.
- Operations:
  - Apply aggregate and inspect output.
- Expected Results:
  - Behavior equals `validA` then `validB` composition; null entries ignored.

#### 4.3.2 Large Chain Performance-Safety Sanity
- Description: Verify no functional degradation in long shader chains.
- Preconditions:
  - Aggregate with many deterministic lightweight shaders.
- Operations:
  - Apply aggregate and validate output against reference composition.
- Expected Results:
  - Output matches expected sequential composition.
  - No crashes or order drift.
