# Method Reorganization and Template-vs-DI Impact Analysis

## Context

This library is optimized for LED output hot paths and currently relies heavily on templates to:

- eliminate virtual dispatch in per-pixel and per-byte operations,
- specialize color feature packing at compile time,
- map method implementations to platform peripherals (PIO, RMT, I2S, DMA, UART, bitbang).

Current layout mixes generic and platform-specific method files under `src/internal/methods/`, with RP2040 partially isolated under `src/internal/methods/Rp2040/`.

## Goal

1. Reorganize methods so platform-specific methods remain in platform folders.
2. Analyze impact of moving from template matrix sprawl to compile-time injection and limited runtime abstraction boundaries.
3. Include impact of changing pixel write APIs from scalar (`applyPixelColor(...)`) to batched/range APIs such as:

```cpp
void applyPixelColor(std::span<uint8_t> pixels, std::ranges::range<Color> colors)
```

4. Cover code impact, runtime performance, and memory usage.

Primary constraint for this analysis: all projected targets are `<4 MB` flash devices, so code-size control is a first-order requirement.

---

## 1) Proposed Method Folder Reorganization

### Recommended structure

```text
src/internal/methods/
  common/
    NeoBits.h
    PixieStreamMethod.h
    DotStarGenericMethod.h
    Lpd8806GenericMethod.h
    Lpd6803GenericMethod.h
    Ws2801GenericMethod.h
    P9813GenericMethod.h
    Tlc5947GenericMethod.h
    Tlc59711GenericMethod.h
    Sm16716GenericMethod.h
    Mbi6033GenericMethod.h
    Hd108GenericMethod.h

  platform/
    avr/
      NeoAvrMethod.h
      NeoPixelAvr.c
    arm/
      NeoArmMethod.h
    esp8266/
      NeoEsp8266DmaMethod.h
      NeoEsp8266I2sDmx512Method.h
      NeoEsp8266I2sMethodCore.h
      NeoEsp8266I2sMethodCore.cpp
      NeoEsp8266UartMethod.h
      NeoEsp8266UartMethod.cpp
      NeoEspBitBangMethod.h
      NeoEspBitBangMethod.cpp
    esp32/
      NeoEsp32I2sMethod.h
      NeoEsp32I2sXMethod.h
      NeoEsp32LcdXMethod.h
      NeoEsp32RmtMethod.h
      NeoEsp32RmtMethod.cpp
      DotStarEsp32DmaSpiMethod.h
      Esp32_i2s.h
      Esp32_i2s.c
      FractionClk.h
      FractionClk.c
    nrf52/
      NeoNrf52xMethod.h
    rp2040/
      NeoRp2040x4Method.h
      NeoRp2040DmaState.h
      NeoRp2040PioInstance.h
      NeoRp2040PioMonoProgram.h
      NeoRp2040PioMonoProgram.cpp
      NeoRp2040PioSpeed.h
```

### Header aggregation strategy

- Keep `src/internal/NeoMethods.h` and `src/internal/XMethods.h` as stable entry points.
- Convert them into thin include routers to the new folder layout.
- Keep all existing public type aliases initially to avoid source breakage.

### Why this helps

- Platform boundaries become explicit and easier to maintain.
- Build-system targeting (future per-platform excludes) becomes simpler.
- Reduces accidental cross-platform include coupling.

---

## 2) Replacing Templates with Interfaces + DI

Assumption for this analysis: dependency injection means **compile-time injection** (policies/concepts/function objects), not a runtime DI container.

## 2.1 Baseline architecture behavior (today)

Template specialization provides:

- compile-time binding of feature + method,
- maximal inlining in pixel write and encode loops,
- no virtual tables, no per-call dispatch indirection,
- higher code size due to many specializations.

This is especially aligned with LED send pipelines where most methods are DMA/PIO setup plus strict byte packing.

## 2.2 Injection/abstraction alternatives

### A) Dynamic polymorphism (`virtual` interface)

Example shape:

- `IPixelPacker::pack(...)`
- `ITransportMethod::show(...)`

**Pros**

- lower template complexity,
- easier test seams/mocking,
- simpler compile dependencies.

**Cons**

- indirect calls inhibit inlining in hot paths,
- vtable/object overhead,
- branch/indirect-call penalty on MCUs,
- potential regression when used per pixel.

### B) Static DI (policy objects / concepts / function object injection)

- Retains compile-time dispatch,
- keeps zero-overhead call sites,
- still allows composition and cleaner module boundaries.
- avoids DI container/runtime service locator complexity.

**This is the preferred compromise for hot loops.**

### C) Hybrid

- Runtime polymorphism only at configuration boundaries,
- hot loops remain templated or function-pointer-specialized after setup.
- no container-based runtime graph construction.

