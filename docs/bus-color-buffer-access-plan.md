# Bus Color Buffer Access Plan (Span + Root Ownership)

Status: draft
Scope: bus color-buffer access and factory construction model updates

## Goal

Shift bus color management to an explicit contiguous-buffer model by exposing a `span<TColor>` accessor on contiguous buses and enforcing root-owned buffer composition for aggregate buses.

This plan is designed to:

1. keep the current virtual-first architecture intact,
2. make the top-most bus the owner of the pixel buffer,
3. expose direct pixel-buffer access to bus consumers,
4. preserve existing behavior for users who want current internal allocation defaults.

## Non-Goals

- No protocol byte-stream behavior changes.
- No transport API behavior changes.
- No full `ConcatBus` ownership rewrite in the first pass (mosaic root-ownership is in scope).
- No C++20-only API additions; remain C++17-compatible.

## Performance Goal (Hot Path)

`getPixel`/`setPixel` style per-pixel APIs are treated as legacy compatibility paths and are not the target hot-path workflow.

Required hot-path behavior:

- Consumers should write/read pixels through direct contiguous span access (`pixelBuffer()`) whenever available.
- Per-pixel animation loops should avoid virtual dispatch entirely.
- Virtual dispatch should remain at coarse boundaries only (construction/lifecycle/update orchestration), not per-pixel operations.

End-state intent:

- Remove virtual single-pixel and bulk pixel transfer methods from contiguous bus hot-path usage.
- Keep contiguous memory access and frame update paths concrete and branch-light.

## Current State Summary

Current contiguous buses already store colors in internal buffers and expose concrete-class `colors()` helpers:

- `PixelBus<TColor>` stores `std::vector<TColor>`.
- `StaticBusDriverPixelBusT<TTransport, TProtocol>` stores `std::vector<ColorType>`.

Factory entry points (`makeBus(...)`) currently construct static buses with internally allocated buffers from `pixelCount`.

## Proposed Model

### 0) Root-ownership rule (new baseline)

For composed buses, the top-most composed bus should own the authoritative pixel buffer.

Implications:

- Child buses become protocol/transport sinks and/or mapped views.
- The root buffer is the single source of truth for frame data.
- `show()` fan-out pushes mapped slices/views from root to children.

### 1) Introduce a canonical pixel-buffer accessor seam

Add a buffer accessor on `IPixelBus<TColor>` for contiguous-storage-capable buses.

Proposed shape:

```cpp
virtual span<TColor> pixelBuffer();
virtual span<const TColor> pixelBuffer() const;
```

Default behavior in `IPixelBus` should return an empty span (capability absent) so non-contiguous/composite buses can remain valid without synthetic buffers.

For contiguous/root-owned buses, this accessor is the primary consumer API for frame mutation.

### 2) Transition away from bulk/single-pixel APIs

End-state target: remove `setPixelColors/getPixelColors` and `setPixelColor/getPixelColor` from the primary bus API surface.

Transition policy:

- Remove bulk/single-pixel APIs from the exposed bus interfaces.
- Use `pixelBuffer()`-first patterns for all contiguous/root-owned bus mutation/read paths.
- If temporary adapters are needed during refactor, keep them internal-only and not part of public API/contracts.

Rationale:

- converges the API on one explicit contiguous data path,
- eliminates duplicated mutation/read surfaces,
- aligns external allocation and ownership behavior around spans.

### 3) Keep static factory bus pixel-count driven

`StaticBusDriverPixelBusT` construction is pixel-count driven in factory entry points.

Direct contiguous access remains available through `pixelBuffer()`.

### 4) Mosaic first: root allocates from known geometry

For mosaic construction, root pixel capacity is derivable at call time:

`panelWidth * panelHeight * tilesWide * tilesHigh`

This makes mosaic the first composite where root-owned storage should be implemented.

## Factory API Changes

### A) Keep existing creation path (backward compatibility)

Keep current forms:

```cpp
auto bus = makeBus<ProtocolDesc, TransportDesc>(pixelCount, ...);
```

This remains internally allocated.

### B) Remove external-buffer overloads

Factory helpers no longer accept external color buffers.

Keep only pixel-count based creation:

```cpp
auto bus = makeBus<ProtocolDesc, TransportDesc>(pixelCount, ...);
```

And corresponding lower-level helper:

```cpp
makeStaticDriverPixelBus(pixelCount, transportSettings, protocolSettings)
```

This avoids split ownership and keeps root-owned composition responsibilities in aggregate buses.

### C) Pixel count source of truth

For bus factories, `pixelCount` is now always an explicit argument in the public creation path.

### D) Composite construction path

Use existing composite factories (`makeMosaicBus(...)`, concat creation helpers) with root-owned buffer behavior handled by the composite bus implementations.

