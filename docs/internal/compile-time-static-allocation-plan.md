# Compile-Time Static Allocation Plan

This document defines the implementation plan for adding a bus path that is fully static-storage friendly (no runtime heap requirement) for fixed-size deployments.

## Motivation

Current static bus construction allows external backing storage, but static paths can still fall back to runtime allocation in shared buffer internals. This prevents strict no-heap deployments from using the existing path with hard guarantees.

Target outcome:

- A clearly defined bus construction path that performs no `new`/`delete` in bus/core buffer code.
- A deployment flow where callers can compute required bytes, provide fixed storage (global/static/stack), and construct a working bus.

## Scope

In scope:

- Static (fixed strand layout) single-bus path.
- Factory + bus + buffer internals needed to guarantee no heap fallback.
- Compile-time and native tests that enforce the no-heap contract for this path.
- Internal and usage documentation updates.

Out of scope (follow-up work):

- Dynamic/composite buses gaining the same strict no-heap guarantee.
- Reworking transport-specific platform allocators (for example DMA-related allocations in platform transports).
- New compatibility wrappers for legacy API shapes.

## Constraints

- Keep active code paths C++17-compatible.
- Preserve seam responsibilities (`IPixelBus`, `IShader`, `IProtocol`, `ITransport`).
- Keep protocol/transport compatibility rules and descriptor-based factory behavior unchanged.
- Do not add timing-last one-wire overloads in factory APIs.

## Current Gaps

The existing static/factory flow already exposes required buffer sizing, but strict no-heap is not guaranteed because:

1. Buffer internals still support lazy-owned allocation fallback.
2. Metadata for protocol slice layout uses dynamic containers in core static path internals.
3. External-buffer usage is optional in signatures instead of being required for the strict path.

## Design Principles

1. Add an explicit fixed-storage path rather than changing behavior of existing dynamic-friendly overloads.
2. Keep no-heap guarantees local and testable (single path, explicit API surface).
3. Separate "fixed external storage" from "owning/auto-allocating" behavior.
4. Keep behavior predictable: invalid or undersized external storage is a hard construction contract failure path (compile/runtime assert policy per existing project conventions).
5. Keep one bus behavior surface and vary only buffer ownership/storage policy via a context type.

## Core Type Strategy

Adopt a buffer-context policy model for static buses:

- `StaticBus` becomes CRTP/policy-templated on the buffer context type it uses.
- Default remains owning behavior to preserve existing call sites.
- A fixed-size context provides strict no-heap behavior.

Proposed direction:

- `StaticBus<..., TBufferContext = OwningBufferContext<TColor>>` (or equivalent CRTP structure)
- `FixedStorageBufferContext<N, TColor>` (name TBD) where `N` is unified total buffer bytes

Context requirements:

- Expose existing buffer access/provider surface expected by `PixelBus`/binder wiring.
- For fixed-storage context, never allocate or resize storage.
- Validate at construction that required layout bytes fit `N`.

## Proposed API Shape

Introduce a dedicated factory/bus creation flow for strict fixed storage:

- Caller obtains required size through existing `getFactory(...).getBufferSize()`.
- Caller provides backing storage explicitly.
- New creation entrypoint selects fixed-size buffer context and constructs without owning allocation fallback.

Example intent (illustrative only):

```cpp
auto factory = getFactory<Ws2812, Rp2040PioN>(pixelCount, protocolCfg, transportCfg);
constexpr size_t kBytes = /* compile-time or pre-validated fixed size */;
static uint8_t buffer[kBytes];
auto bus = factory.makeFixedStorage(buffer, kBytes);
```

Notes:

- Naming should avoid overloading the term "compile-time" unless it materially guarantees constexpr-only sizing.
- Preferred naming family: `FixedStorage*` / `StaticStorage*` / `ExternalStorage*`.
- Existing `makeCompileTimeBus` wording can remain as an internal discussion label, but public API should describe storage behavior directly.
- Existing `make(...)` paths remain available for current behavior.

## Size-Type Computation Strategy

For the fixed-storage path, `N` (buffer byte count) should be computed in type space by the builder/factory helper when protocol/transport/settings permit constexpr evaluation.

- Add a helper alias/value used by the fixed-storage creation path to derive unified bytes from:
  - root pixel region size,
  - shader scratch size,
  - protocol buffer requirements.
- If full constexpr evaluation is not available for a configuration, preserve a runtime-checked fixed-storage overload as fallback (still no heap allocation in the path itself).

## Implementation Plan

### Phase 1: Buffer internals split (no behavior break)

- Introduce/adjust buffer accessor/context internals to support:
  - fixed-size context mode (no allocation path), and
  - existing owning mode (legacy behavior).
- Refactor `StaticBus` to parameterize buffer context type while preserving existing default behavior.
- Remove dynamic-metadata dependency from static path internals where possible (prefer fixed-size metadata containers derived from strand count).

Deliverable:

- Static bus internals can operate with externally provided storage and without heap fallback.

### Phase 2: Explicit fixed-storage factory entrypoint

- Add strict fixed-storage creation API on static factory surface, routing to fixed-size context.
- Enforce storage validity contract (pointer + size must satisfy required bytes).
- Keep existing overloads untouched for backward-compatible behavior.
- Add builder alias/value plumbing for computed `N` where constexpr composition is possible.

Deliverable:

- Public static/factory path with clear no-heap intent.

### Phase 3: Contract tests and docs

- Add compile contract coverage for fixed-storage API availability and compatibility.
- Add native tests that construct via fixed storage and validate begin/show/update lifecycle.
- Update usage docs with the fixed deployment flow.

Deliverable:

- Tests and docs lock in behavior and usage.

## Validation and Test Gates

Minimum expected validation for this feature:

- `pio test -e native-test --filter contracts/test_factory_descriptor_first_pass_compile`
- `pio test -e native-test` (or targeted additions for new fixed-storage test suites)

Feature-specific assertions should include:

- Fixed-storage path does not require owning allocation fallback.
- Required byte sizing is honored.
- Protocol buffer slices remain correct and stable for the declared strand layout.

## Acceptance Criteria

The backlog item is complete when all are true:

1. A documented, explicit static bus creation path exists that requires caller-provided storage.
2. Bus/core buffer path for this flow performs no runtime heap allocation.
3. Contract and runtime tests cover construction and normal frame lifecycle for this flow.
4. Existing dynamic-friendly paths continue to compile and behave as before.

## Follow-Up Items

- Evaluate extending strict fixed-storage guarantees to composite buses.
- Evaluate optional compile-time byte computation helpers for fully constexpr static-array sizing where protocol/transport settings permit it.