**This is usually the best migration target for this library.**

---

## 3) Runtime Performance Impact

For this project, runtime performance must be balanced against a hard flash ceiling; the best solution is not maximum specialization, but the best performance that fits the flash budget.

## 3.1 Dispatch model impact

### If virtual dispatch is inside per-pixel loop

Expected impact: **high negative**.

Reasoning:

- one indirect call per pixel (or channel) prevents full inlining,
- adds call/return + pointer dereference overhead,
- can reduce optimizer ability to coalesce loads/stores.

In practical LED workloads (hundreds to thousands of pixels per frame), this can become a visible frame-rate/latency regression.

### If virtual dispatch happens once per frame / per strip

Expected impact: **low to moderate**.

Reasoning:

- dispatch cost amortized over buffer processing,
- DMA transfer setup still dominates in many hardware methods,
- pixel packing can stay in a specialized internal routine.

## 3.2 DMA-centric method consideration

Because many implementations primarily configure DMA/PIO/RMT/I2S transfers:

- transport-object polymorphism cost is often minor **if done outside pixel loop**,
- pixel encoding/packing remains the critical path for CPU time,
- frame pacing/jitter risk increases mainly if packing path is de-optimized.

## 3.3 `std::span` + ranges packing API impact

A batched API can improve throughput by enabling:

- bounds-known buffer views (`std::span<uint8_t>`),
- loop hoisting and reduced repeated pointer arithmetic,
- better vectorization opportunities on host builds,
- cleaner separation of scalar vs bulk packers.

But `std::ranges::range<Color>` in embedded hot loops can regress performance if it introduces:

- non-contiguous iterators,
- abstraction layers that are not optimized out,
- additional template depth and compile cost.

### Recommendation

- Use `std::span<const Color>` (contiguous) as the primary hot-path bulk API.
- Keep a scalar fast path for small updates.
- Avoid generic range concepts in the innermost encode loop unless profiling proves equal performance.

## 3.4 Ranges/views for zero-allocation color generation

Using ranges/views to generate and transform `Color` values lazily can remove temporary buffers and reduce RAM pressure.

### Where it helps

- remove intermediate allocation for pipelines like generate -> transform -> serialize,
- reduce peak working-set memory for long strips,
- simplify composition of effects while keeping effect code declarative.

### Where it can hurt

- extra iterator adaptor layers can increase instruction count per pixel,
- non-trivial view iterators can inhibit inlining and loop simplification,
- higher compile time and larger debug symbols from heavy range adaptor templates,
- some embedded libstdc++ versions have less mature optimizer behavior for complex range pipelines.

### Practical guidance for this codebase

- Treat ranges/views as an **effect-construction layer**, not the byte-pack hot loop itself.
- Constrain hot-path overloads to `std::ranges::contiguous_range` + `std::ranges::sized_range` where possible.
- If input is non-contiguous, serialize in chunks to a small stack buffer and pass chunk spans to packers.
- Keep serialization kernels pointer/span based and branch-light.
- Prefer simple views (`transform`, `take`, `drop`) and avoid deeply nested adaptor chains in frame loops.

### Net impact expectation

- **Code impact:** moderate increase in template surface, but cleaner effect composition APIs.
- **Runtime impact:** neutral to negative in hot loops unless constrained to contiguous/sized ranges.
- **Memory impact:** positive when replacing heap/large-temp buffers with lazy pipelines.

---

## 4) Memory Usage Impact

## 4.1 Flash/ROM

### Observed budget risk

One measured subset build (RP2040 + limited color/method combinations) was estimated at **~1.2 MB** of code size.

Implication:

- if a small subset is already near 1.2 MB, full matrix-enabled builds can quickly become prohibitive,
- on devices with <4 MB flash, this materially reduces headroom for app logic, networking stacks, OTA safety margins, and diagnostics,
- flash pressure becomes a primary architecture driver, not a secondary optimization.

### Moving toward interfaces may reduce:

- duplicate template instantiations,
- alias/symbol bloat.

### But may increase:

- non-inlined generic code paths,
- extra adapter layers,
- vtable/typeinfo-like metadata (depending on toolchain/settings).

Net flash impact can go either way; libraries with very large type matrices often see moderate flash savings from reducing instantiation count.

### Recommendation under <4 MB flash targets

Assumption: all projected production targets fall in this class.

- treat template-instantiation control as a hard requirement,
- ship narrowed configuration profiles (per platform/method family) rather than enabling the broad matrix by default,
- centralize pack kernels around shared `RgbColor`-based paths to reduce duplicate feature-specialized emitters,
- keep compile-time injection, but aggressively collapse equivalent specializations into common kernels.

