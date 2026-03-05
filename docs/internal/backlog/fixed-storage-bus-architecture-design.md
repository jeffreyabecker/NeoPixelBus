# Fixed-Storage Static Bus Architecture Design

This is a second-stage architecture document for the fixed-storage static bus initiative.
It expands the planning depth from the initial concept doc into concrete architecture decisions, sequencing, and execution TODOs.

Related:

- `compile-time-static-allocation-plan.md` (feature scope and high-level rollout)

## 1. Problem Statement

We need a static bus path that can be used in fixed-size deployments with a strict no-heap guarantee in bus/core buffer paths.

The current static path has two blockers for this guarantee:

1. Buffer policy and storage ownership are not explicitly separated in the type model.
2. Static path internals still carry dynamic-friendly behavior that can trigger allocation fallback or dynamic metadata dependencies.

## 2. Architectural Goals

1. Keep one `StaticBus` behavioral model and vary storage policy via a buffer-context type.
2. Preserve current default dynamic-storage behavior (currently exposed as legacy `OwningBufferContext`) for compatibility.
3. Use a single storage-parameterized context type to represent both dynamic and fixed-storage semantics.
4. Keep all active code C++17.
5. Keep protocol/transport compatibility rules and descriptor-based factory rules unchanged.

## 3. Proposed Type Model

### 3.1 Bus composition direction

`StaticBus` adopts a buffer-context policy template/CRTP direction:

- Existing default (compatibility):
  - `StaticBus<..., BufferContext<std::vector<uint8_t>, TColor>>`
- New strict path:
  - `StaticBus<..., BufferContext<std::array<uint8_t, N>, TColor>>`

This keeps bus behavior centralized while swapping memory strategy through the context.

### 3.2 Buffer context responsibilities

All buffer contexts must provide:

- `bufferAccess()` / `bufferAccess() const`
- protocol slice layout compatibility with binder expectations
- deterministic initialization rules for root/shader/protocol regions

`BufferContext<std::vector<uint8_t>, TColor>` responsibilities:

- preserve current behavior and existing allocation-compatible mode

`BufferContext<std::array<uint8_t, N>, TColor>` responsibilities:

- never allocate, grow, or own storage
- validate storage contract against required unified bytes
- expose identical access/provider behavior to the bus runtime

### 3.3 Storage contract

For the fixed-storage path:

- required bytes = unified bytes (root + shader + protocol total)
- provided storage must satisfy required bytes
- undersized storage is a hard contract violation (assert/failure mode follows project conventions)

### 3.4 Selected Strategy: storage-templated `BufferContext`

Selected direction:

- use one context type parameterized by storage container, for example:
  - `BufferContext<std::vector<uint8_t>, TColor>`
  - `BufferContext<std::array<uint8_t, N>, TColor>`

Potential benefits:

- fewer top-level context types
- shared layout/provider implementation in one class template
- explicit storage policy at type level

Key caveats:

- legacy `Owning*` naming may become semantically misleading for external span/array-backed modes
- no-heap guarantee is incomplete if slice metadata still uses dynamic containers
- runtime-sized layouts may not map cleanly to `std::array<uint8_t, N>` without a parallel runtime-validated path

Naming/compatibility guidance:

- keep a shared internal implementation layer parameterized by storage + metadata policies
- expose clear semantic aliases/wrappers for public/internal readability, for example:
  - `HeapBufferContext<TColor> = BufferContext<std::vector<uint8_t>, TColor>`
  - `OwningBufferContext<TColor>` (legacy compatibility alias)
  - `StaticBufferContext<N, TColor> = BufferContext<std::array<uint8_t, N>, TColor>`

This gives implementation reuse while preserving intent clarity in APIs and docs.

## 4. Size Computation Strategy (`N`)

`N` is the total unified buffer bytes for a specific static bus composition.

Preferred path:

- compute `N` in type/value space through factory/builder helper aliases where protocol/settings allow constexpr evaluation

Fallback path:

- use runtime-sized external storage construction with strict no-heap behavior in bus/core path

Important distinction:

- "fixed storage" guarantee is required
- "fully constexpr byte derivation for every configuration" is aspirational and configuration-dependent

## 5. API Surface Direction

Use behavior-descriptive naming rather than overloading "compile-time" in public APIs.

Preferred naming family:

- `FixedStorage*`
- `StaticStorage*`
- `ExternalStorage*` (if semantics remain strict and non-owning)

Examples (illustrative only):

- `factory.makeFixedStorage(buffer, size)`
- `makeFixedStorageBus(...)`
- `BufferContext<std::array<uint8_t, N>, TColor>`

Compatibility rule:

- keep current `make(...)` and default `StaticBus` path available

## 6. Integration Touchpoints

