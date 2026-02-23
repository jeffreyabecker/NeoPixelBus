# C++11 Backward Compatibility Plan (src/virtual)

## 1) Purpose

This document defines a practical migration path to make the `src/virtual/` codebase build and run with C++11 while preserving current behavior and keeping modern builds working.

The strategy is **compatibility-first**:
1. add a small compatibility layer,
2. migrate APIs to use the layer,
3. preserve behavior with dual-standard validation.

---

## 2) Scope

### In Scope
- `src/virtual/**`
- `src/VirtualNeoPixelBus.h` public surface where it exposes virtual-layer types
- Build configuration needed to compile virtual layer as C++11
- Example smoke targets used for verification

### Out of Scope (initially)
- Refactoring non-virtual legacy folders unless required by compile dependencies
- New features unrelated to compatibility

---

## 3) Baseline and Constraints

- Current project standard is configured for C++23.
- `src/virtual` currently uses C++17 and C++20 features (`std::span`, concepts/requires, `std::optional`, inline variables, etc.).
- Backward compatibility target is C++11 without changing runtime semantics.
- No exceptions or RTTI-heavy additions should be introduced.

---

## 4) Inventory of C++17+ Features Used in `src/virtual`

### C++20
- `std::span`
- `requires` clauses and `<concepts>` (`std::derived_from`, `std::convertible_to`)

### C++17
- `std::optional`
- inline variables (`inline constexpr`, `static inline` members)
- `std::clamp`
- `[[maybe_unused]]`

### C++14+
- `std::make_unique` is used in code today; C++11 baseline requires compat fallback.

---

## 5) Compatibility Design

Create `src/virtual/compat/` with shim headers that expose project-level aliases/helpers, backed by vetted third-party polyfills where appropriate.

## 5.1 Compatibility API Surface

- `npb::Span<T>` and `npb::ConstSpan<T>` (adapter over `std::span` or `tcb::span`)
- `npb::Optional<T>` (adapter over `std::optional` or `tl::optional`)
- `npb::make_unique<T>(...)`
- `npb::clamp(value, lo, hi)`
- `NPB_MAYBE_UNUSED` macro
- Feature macros:
  - `NPB_HAS_STD_SPAN`
  - `NPB_HAS_STD_OPTIONAL`
  - `NPB_HAS_CONCEPTS`
  - `NPB_CXX_STANDARD`

Rule: internal and public virtual APIs should depend on `npb` compatibility wrappers, not direct C++17/20 library symbols.

Polyfill sources to standardize on:
- `std::span` fallback: `tcb::span` (tcbrindle/span)
- `std::optional` fallback: `tl::optional` (TartanLlama/optional)
- `std::clamp`: local helper in `compat` (small and deterministic)

---

## 6) Feature Mapping to C++11

### 6.1 `std::span` -> `npb::Span`
- For C++20 builds, alias to `std::span`.
- For C++11/C++14 builds, alias to `tcb::span`.
- Keep all project APIs typed as `npb::Span`/`npb::ConstSpan` so backend choice is centralized.

### 6.2 `requires`/concepts -> SFINAE + `static_assert`
- Replace constrained templates with `std::enable_if` and type traits.
- Add readable `static_assert` diagnostics in constructors/factories.

### 6.3 `std::optional` -> `npb::Optional`
- C++17+ alias to `std::optional`.
- C++11/C++14 builds alias to `tl::optional`.
- Keep project APIs on `npb::Optional` to avoid direct dependency leakage.

### 6.4 Inline variables -> C++11-safe static definitions
- Convert `inline constexpr` namespace variables to:
  - `static const` in class/namespace + one out-of-line definition (in `.cpp`), or
  - function-local `static` accessors if header-only is required.

### 6.5 `std::clamp` -> `npb::clamp`
- Implement a small local helper in `compat` (constexpr-friendly where possible).

### 6.6 `[[maybe_unused]]` -> macro
- `#if` guard to use attribute where available; otherwise define empty macro.