### Architecture priority shift for this project

Given the `<4 MB` baseline across all projected targets, prioritize changes in this order:

1. reduce template surface area and duplicate instantiations,
2. preserve hot-loop efficiency using shared span/pointer pack kernels,
3. keep runtime abstraction limited to setup-time selection only.

## 4.2 RAM

### Interface model adds

- object state for injected components,
- vptr per polymorphic object (when using virtuals),
- possible heap usage if factories allocate dynamically.

### Template/static model adds

- little runtime object overhead,
- but can increase static data if each specialization carries constants/tables.

For microcontrollers, avoid heap-centric DI containers; use static storage or stack-bound configuration objects.

---

## 5) Codebase Impact

## 5.1 Positive effects

- clearer separation: platform transport vs protocol/packer logic,
- easier platform onboarding and ownership,
- reduced include complexity in aggregators,
- better unit-test isolation when boundaries are explicit.

## 5.2 Migration risks

- broad API and alias ripple effects,
- subtle performance regressions from abstraction leakage,
- larger binary if both old and new paths coexist too long.

## 5.3 Complexity tradeoff

- templates shift complexity to compile-time and type surfaces,
- compile-time injection keeps wiring explicit at build time and avoids container lifecycle overhead.

For this domain (embedded real-time LED I/O), preserving compile-time specialization in hot loops is typically the safer performance choice.

---

## 6) `applyPixelColor` Evolution Strategy

Current scalar signature pattern is efficient for single-pixel writes but incurs per-call overhead in full-frame writes.

### Proposed layered API

1. Keep scalar API for compatibility and sparse updates.
2. Add bulk contiguous API:

```cpp
void applyPixelColors(std::span<uint8_t> pixelBytes,
                      std::span<const ColorObject> colors,
                      uint16_t startPixel);
```

3. Internally dispatch to chipset-specific pack kernels selected once during setup.

### Optimization opportunities

- precompute channel offsets/order once per strip,
- unroll fixed-byte packers (3-byte, 4-byte, DotStar with luminance byte),
- use restrict-like assumptions where toolchain allows,
- minimize bounds checks in inner loop (validate once before loop),
- keep pointer arithmetic linear and branch-free.

### Note on ranges API

If a range-based overload is desired, make it a wrapper over span-based core:

- range overload avoids allocation for contiguous/sized inputs and only materializes when needed,
- core hot path remains contiguous + predictable.

---

## 7) Recommended Target Architecture (Practical)

Use a **hybrid model**:

- Keep compile-time/static specialization for pixel pack kernels and transport primitives.
- Introduce runtime selection only at setup time (method/profile/descriptor).
- Use non-virtual function pointer tables or small static strategy objects for hot-path dispatch.
- Keep virtual interfaces optional and outside per-pixel loops.
- Do not use DI containers; use explicit compile-time wiring and small setup-time selector tables.

For this project specifically, the hybrid should be **flash-first**:

- default to shared kernels and constrained profiles,
- allow specialization only where measured performance gain justifies flash cost,
- reject new template combinations that do not pass flash budget gates.

This gives most maintainability benefits of DI without paying full virtual-dispatch costs in frame-critical code.

---

## 8) Suggested Migration Sequence (No Code Changes Yet)

1. Reorganize method folders and update include routers (`NeoMethods.h`, `XMethods.h`) with no behavior change.
2. Prune default build profiles to constrained, flash-safe combinations (platform/method/color subsets).
3. Add a benchmark harness for:
   - full frame fill,
   - partial updates,
   - show/flush latency,
   - throughput (pixels/s),
   - flash/RAM size.
4. Define a strict default flash budget for `<4 MB` devices and fail CI when exceeded.
5. Introduce bulk span-based packing API alongside scalar API.
6. Add setup-time method abstraction (hybrid dispatch) while preserving current template hot paths.
7. Evaluate whether any virtual interface remains after measurements.

---

## 9) Decision Summary

- **Primary constraint:** all projected targets are `<4 MB` flash, so code size is a release-gating metric.
- **Folder reorganization:** strongly recommended now; low risk, high maintainability benefit.
- **Full virtual/DI replacement of templates:** not recommended for inner loops.
- **Hybrid (runtime setup + static hot path):** recommended, with flash-first specialization policy.
- **`std::span` bulk packing:** recommended for primary fast path.
- **`ranges`/`views` pipelines:** recommended for zero-allocation effect composition, but keep final serialization on contiguous span/pointer kernels.
- **Template matrix control:** mandatory for all projected targets (<4 MB); default builds should use constrained profiles, not full combinatorial surfaces.

This approach aligns with the library’s hot-path constraints while reducing architectural sprawl and preserving performance headroom on microcontrollers.