### 6.1 Bus internals

- `src/factory/busses/StaticBus.h`
  - parameterize by buffer-context type
  - keep runtime bus behavior unchanged

### 6.2 Buffer context/access internals

- `src/factory/busses/OwningBufferContext.h`
  - remain default-compatible implementation path (with neutral `BufferContext` naming facade/alias)
- Removed: fixed buffer accessor subsystem (`src/buses/composite/FixedBufferAccessor.h`)
  - ensure fixed path can avoid dynamic-friendly metadata/ownership behavior where feasible

### 6.3 Factory entrypoints

- `src/factory/MakeBus.h`
  - add explicit fixed-storage construction entrypoint(s)
  - preserve timing-first overload ordering constraints

### 6.4 Contracts/tests/docs

- `test/contracts/*`
  - compile-time API/compatibility checks
- `test/busses/*` and/or targeted native suites
  - runtime lifecycle checks for fixed-storage path
- docs
  - update internal and usage docs with fixed-storage construction flow

## 7. Risks and Mitigations

Risk 1: Template complexity/regression in `StaticBus` signatures.

- Mitigation: keep default template parameter mapping to current behavior and add compile contract snapshots.

Risk 2: Hidden dynamic behavior remains in shared helpers.

- Mitigation: add targeted no-heap behavioral assertions in tests for fixed-storage construction path.

Risk 3: Naming drift creates ambiguous API semantics.

- Mitigation: lock naming decision before public API additions and apply consistently across factory/builder/docs.

Risk 4: constexpr-size derivation not possible for all protocols/settings.

- Mitigation: separate guarantees (no-heap vs constexpr sizing) in docs and APIs.

## 8. Detailed TODO List

### Phase A — Architecture Lock

- [ ] A1: Finalize buffer-context template shape for `StaticBus` (CRTP/policy form and default parameter behavior).
- [x] A2: Lock context-family strategy to storage-templated `BufferContext<TStorage, ...>`.
- [ ] A3: Define and document fixed-storage contract surface for `BufferContext<std::array<uint8_t, N>, ...>`.
- [ ] A4: Lock failure/validation behavior for undersized storage consistent with project conventions.
- [ ] A5: Lock public naming (`FixedStorage*` vs `StaticStorage*`) and apply naming matrix across API/doc/tests.

### Phase A.1 — Decision Criteria (context strategy)

- [ ] A1.1: Verify no-heap guarantee includes metadata structures, not just payload storage.
- [ ] A1.2: Evaluate readability/maintainability of storage-templated context naming in call sites and docs.
- [ ] A1.3: Confirm C++17 ergonomics for aliases/helpers around the chosen strategy.
- [x] A1.4: Capture rationale in an ADR-style section in this doc once strategy is selected.

### Phase B — Internal Refactor Design Checklist

- [ ] B1: Identify all dynamic-metadata structures in static-path internals.
- [ ] B2: Specify fixed-size metadata replacements where strand count is compile-known.
- [ ] B3: Define transition strategy that keeps default dynamic path behavior unchanged.
- [ ] B4: Define compile guards/static asserts needed for policy-type misuse.

### Phase C — Factory Design Checklist

- [ ] C1: Define fixed-storage factory method signatures and overload ordering.
- [ ] C2: Define how required bytes are surfaced for both runtime-validated and constexpr-capable flows.
- [ ] C3: Document interaction rules with `getFactory(...).getBufferSize()`.
- [ ] C4: Ensure no ambiguous overloads are introduced.

### Phase D — Verification Plan Checklist

- [ ] D1: Add compile-time contract cases for new fixed-storage API visibility and descriptor compatibility.
- [ ] D2: Add native tests for begin/show/isReady/update behavior using external fixed storage.
- [ ] D3: Add tests validating buffer size contract enforcement.
- [ ] D4: Add negative tests for unsupported/invalid construction combinations (where appropriate).

### Phase E — Documentation Checklist

- [ ] E1: Update internal architecture docs and backlog links.
- [ ] E2: Add usage doc for fixed-storage deployment recipe.
- [ ] E3: Document guarantee boundaries clearly (no-heap guarantee vs constexpr-size availability).

## 9. Exit Criteria

Architecture design phase is complete when:

1. All Phase A items are resolved and recorded.
2. Phase B/C design checklists are concrete enough to implement without re-opening core architecture debates.
3. Test plan from Phase D is approved as the contract gate.
4. Documentation plan from Phase E is accepted.

## 10. Open Questions

1. Should fixed-storage construction require explicit storage argument always, or also support internal container ownership for pure type-level construction?
2. What is the preferred assertion/error mechanism for undersized external storage in native vs embedded targets?
3. Do we want a formal compile-time trait that marks configurations as constexpr-size-derivable?