## Composite Factory Impact (Concat + Mosaic)

This section defines how root-owned buffering affects `concatBus`/`makeBus(...)` composition and `makeMosaicBus(...)`.

### 0) Concat factory call shape (expected)

Concat should support root-owned construction via explicit per-child pixel lengths and child factory entries.

Target call shape:

```cpp
auto concatBus = makeBus(
   std::initializer_list<uint16_t>{ 64, 32, 99 },
   makeBus<Ws2812, RpPio>(...),
   makeBus<Ws2812, RpPio>(...),
   makeBus<Ws2812, RpPio>(...));
```

Interpretation:

- The top-level concat bus allocates and owns the authoritative pixel buffer with size `64 + 32 + 99`.
- The initializer list defines child slice lengths in root index order.
- Each child factory entry maps to one slice and acts as an emit endpoint.
- Child count must match initializer-list length.

### 1) Mosaic factory impact

To satisfy root-ownership, mosaic construction must become root-driven.

Current direction:

- `makeMosaicBus(...)` remains the mosaic entry point.
- Root computes mosaic pixel capacity from settings and owns backing storage.
- Child instances are mapped to root ranges during lifecycle setup.

### 2) Behavioral impact for mosaic

- Root mosaic buffer is authoritative for read/write.
- Child buses no longer need to own primary pixel storage.
- `show()` translates root slices into child updates.
- Geometry validation (tile count vs child count) moves to root constructor boundary.
- Consumer render loops target root `pixelBuffer()` directly and avoid child per-pixel virtual calls.

### 3) Contract updates required for mosaic usage

Document these invariants for root-owned mosaic factories:

1. Root mosaic object owns the authoritative frame buffer.
2. Child bus objects are implementation endpoints for emit/update, not authoritative storage owners.
3. Tile count mismatch is a construction error (`tilesWide * tilesHigh` must match provided child factories unless explicitly allowing sparse tiles).

### 4) `pixelBuffer()` capability expectations for composites

For first pass:

- Root-owned mosaic exposes non-empty `pixelBuffer()`.
- Root-owned concat exposes non-empty `pixelBuffer()` when built via the initializer-list factory form.

Implication for users:

- Use root mosaic `pixelBuffer()` for direct contiguous mutation.
- Use root concat `pixelBuffer()` for direct contiguous mutation.
- Treat child buses as emit endpoints, not direct frame-edit surfaces.

### 5) Optional phase-2+ composite factory enhancements (not in first pass)

Potential future additions, intentionally deferred:

- `makeConcatBusView(...)` helper exposing child contiguous-buffer list metadata.
- `makeMosaicBusView(...)` helper exposing panel-to-child mapping metadata.
- Read-only flattened snapshot helper for diagnostics/export.

These are separate from the first-pass external allocation objective and should not block it.

### 6) Composite example patterns (first-pass intended usage)

#### A) Root-owned concat via initializer-list lengths + child factories

```cpp
auto concatBus = makeBus(
   std::initializer_list<uint16_t>{ 64, 32, 99 },
   makeBus<Ws2812, RpPio>(/* transport/protocol config */),
   makeBus<Ws2812, RpPio>(/* transport/protocol config */),
   makeBus<Ws2812, RpPio>(/* transport/protocol config */));
```

Notes:

- `concatBus` owns the authoritative root pixel buffer.
- Child factory count must match the length-list entry count.

#### B) Root-owned mosaic via `makeMosaicBus(...)`

```cpp
auto wall = makeMosaicBus(
   MosaicBusSettings{
      .panelWidth = 8,
      .panelHeight = 8,
      .layout = PanelLayout::RowMajor,
      .tilesWide = 8,
      .tilesHigh = 8,
      .tileLayout = PanelLayout::RowMajor
   },
   bus0,
   bus1
   /* ... one per tile ... */);
```

Notes:

- `makeMosaicBus(...)` computes root pixel capacity from mosaic settings and allocates/owns root storage.
- Direct contiguous writes target root mosaic `pixelBuffer()`.

## Ownership and Lifetime Contract

### Internal mode

- Bus owns storage.
- Lifetime tied to bus object.

For root-owned mosaic, this rule applies at the mosaic root.

## Dirty/Show Semantics

`show()` behavior must remain functionally equivalent:

- If protocol is not `alwaysUpdate()` and no mutations occurred, no update.
- If mutations occurred, update then clear dirty state.

Mutation policy:

- Direct writable span access is the mutation path.
- Dirty state is signaled by writable `pixelBuffer()` access in contiguous implementations.

Alternative (future): tracked proxy span wrappers. Not recommended for first pass due to complexity.

## Bus-Type Capability Policy

`pixelBuffer()` capability in first pass:

