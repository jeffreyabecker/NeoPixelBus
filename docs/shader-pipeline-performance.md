# Shader Pipeline Performance Analysis (Current Architecture)

## Overview

This document analyzes the render hot path in the current virtual architecture and clarifies where cycle cost is actually spent.

Key result: **shader vtable lookup is not the dominant per-pixel cost in the current implementation**.

---

## Architecture: Current Hot Path

When `PixelBus::show()` calls emitter `update(span<const Color>)`, the emitter executes:

```
PixelBus::show()                            // 1× per frame
  └─ _emitter->update(colors)               // virtual — 1× per frame
       ├─ copy colors -> _scratchColors     // O(N), only when shader present
       ├─ _shader->apply(_scratchColors)    // virtual — 1× per frame
       │    └─ loops all pixels/channels inside concrete shader
       ├─ serialize source colors to bytes   // O(N * channels)
       │    (ColorOrderTransform or emitter-specific loop)
       └─ transmitBytes(buffer) / DMA handoff
```

`ShaderChain` behavior is also frame-level:

```
_shader->apply(span)
  └─ for each shader in chain:
       shader->apply(span)                  // virtual — once per shader per frame
```

---

## What Is Not Dominating

The previous model assumed a per-pixel virtual shader call (`apply(i, color)`) and a per-pixel virtual transform call. That is not how the code runs today.

In current code:
- shader dispatch is one virtual call per frame (or one per shader in chain per frame), not per pixel
- transform/serialization work is concrete loops in emitter/transform classes
- no per-pixel vtable call on `_transform.apply(...)`

So the old “~30–40 cycles/pixel from two vtable calls” estimate is no longer representative.

---

## Dominant Costs in Current Path

For large strips/matrices, dominant cost is usually:

1. **Memory traffic**
   - copy input span to `_scratchColors` when shader exists
   - read shaded colors and write serialized byte buffer

2. **Shader arithmetic across full span**
   - gamma correction and/or current limiter loops over all channels

3. **Serialization loops**
   - channel reorder and byte packing in transform/emitter

4. **Transport handoff / bus transmission timing**
   - bus-specific, often throughput-limited rather than dispatch-limited

Virtual dispatch overhead remains present but generally amortized at frame granularity.

---

## Practical Performance Guidance

### Highest-impact optimizations first

1. Reduce full-frame copies where safe (reuse buffers, in-place paths, no-shader fast path)
2. Minimize per-channel math in shader passes (especially limiter/gamma combinations)
3. Optimize serialization loops and buffer layout for cache/fetch locality
4. Keep DMA/bus transfer efficient and non-blocking where supported

### Lower-impact in current architecture

- Replacing frame-level shader virtual dispatch alone is unlikely to produce large end-to-end wins.

---

## Compile-Time `ShaderPipeline`: Revised Value

A compile-time pipeline can still be useful, but for **different reasons** than originally claimed:

- improves inlining opportunities between shader stages
- can remove dynamic chain iteration overhead
- enables compile-time composition guarantees

However, expected gains should be measured against current baseline, where dispatch overhead is already mostly frame-level.

---

## Recommended Measurement Plan

Before redesigning shader dispatch, measure these variants on target boards:

1. **No shader**
2. **Single shader** (`GammaShader`)
3. **Two shaders** (`GammaShader + CurrentLimiterShader`)
4. **ShaderChain vs direct concrete shader object** (same shader math)

Collect per frame:
- total `update()` time
- shader pass time
- serialization time
- transmit/handoff time

Only pursue architectural changes if measured wins are meaningful for target frame budgets.

---

## Recommendation

- Treat the old per-pixel-vtable model as historical and not representative of the current code path.
- Prioritize optimization of copy + shader math + serialization first.
- Keep `ShaderPipeline` as an optional future optimization path, justified by benchmarked gains rather than assumed vtable savings.