### 6.7 `std::make_unique`
- Use `std::make_unique` when available; provide `npb::make_unique` fallback for C++11.

---

## 7) Phased Execution Plan

## Phase 1 — Freeze Baseline
- Record exact compiler targets/toolchains for current green build.
- Add a compatibility branch note and migration checklist.
- Identify one smoke example as acceptance sentinel.

Exit criteria:
- Current C++23 build reproducible from clean checkout.

## Phase 2 — Build Matrix and Guardrails
- Add C++11 build target(s) in `platformio.ini` or CI workflows.
- Keep C++23 target active.
- Mark C++11 failures as expected until migration slices land.

Exit criteria:
- Both standards build jobs exist and run.

## Phase 3 — Introduce Compat Layer
- Add `src/virtual/compat/*.h`.
- Vendor/pin polyfills for `tcb::span` and `tl::optional` in project-approved third-party location.
- Add thin adapter headers mapping polyfill/std types to `npb::Span` and `npb::Optional`.
- Add unit/smoke compile checks for shim APIs.

Exit criteria:
- Shim headers compile standalone in C++11 and C++23.

## Phase 4 — Public API Migration (Low Risk First)
- Replace `std::span` in interfaces with compat alias.
- Replace `std::optional` in signatures with compat alias.
- Replace attributes/macros in headers.

Exit criteria:
- Public virtual headers parse in C++11.

## Phase 5 — Template Constraints Migration
- Replace `requires` blocks with C++11-compatible templates.
- Preserve compile-time misuse diagnostics.

Exit criteria:
- Constrained emitters/resources compile in C++11.

## Phase 6 — Inline Variable and Constant Cleanup
- Convert inline variable definitions to C++11-safe patterns.
- Ensure no ODR violations.

Exit criteria:
- No C++17-inline-variable dependency remains in `src/virtual`.

## Phase 7 — Validation and Stabilization
- Run smoke examples under C++11 and C++23.
- Compare behavior for representative pixel paths:
  - `PixelBus`
  - `SegmentBus`
  - `ConcatBus` / `MosaicBus`
  - at least one one-wire and one clock/data emitter

Exit criteria:
- Same observable output behavior (within timing tolerance).

## Phase 8 — Documentation and Policy
- Update contributor guidance for compatibility macros/wrappers.
- Add “do not introduce direct C++17+ APIs in virtual public headers” guidance.

Exit criteria:
- Docs and review checklist updated.

---

## 8) Risk Register

1. **API break risk** from changing method parameter types.
   - Mitigation: use aliases preserving call sites (`npb::Span` style wrappers).

2. **ODR/link risk** from inline-variable replacement.
   - Mitigation: centralize definitions and add link-stage checks.

3. **Template diagnostics degrade** after removing concepts.
   - Mitigation: add explicit `static_assert` messages.

4. **Behavior drift** in optional-based topology probing.
  - Mitigation: preserve semantics via `npb::Optional` wrapper before any API redesign.

5. **Third-party dependency drift** for polyfills.
  - Mitigation: pin exact versions/commits, document upgrade path, and keep adapter surface minimal.

---

## 9) Acceptance Criteria

A release candidate is C++11-backward-compatible when:
- `src/virtual` compiles under C++11 with no feature-test hacks in user code.
- Existing C++23 build still passes.
- Smoke examples run successfully on target boards for both standards.
- Public virtual headers do not directly require C++17/C++20 standard library types.

---

## 10) Immediate Next Actions

1. Add `compat/` shim headers (`span`, `optional`, `utility`, `attributes`, `config`).
  - Back `span` with `tcb::span` and `optional` with `tl::optional` for C++11/C++14 builds.
  - Add `npb::make_unique` fallback for C++11.
2. Update `IShader`, `IPixelBus`, `IProtocol`, and bus interfaces to compat span.
3. Replace concept constraints in emitter templates with C++11-compatible SFINAE equivalents.
4. Stand up dual-standard build jobs and resolve compile deltas in priority order.
