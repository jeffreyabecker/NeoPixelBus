# Post-Testing Example Implementation Plan

## Purpose

Define which examples to implement after native test coverage is in place for:

- busses
- shaders
- protocols

This plan assumes the testing milestones in `docs/testing-plan-native-unity-arduinofake.md` are complete and stable.

---

## Success Criteria Before Starting Examples

Examples work starts only after:

- Native test suite passes for `busses`, `shaders`, and `protocols`
- CI gate is enforcing native test success on PRs
- Core fake harness and smoke tests are no longer changing frequently

---

## Example Strategy

- Keep examples minimal and focused (one concept per example).
- Prefer demonstrable outputs over abstraction-heavy sample code.
- Start in `examples-virtual/` for new virtual architecture demonstrations.
- Add/port into `examples/` only where parity with classic user expectations is needed.

---

## Priority Order

## Phase A — Bus Examples (first)

Goal: show topology/composition APIs first because they are the primary consumer entry point.

1. `Bus_SegmentBasics`
   - Demonstrate splitting a parent strip into logical segments.
   - Show independent segment writes and one shared `show()` flow.

2. `Bus_ConcatTwoStrips`
   - Demonstrate one logical index space across two child buses.
   - Include uneven child lengths.

3. `Bus_Mosaic2x2Panels`
   - Demonstrate tiled coordinate mapping in a small panel layout.
   - Include one orientation remap example.

## Phase B — Shader Examples (second)

Goal: show deterministic transform chains independent of transport/protocol specifics.

1. `Shader_GammaSingle`
   - Apply gamma to a static test frame.
   - Compare pre/post values through serial output (or equivalent debug print).

2. `Shader_CurrentLimiter`
   - Apply current limiting to a high-brightness input frame.
   - Demonstrate preserved color intent at reduced power.

3. `Shader_ChainedPipeline`
   - Chain gamma + limiter in explicit order.
   - Show order-dependent behavior clearly.

## Phase C — Protocol Examples (third)

Goal: show protocol framing/channel-order behavior once bus/shader usage is already understood.

1. `Protocol_Ws2812xColorOrder`
   - Demonstrate channel order variants with identical logical colors.
   - Include side-by-side GRB vs RGB outcome note.

2. `Protocol_DotStarFrameBasics`
   - Demonstrate start/data/end framing behavior for DotStar-style output.
   - Show brightness/channel packing expectations.

3. `Protocol_Hd108HighDepth` (optional if maturity is sufficient)
   - Demonstrate high-depth color path behavior and expected constraints.

---

## Suggested Folder Targets

Primary (virtual architecture):

```text
examples-virtual/
  bus-segment-basics/
  bus-concat-two-strips/
  bus-mosaic-2x2-panels/
  shader-gamma-single/
  shader-current-limiter/
  shader-chained-pipeline/
  protocol-ws2812x-color-order/
  protocol-dotstar-frame-basics/
```

Optional parity mirrors (classic examples area):

```text
examples/
  topologies/
  gamma/
  DotStarTest/
```

---

## Per-Example Template

Each example should include:

- A short top-of-file "What this shows" section
- Minimal configuration constants (pixel count, pin/config, brightness)
- Deterministic render loop (avoid hidden randomness in baseline sample)
- Notes on expected visual output

Keep each sample standalone and avoid shared runtime dependencies between examples.

---

## Rollout Plan

1. Implement one example per category first (`Bus_SegmentBasics`, `Shader_GammaSingle`, `Protocol_Ws2812xColorOrder`).
2. Validate build with PlatformIO smoke environments.
3. Add remaining examples in phase order.
4. Update `ReadMe.md` with links grouped by category.

---

## Done Definition

- At least 3 baseline examples (one per category) are merged and buildable.
- Full phase list examples compile in their target environment.
- Documentation links from `ReadMe.md` are present and accurate.
- Each example clearly states expected output and the API seam it demonstrates.
