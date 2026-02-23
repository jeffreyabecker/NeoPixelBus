# Implementation Plan (Remaining Unimplemented Work)

This plan tracks only work that is still outstanding in the current repository.
Completed foundations (core virtual bus, core protocols, shader method set, examples) are intentionally omitted.

---

## 1) Goal

Close the gap between current code and intended end-state by implementing missing infrastructure, missing protocols/buses, and convenience APIs.

---

## 2) Current Delta Summary

Chip/protocol coverage deltas are tracked in `docs/chip-gap-analysis.md`.

### Missing convenience layer

1. `makeNeoPixelBus(...)` factory functions

### Behavioral gap to resolve
- No unresolved protocol behavior gaps tracked in this plan.

---

## 3) Remaining Phases

## Phase D — Convenience API + Migration Surface

### D.1 Factory functions
- Add `makeNeoPixelBus(...)` family for common chip/platform combinations
- Default selection should hide platform timing/inversion details where possible

Exit criteria:
- Public, ergonomic construction paths exist without manual emitter wiring for common use cases.

---

## 4) Validation Checklist

For each phase completion:

1. Build succeeds for current primary environment(s)
2. Relevant `examples-virtual/*` target compiles
3. Byte-stream output matches expected framing/ordering for known fixture pixels
4. Existing implemented emitters/shaders are not behavior-regressed

---

## 5) Non-Goals (for this remaining plan)

- Reworking already functional core classes solely to match old naming in prior drafts
- Broad architecture rewrites unrelated to missing deliverables
- Introducing new chip families not already listed above

---

## 6) Suggested Execution Order

1. Phase D (factory APIs)

This order focuses remaining work on ergonomic public construction APIs.