- `PixelBus`: supported (returns contiguous span).
- `StaticBusDriverPixelBusT`: supported.
- Root-owned `MosaicBus`: supported (returns contiguous span).
- `SegmentBus`, `ConcatBus`: supported (mapped/root spans).

Future optional enhancement:

- a read-only flattened view API for composites (separate design).

## Migration Plan

### Phase 1: Interface and implementation seam

- Add `pixelBuffer()` virtuals to `IPixelBus` with empty default.
- Implement overrides in contiguous buses.
- Remove bulk/single-pixel APIs from exposed contiguous bus interfaces.

### Phase 2: Factory cleanup

- Keep factory construction pixel-count based.
- Remove external-buffer overloads from public factory entry points.

### Phase 2b: Root-owned mosaic lifecycle binding

- Keep `makeMosaicBus(...)` as the construction API.
- Compute root capacity from mosaic geometry.
- Bind child endpoints to root-owned ranges during lifecycle setup.

### Phase 3: Dirty signaling for direct span writes

- Ensure writable `pixelBuffer()` access marks frames dirty in contiguous buses.
- Document expected usage pattern for direct buffer mutation + `show()`.

### Phase 4: Examples + docs

- Add minimal examples demonstrating root-owned mosaic construction with `makeMosaicBus(...)`.
- Update architecture and factory docs to include root-owned aggregate capability semantics.

### Phase 5: Contract and cleanup pass

- Verify no exposed APIs or examples rely on bulk/single-pixel methods.
- Keep composite behavior explicit via composition-specific APIs and child-buffer access patterns.
- Ensure hot-path rendering examples use direct span iteration only.

## Test Plan Additions

Run existing native gates plus new focused tests:

1. Pixel buffer round-trip
   - Write/read via `pixelBuffer()` spans and verify expected frame data.
2. Direct span mutation path
   - Modify `pixelBuffer()` data, verify `show()` updates protocol.
3. Bounds and clamp behavior unchanged
   - Re-run existing bus-spec cases for offset clamp and out-of-range safety.
4. Root mosaic ownership
   - Verify root `pixelBuffer()` mutations correctly propagate to all child updates.
5. Mosaic geometry/factory validation
   - Verify invalid tile/factory count mismatches fail at construction.
6. Hot-path dispatch check
   - Verify hot-path frame loops can execute without per-pixel virtual calls in contiguous/root-owned paths.
7. API contraction gate
   - Verify no production/public call sites depend on removed bulk/single APIs.

Minimum validation commands:

```powershell
pio test -e native-test
pio test -e native-test --filter contracts/test_protocol_transport_contract_matrix_compile
```

## Risks and Mitigations

1. Silent stale frames from direct span writes
   - Mitigation: contiguous writable `pixelBuffer()` marks dirty + docs + tests.
2. Overload ambiguity in factory templates
   - Mitigation: use strongly typed `span<ColorType>` signatures and targeted SFINAE constraints.
3. Capability confusion for composite buses
   - Mitigation: explicit root-owned mapping rules for mosaic and concat.
4. Migration churn from API removal
   - Mitigation: explicit call-site migration checklist and targeted compile-gate checks.
5. Performance regressions from fallback paths
   - Mitigation: enforce span-first examples/tests and verify no per-pixel virtual dispatch in hot paths.

## Open Decisions

1. Should dirty semantics remain implicit on writable `pixelBuffer()` access?
   - Current recommendation: yes, keep implicit dirtying in contiguous concrete classes.
2. Should first pass include a `hasPixelBuffer()` helper?
   - Optional; can infer via `pixelBuffer().data() != nullptr` and size.
3. Should `colors()` concrete helpers be retained as aliases?
   - Recommendation: retain temporarily for compatibility; prefer `pixelBuffer()` in docs.
4. Final composite write/read surface after bulk/single removal?
   - Recommendation: keep composite-specific operations explicit and avoid pretending composites are contiguous buffers.
5. Should `MosaicOptions` be a new type alias to `MosaicBusSettings` or a new descriptor type?
   - Recommendation: start with alias/descriptor shim for minimal migration friction.

## Acceptance Criteria

- Factory users use pixel-count based creation without changing protocol/transport behavior.
- Contiguous buses expose writable/readable contiguous spans through one canonical accessor.
- Existing `makeBus(pixelCount, ...)` usage compiles unchanged.
- Native tests confirm safety and behavior across contiguous and aggregate ownership modes.
- Bulk/single-pixel APIs are removed from exposed bus interfaces and replaced by span/`pixelBuffer()`-first usage patterns.
- Root-owned mosaic construction is available via `makeMosaicBus(...)` and root buffer size is derived from geometry.
- Consumers can run pixel render loops against direct buffer spans with no per-pixel virtual dispatch in contiguous/root-owned buses.
